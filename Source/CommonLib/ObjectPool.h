#pragma once

namespace est
{
	namespace memory
	{
		template <typename T, size_t ObjectCountPerChunk>
		class ObjectPool
		{
		private:
			struct ObjectNode
			{
				char* pObject{ nullptr };
				ObjectNode* pNext{ nullptr };
			};

			struct Chunk
			{
				char pool[sizeof(T) * ObjectCountPerChunk]{ 0 };
				bool allocateMarking[ObjectCountPerChunk]{ false };
				ObjectNode objectNodes[ObjectCountPerChunk];

				ObjectNode* pFreeNodes{ nullptr };
				ObjectNode* pAllocatedNodes{ nullptr };
				size_t allocateCount{ 0 };

				Chunk* pPrev{ nullptr };
				Chunk* pNext{ nullptr };

				void Initialize()
				{
					const size_t lastIndex = ObjectCountPerChunk - 1;
					for (size_t i = 0; i < lastIndex; ++i)
					{
						objectNodes[i].pObject = &pool[sizeof(T) * i];
						objectNodes[i].pNext = &objectNodes[i + 1];
					}

					objectNodes[lastIndex].pObject = &pool[sizeof(T) * lastIndex];
					objectNodes[lastIndex].pNext = nullptr;

					pFreeNodes = &objectNodes[0];

					memset(allocateMarking, 0, sizeof(allocateMarking));

					pAllocatedNodes = nullptr;
					allocateCount = 0;
				}

				void Release()
				{
					for (ObjectNode* pNode = pAllocatedNodes; pNode != nullptr; pNode = pNode->pNext)
					{
						reinterpret_cast<T*>(pAllocatedNodes->pObject)->~T();
					}
					Initialize();
				}

				char* Pop()
				{
					ObjectNode* pAllocateNode = pFreeNodes;
					pFreeNodes = pFreeNodes->pNext;

					char* pFreeObject = pAllocateNode->pObject;
					pAllocateNode->pObject = nullptr;

					pAllocateNode->pNext = pAllocatedNodes;
					pAllocatedNodes = pAllocateNode;

					const size_t distance = (pFreeObject - pool);
					const size_t index = distance / sizeof(T);
					allocateMarking[index] = true;

					++allocateCount;

					return pFreeObject;
				}

				void Push(char* p)
				{
					ObjectNode* pFreeNode = pAllocatedNodes;
					pAllocatedNodes = pAllocatedNodes->pNext;

					pFreeNode->pObject = p;
					pFreeNode->pNext = pFreeNodes;
					pFreeNodes = pFreeNode;

					const size_t distance = (p - pool);
					const size_t index = distance / sizeof(T);
					allocateMarking[index] = false;

					--allocateCount;
				}

				bool IsInside(char* p) const
				{
					return pool <= p && p < (pool + sizeof(T) * ObjectCountPerChunk);
				}

				bool IsValid(char* p) const
				{
					if (IsInside(p) == false)
						return false;

					const size_t distance = (p - pool);
					if (distance % sizeof(T) != 0)
						return false;

					const size_t index = distance / sizeof(T);
					return allocateMarking[index];
				}
			};

		public:
			ObjectPool()
			{
			}

			~ObjectPool()
			{
				if (m_totalAllocateCount > 0)
				{
					LOG_ERROR(L"[memory leak] left object count : %I64u", m_totalAllocateCount);
				}

				Chunk* pChunk = m_pChunks;
				while (pChunk != nullptr)
				{
					pChunk->Release();

					Chunk* pDeleteChunk = pChunk;
					pChunk = pChunk->pNext;
					delete pDeleteChunk;
				}
			}

		public:
			template <typename... Args>
			T* Allocate(Args&&... args)
			{
				Chunk* pChunk = nullptr;
				for (Chunk* pFreeChunk = m_pChunks; pFreeChunk != nullptr; pFreeChunk = pFreeChunk->pNext)
				{
					if (pFreeChunk->pFreeNodes != nullptr)
					{
						pChunk = pFreeChunk;
						break;
					}
				}

				if (pChunk == nullptr)
				{
					pChunk = CreateChunk();
				}

				char* pObjectPointer = pChunk->Pop();
				++m_totalAllocateCount;

				return new(pObjectPointer) T(std::forward<Args>(args)...);
			}

			void Destroy(T* pObject)
			{
				Chunk* pChunk = FindChunk(pObject);
				if (pChunk == nullptr ||
					pChunk->allocateCount == 0)
					return;

				char* p = reinterpret_cast<char*>(pObject);
				if (pChunk->IsValid(p) == false)
				{
					LOG_ERROR(L"already destroyed object or unknown bad object, please check the logic that calls this function.");
					return;
				}

				pObject->~T();
				pChunk->Push(reinterpret_cast<char*>(p));
				--m_totalAllocateCount;

				if (pChunk->allocateCount == 0)
				{
					ReleaseEmptyChunk(1);
				}
			}

			void ReleaseEmptyChunk(size_t remainEmptyChunkCount)
			{
				size_t emptyChunkCount = 0;
				for (Chunk* pChunk = m_pChunks; pChunk != nullptr; pChunk = pChunk->pNext)
				{
					if (pChunk->allocateCount == 0)
					{
						++emptyChunkCount;
					}
				}

				if (emptyChunkCount >= remainEmptyChunkCount)
					return;

				Chunk* pChunk = m_pChunks;
				while (pChunk != nullptr)
				{
					if (emptyChunkCount >= remainEmptyChunkCount)
						return;

					if (pChunk->allocateCount == 0)
					{
						Chunk* pDeleteChunk = pChunk;
						if (pChunk->pPrev != nullptr)
						{
							pChunk->pPrev->pNext = pChunk->pNext;
						}

						if (pChunk->pNext != nullptr)
						{
							pChunk->pNext->pPrev = pChunk->pPrev;
						}
						pChunk = pChunk->pNext;

						if (m_pChunks == pDeleteChunk)
						{
							m_pChunks = pChunk;
						}

						pDeleteChunk->Release();
						delete pDeleteChunk;

						--emptyChunkCount;
						--m_chunkCount;
					}
					else
					{
						pChunk = pChunk->pNext;
					}
				}
			}

		public:
			size_t GetChunkCount() const { return m_chunkCount; }
			size_t GetAllocateCount() const { return m_totalAllocateCount; }

		private:
			Chunk* CreateChunk()
			{
				Chunk* pChunk = new Chunk;
				pChunk->Initialize();

				++m_chunkCount;

				if (m_pChunks == nullptr)
				{
					m_pChunks = pChunk;
				}
				else
				{
					Chunk* pLastChunk = m_pChunks;
					while (true)
					{
						if (pLastChunk->pNext == nullptr)
						{
							pLastChunk->pNext = pChunk;
							pChunk->pPrev = pLastChunk;
							break;
						}
						else
						{
							pLastChunk = pLastChunk->pNext;
						}
					}
				}

				return pChunk;
			}

			Chunk* FindChunk(T* pObject)
			{
				char* p = reinterpret_cast<char*>(pObject);
				for (Chunk* pChunk = m_pChunks; pChunk != nullptr; pChunk = pChunk->pNext)
				{
					if (pChunk->IsInside(p) == true)
						return pChunk;
				}
				return nullptr;
			}

		private:
			Chunk* m_pChunks{ nullptr };
			size_t m_chunkCount{ 0 };

			size_t m_totalAllocateCount{ 0 };
		};
	}
}