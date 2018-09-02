#pragma once

#include "GraphicsInterface/RenderJob.h"

namespace eastengine
{
	namespace graphics
	{
		class Camera;

		namespace dx11
		{
			class EnvironmentRenderer
			{
			public:
				EnvironmentRenderer();
				~EnvironmentRenderer();

			public:
				void Render(ID3D11Device* pDevice, ID3D11DeviceContext* pDeviceContext, Camera* pCamera);
				void Flush();

			private:
				class Impl;
				std::unique_ptr<Impl> m_pImpl;
			};
		}
	}
}