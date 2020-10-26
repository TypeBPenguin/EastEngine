#pragma once

namespace est
{
	namespace graphics
	{
		enum
		{
			eMaxInstancingCount = 64,
		};
		
		struct TransformInstancingData
		{
			math::float4 f4World1;
			math::float4 f4World2;
			math::float4 f4World3;

			TransformInstancingData() = default;
			TransformInstancingData(const math::Matrix& f4x4World);

			void EncodeMatrix(const math::Matrix& f4x4World)
			{
				f4World1 = math::float4(f4x4World._11, f4x4World._12, f4x4World._13, f4x4World._41);
				f4World2 = math::float4(f4x4World._21, f4x4World._22, f4x4World._23, f4x4World._42);
				f4World3 = math::float4(f4x4World._31, f4x4World._32, f4x4World._33, f4x4World._43);
			}

			math::Matrix DecodeMatrix()
			{
				return math::Matrix(math::float4(f4World1.x, f4World1.y, f4World1.z, 0.f),
					math::float4(f4World2.x, f4World2.y, f4World2.z, 0.f),
					math::float4(f4World3.x, f4World3.y, f4World3.z, 0.f),
					math::float4(f4World1.w, f4World2.w, f4World3.w, 1.f));
			}
		};
		static_assert(sizeof(TransformInstancingData) == (sizeof(math::float4) * 3), "something happends!!");

		struct MotionInstancingData
		{
			uint32_t nVTFOffset = 0;
			uint32_t nDummy0 = 0;
			uint32_t nDummy1 = 0;
			uint32_t nDummy2 = 0;

			MotionInstancingData() = default;
			MotionInstancingData(uint32_t nVTFOffset);
		};

		struct SkinningInstancingData
		{
			TransformInstancingData worldData;
			MotionInstancingData motionData;

			SkinningInstancingData() = default;
			SkinningInstancingData(const math::Matrix& matWorld, MotionInstancingData motionData);
		};
	}
}