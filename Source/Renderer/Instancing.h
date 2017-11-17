#pragma once

namespace EastEngine
{
	namespace Graphics
	{
		static const uint32_t MAX_INSTANCE_NUM = 128;

		struct InstStaticData
		{
			Math::Vector4 f4World1;
			Math::Vector4 f4World2;
			Math::Vector4 f4World3;
			Math::Vector4 f4Padding;

			InstStaticData();
			InstStaticData(const Math::Matrix& f4x4World);

			void EncodeMatrix(const Math::Matrix& f4x4World)
			{
				f4World1 = Math::Vector4(f4x4World._11, f4x4World._12, f4x4World._13, f4x4World._41);
				f4World2 = Math::Vector4(f4x4World._21, f4x4World._22, f4x4World._23, f4x4World._42);
				f4World3 = Math::Vector4(f4x4World._31, f4x4World._32, f4x4World._33, f4x4World._43);
			}

			Math::Matrix DecodeMatrix()
			{
				return Math::Matrix(Math::Vector4(f4World1.x, f4World1.y, f4World1.z, 0.f),
					Math::Vector4(f4World2.x, f4World2.y, f4World2.z, 0.f),
					Math::Vector4(f4World3.x, f4World3.y, f4World3.z, 0.f),
					Math::Vector4(f4World1.w, f4World2.w, f4World3.w, 1.f));
			}
		};

		struct InstMotionData
		{
			uint32_t nVTFOffset = 0;
			uint32_t nDummy0 = 0;
			uint32_t nDummy1 = 0;
			uint32_t nDummy2 = 0;

			InstMotionData();
			InstMotionData(uint32_t nVTFOffset);
		};

		struct InstSkinnedData
		{
			InstStaticData worldData;
			InstMotionData motionData;

			InstSkinnedData();
			InstSkinnedData(const Math::Matrix& matWorld, InstMotionData motionData);
		};

		struct InstUIData
		{
			/*
			x : Left
			y : Right
			z : Top
			w : Bottom
			*/
			Math::Vector4 rect;
			Math::Vector4 uv;
			Math::Color color;

			float fDepth;
		};
	}
}