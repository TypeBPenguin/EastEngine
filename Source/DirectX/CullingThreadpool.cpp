/*
 * Copyright (c) 2016, Intel Corporation
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 * - Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 * - Neither the name of Intel Corporation nor the names of its contributors
 *   may be used to endorse or promote products derived from this software
 *   without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */
#include "stdafx.h"
#include <assert.h>
#include "CullingThreadpool.h"

#define SAFE_DELETE(X) {if (X != nullptr) delete X; X = nullptr;}
#define SAFE_DELETE_ARRAY(X) {if (X != nullptr) delete[] X; X = nullptr;}

template<class T> CullingThreadpool::StateData<T>::StateData(unsigned int maxJobs) :
	mCurrentIdx(~0u),
	mMaxJobs(maxJobs)
{
	mData = new T[mMaxJobs];
}

template<class T> CullingThreadpool::StateData<T>::~StateData() 
{
	SAFE_DELETE_ARRAY(mData);
}

template<class T> void CullingThreadpool::StateData<T>::AddData(const T &data) 
{ 
	mCurrentIdx++; mData[mCurrentIdx % mMaxJobs] = data; 
}

template<class T> const T *CullingThreadpool::StateData<T>::GetData() const
{ 
	return &mData[mCurrentIdx % mMaxJobs];
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Helper class: Mostly lockless queue for render jobs
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CullingThreadpool::RenderJobQueue::RenderJobQueue(unsigned int nBins, unsigned int maxJobs) : 
	mNumBins(nBins),
	mMaxJobs(maxJobs)
{
	mRenderPtrs = new std::atomic_uint[mNumBins];
	mBinMutexes = new std::atomic_uint[mNumBins];
	for (unsigned int i = 0; i < mNumBins; ++i)
		mBinMutexes[i] = 0;

	mJobs = new Job[mMaxJobs];
	for (unsigned int i = 0; i < mMaxJobs; ++i)
		mJobs[i].mRenderJobs = new TriList[mNumBins];

	// Compute worst case job size (we allocate memory for the worst case)
	const unsigned int TriSize = 3 * 3;
	const unsigned int MaxTrisPerJob = TRIS_PER_JOB * 6;
	const unsigned int MaxJobSize = MaxTrisPerJob * TriSize;
	mTrilistData = new float[MaxJobSize * mMaxJobs * mNumBins];

	// Setup trilist objects used for binning
	for (unsigned int i = 0; i < mMaxJobs; ++i)
	{
		for (unsigned int j = 0; j < mNumBins; ++j)
		{
			int idx = i*mNumBins + j;
			TriList &tList = mJobs[i].mRenderJobs[j];
			tList.mNumTriangles = MaxTrisPerJob;
			tList.mTriIdx = 0;
			tList.mPtr = mTrilistData + idx*MaxJobSize;
		}
	}

	// Clear render queue
	Reset();
}

CullingThreadpool::RenderJobQueue::~RenderJobQueue()
{
	SAFE_DELETE_ARRAY(mRenderPtrs);
	SAFE_DELETE_ARRAY(mBinMutexes);
	for (unsigned int i = 0; i < mMaxJobs; ++i)
		SAFE_DELETE_ARRAY(mJobs[i].mRenderJobs);
	SAFE_DELETE_ARRAY(mJobs);
	SAFE_DELETE_ARRAY(mTrilistData);
}

inline unsigned int CullingThreadpool::RenderJobQueue::GetMinRenderPtr() const
{
	unsigned int minRenderPtr = mRenderPtrs[0];
	for (unsigned int i = 1; i < mNumBins; ++i)
	{
		unsigned int renderPtr = mRenderPtrs[i];
		minRenderPtr = renderPtr < minRenderPtr ? renderPtr : minRenderPtr;
	}
	return minRenderPtr;
}

inline void CullingThreadpool::RenderJobQueue::AdvanceRenderJob(int binIdx)
{
	mRenderPtrs[binIdx]++;
	mBinMutexes[binIdx] = 0;
}

inline unsigned int CullingThreadpool::RenderJobQueue::GetBestGlobalQueue() const
{
	// Find least advanced queue
	unsigned int bestBin = ~0u, bestPtr = mWritePtr;
	for (unsigned int i = 0; i < mNumBins; ++i)
	{
		if (mRenderPtrs[i] < bestPtr && mBinMutexes[i] == 0)
		{
			bestBin = i;
			bestPtr = mRenderPtrs[i];
		}
	}
	return bestBin;
}

inline bool CullingThreadpool::RenderJobQueue::IsPipelineEmpty() const
{
	return GetMinRenderPtr() == mWritePtr;
}

inline bool CullingThreadpool::RenderJobQueue::CanWrite() const
{
	return mWritePtr - mBinningCompletedPtr < mMaxJobs;
}

inline bool CullingThreadpool::RenderJobQueue::CanBin() const
{
	return mBinningPtr < mWritePtr && mBinningPtr - GetMinRenderPtr() < mMaxJobs;
}

inline bool CullingThreadpool::RenderJobQueue::CanRender(int binIdx) const
{
	return mRenderPtrs[binIdx] < mBinningCompletedPtr;
}

inline CullingThreadpool::RenderJobQueue::Job *CullingThreadpool::RenderJobQueue::GetWriteJob()
{
	return &mJobs[mWritePtr % mMaxJobs];
}

inline void CullingThreadpool::RenderJobQueue::AdvanceWriteJob()
{
	mWritePtr++;
}

inline CullingThreadpool::RenderJobQueue::Job *CullingThreadpool::RenderJobQueue::GetBinningJob()
{
	unsigned int binningPtr = mBinningPtr;
	if (binningPtr < mWritePtr && binningPtr - GetMinRenderPtr() < mMaxJobs)
	{
		if (mBinningPtr.compare_exchange_strong(binningPtr, binningPtr + 1))
		{
			mJobs[binningPtr % mMaxJobs].mBinningJobStartedIdx = binningPtr;
			return &mJobs[binningPtr % mMaxJobs];
		}
	}
	return nullptr;
}

inline void CullingThreadpool::RenderJobQueue::FinishedBinningJob(Job *job)
{
	// Increment pointer until all finished jobs are accounted for
	job->mBinningJobCompletedIdx = job->mBinningJobStartedIdx;
	unsigned int completedPtr = mBinningCompletedPtr;
	while (completedPtr < mBinningPtr && mJobs[completedPtr % mMaxJobs].mBinningJobCompletedIdx == completedPtr)
	{
		mBinningCompletedPtr.compare_exchange_strong(completedPtr, completedPtr + 1);
		completedPtr = mBinningCompletedPtr;
	}
}

inline CullingThreadpool::RenderJobQueue::Job *CullingThreadpool::RenderJobQueue::GetRenderJob(int binIdx)
{
	// Attempt to lock bin mutex
	unsigned int expected = 0;
	if (!mBinMutexes[binIdx].compare_exchange_strong(expected, 1))
		return nullptr;

	// Check any items in the queue, and bail if empty
	if (mRenderPtrs[binIdx] >= mBinningCompletedPtr.load())
	{
		mBinMutexes[binIdx] = 0;
		return nullptr;
	}

	return &mJobs[mRenderPtrs[binIdx] % mMaxJobs];
}

void CullingThreadpool::RenderJobQueue::Reset()
{
	mWritePtr = 0;
	mBinningCompletedPtr = 0;
	mBinningPtr = 0;

	for (unsigned int i = 0; i < mNumBins; ++i)
		mRenderPtrs[i] = 0;

	for (unsigned int i = 0; i < mMaxJobs; ++i)
	{
		mJobs[i].mBinningJobCompletedIdx = UINT32_MAX;
		mJobs[i].mBinningJobStartedIdx = UINT32_MAX;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Culling threadpool private helper functions
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void CullingThreadpool::SetupScissors()
{
	unsigned int width, height;
	mMOC->GetResolution(width, height);

	// Scissor box of masked occlusion culling library must be a multiple of 32x8 
	const unsigned int BIN_WIDTH_CLAMP = 32;
	const unsigned int BIN_HEIGHT_CLAMP = 8;

	unsigned int binWidth = (width / mBinsW) - ((width / mBinsW) % BIN_WIDTH_CLAMP);
	unsigned int binHeight = (height / mBinsH) - ((height / mBinsH) % BIN_HEIGHT_CLAMP);

	for (unsigned int ty = 0; ty < mBinsH; ++ty)
	{
		for (unsigned int tx = 0; tx < mBinsW; ++tx)
		{
			unsigned int threadIdx = tx + ty*mBinsW;

			// Adjust rects on final row / col to match resolution
			mRects[threadIdx].mMinX = tx*binWidth;
			mRects[threadIdx].mMaxX = tx + 1 == mBinsW ? width : (tx + 1) * binWidth;
			mRects[threadIdx].mMinY = ty * binHeight;
			mRects[threadIdx].mMaxY = ty + 1 == mBinsH ? height : (ty + 1) * binHeight;
		}
	}
}

void CullingThreadpool::ThreadRun(CullingThreadpool *threadPool, unsigned int threadId)
{ 
	threadPool->ThreadMain(threadId); 
}

void CullingThreadpool::ThreadMain(unsigned int threadIdx)
{
	while (true)
	{
		bool threadIsIdle = true;
		unsigned int threadBinIdx = threadIdx;

		// Wait for threads to be woken up (low CPU load sleep)
		std::unique_lock<std::mutex> lock(mSuspendedMutex);
		mNumSuspendedThreads++;
		mSuspendedCV.wait(lock, [&] {return !mSuspendThreads; });
		mNumSuspendedThreads--;
		lock.unlock();

		// Loop until suspended again
		while (!mSuspendThreads || !threadIsIdle)
		{
			if (mKillThreads)
				return;

			threadIsIdle = false;

			// Prio 1: Process any render jobs local to this thread
			unsigned int binIdx = threadBinIdx;
			threadBinIdx = threadBinIdx + mNumThreads < mNumBins ? threadBinIdx + mNumThreads : threadIdx;
			RenderJobQueue::Job *job = mRenderQueue->GetRenderJob(binIdx);
			if (job != nullptr)
			{
				if (job->mRenderJobs[binIdx].mTriIdx > 0)
					mMOC->RenderTrilist(job->mRenderJobs[binIdx], &mRects[binIdx]);

				mRenderQueue->AdvanceRenderJob(binIdx);
				continue;
			}

			// Prio 2: Process any outstanding setup/binning jobs
			if (mRenderQueue->CanBin())
			{
				// If no more rasterization jobs, get next binning job
				RenderJobQueue::Job *benningJob = mRenderQueue->GetBinningJob();
				if (benningJob != nullptr)
				{
					RenderJobQueue::BinningJob &sjob = benningJob->mBinningJob;
					for (unsigned int i = 0; i < mNumBins; ++i)
						benningJob->mRenderJobs[i].mTriIdx = 0;
					mMOC->BinTriangles(sjob.mVerts, sjob.mTris, sjob.nTris, benningJob->mRenderJobs, mBinsW, mBinsH, sjob.mMatrix, sjob.mClipPlanes, *sjob.mVtxLayout);
					mRenderQueue->FinishedBinningJob(benningJob);
				}
				continue;
			}

			// Prio 3: No work is available, work steal from another thread's queue
			if (mNumBins > mNumThreads)
			{
				binIdx = mRenderQueue->GetBestGlobalQueue();
				if (binIdx < mRenderQueue->mNumBins)
				{
					RenderJobQueue::Job *benningJob = mRenderQueue->GetRenderJob(binIdx);
					if (benningJob != nullptr)
					{
						if (benningJob->mRenderJobs[binIdx].mTriIdx > 0)
							mMOC->RenderTrilist(benningJob->mRenderJobs[binIdx], &mRects[binIdx]);

						mRenderQueue->AdvanceRenderJob(binIdx);
					}
					continue;
				}
			}

			// No work available: Yield this thread
			std::this_thread::yield();
			threadIsIdle = true;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Culling threadpool public API, similar to the MaskedOcclusionCulling class
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CullingThreadpool::CullingThreadpool(unsigned int numThreads, unsigned int binsW, unsigned int binsH, unsigned int maxJobs) :
	mNumThreads(numThreads),
	mKillThreads(false),
	mSuspendThreads(true),
	mNumSuspendedThreads(0),
	mBinsW(binsW),
	mBinsH(binsH),
	mMOC(nullptr),
	mVertexLayouts(maxJobs),
	mModelToClipMatrices(maxJobs),
	mMaxJobs(maxJobs)
{
	mNumBins = mBinsW*mBinsH;
	assert(mNumBins >= mNumThreads);	// Having less bins than threads is a bad idea!

	mRects = new ScissorRect[mNumBins];
	mRenderQueue = new RenderJobQueue(mNumBins, mMaxJobs);

	// Add default vertex layout and matrix
	mVertexLayouts.AddData(VertexLayout(16, 4, 12));
	mCurrentMatrix = nullptr;

	mThreads = new std::thread[mNumThreads];
	for (unsigned int i = 0; i < mNumThreads; ++i)
		mThreads[i] = std::thread(ThreadRun, this, i);

}

CullingThreadpool::~CullingThreadpool()
{
	// Wait for threads to terminate
	if (mThreads != nullptr || !mKillThreads)
	{
		mKillThreads = true;
		WakeThreads();
		for (unsigned int i = 0; i < mNumThreads; ++i)
			mThreads[i].join();

	}

	// Free memory
	SAFE_DELETE(mRenderQueue);
	SAFE_DELETE_ARRAY(mRects);
	SAFE_DELETE_ARRAY(mThreads);
}

void CullingThreadpool::WakeThreads()
{
	// Wait for all threads to be in suspended mode
	while (mNumSuspendedThreads < mNumThreads)
		std::this_thread::yield();

	// Send wake up event
	std::unique_lock<std::mutex> lock(mSuspendedMutex);
	mSuspendThreads = false;
	lock.unlock();
	mSuspendedCV.notify_all();
}

void CullingThreadpool::SuspendThreads()
{
	// Signal threads to go into suspended mode (after finishing all outstanding work)
	mSuspendThreads = true;
}

void CullingThreadpool::Flush()
{
	// Wait for pipeline to be empty (i.e. all work is finished)
	while (!mRenderQueue->IsPipelineEmpty())
		std::this_thread::yield();

	// Reset queue counters
	mRenderQueue->Reset();
}

void CullingThreadpool::SetBuffer(MaskedOcclusionCulling *moc)
{
	Flush();
	mMOC = moc;
	SetupScissors();
}

void CullingThreadpool::SetResolution(unsigned int width, unsigned int height)
{
	Flush();
	mMOC->SetResolution(width, height);
	SetupScissors();
}

void CullingThreadpool::SetNearClipPlane(float nearDist)
{
	Flush();
	mMOC->SetNearClipPlane(nearDist);
}

void CullingThreadpool::SetMatrix(const float *modelToClipMatrix)
{
	// Treat nullptr matrix as a special case, otherwise copy the contents of the pointer and add to state
	if (modelToClipMatrix == nullptr)
		mCurrentMatrix = nullptr;
	else
	{
		mModelToClipMatrices.AddData(Matrix4x4(modelToClipMatrix));
		mCurrentMatrix = mModelToClipMatrices.GetData()->mValues;
	}
}

void CullingThreadpool::SetVertexLayout(const VertexLayout &vtxLayout)
{
	mVertexLayouts.AddData(vtxLayout);
}

void CullingThreadpool::ClearBuffer()
{
	Flush();
	mMOC->ClearBuffer();
}

void CullingThreadpool::RenderTriangles(const float *inVtx, const unsigned int *inTris, int nTris, ClipPlanes clipPlaneMask)
{
	for (int i = 0; i < nTris; i += TRIS_PER_JOB)
	{
		// Yield if work queue is full 
		while (!mRenderQueue->CanWrite())
			std::this_thread::yield();

		// Create new renderjob
		RenderJobQueue::Job *job = mRenderQueue->GetWriteJob();
		job->mBinningJob.mVerts = inVtx;
		job->mBinningJob.mTris = inTris + i * 3;
		job->mBinningJob.nTris = nTris - i < TRIS_PER_JOB ? nTris - i : TRIS_PER_JOB;
		job->mBinningJob.mMatrix = mCurrentMatrix;
		job->mBinningJob.mClipPlanes = clipPlaneMask;
		job->mBinningJob.mVtxLayout = mVertexLayouts.GetData();
		mRenderQueue->AdvanceWriteJob();
	}
}

CullingThreadpool::CullingResult CullingThreadpool::TestRect(float xmin, float ymin, float xmax, float ymax, float wmin)
{
	return mMOC->TestRect(xmin, ymin, xmax, ymax, wmin);
}

CullingThreadpool::CullingResult CullingThreadpool::TestTriangles(const float *inVtx, const unsigned int *inTris, int nTris, ClipPlanes clipPlaneMask)
{
	return mMOC->TestTriangles(inVtx, inTris, nTris, mCurrentMatrix, clipPlaneMask, nullptr, *mVertexLayouts.GetData());
}

void CullingThreadpool::ComputePixelDepthBuffer(float *depthData)
{
	Flush();
	mMOC->ComputePixelDepthBuffer(depthData);
}
