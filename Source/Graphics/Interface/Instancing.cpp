#include "stdafx.h"
#include "Instancing.h"

namespace est
{
	namespace graphics
	{
		TransformInstancingData::TransformInstancingData(const math::Matrix& f4x4World)
		{
			EncodeMatrix(f4x4World);
		}

		MotionInstancingData::MotionInstancingData(uint32_t nVTFOffset)
			: nVTFOffset(nVTFOffset)
		{
		}
		
		SkinningInstancingData::SkinningInstancingData(const math::Matrix& matWorld, MotionInstancingData motionData)
			: worldData(matWorld), motionData(motionData)
		{
		}
	}
}