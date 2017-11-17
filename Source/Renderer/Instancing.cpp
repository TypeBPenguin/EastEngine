#include "stdafx.h"
#include "Instancing.h"

namespace EastEngine
{
	namespace Graphics
	{
		InstStaticData::InstStaticData()
		{
		}

		InstStaticData::InstStaticData(const Math::Matrix& f4x4World)
		{
			EncodeMatrix(f4x4World);
		}

		InstMotionData::InstMotionData()
		{
		}

		InstMotionData::InstMotionData(uint32_t nVTFOffset)
			: nVTFOffset(nVTFOffset)
		{
		}
		
		InstSkinnedData::InstSkinnedData()
		{
		}

		InstSkinnedData::InstSkinnedData(const Math::Matrix& matWorld, InstMotionData motionData)
			: worldData(matWorld), motionData(motionData)
		{
		}
	}
}