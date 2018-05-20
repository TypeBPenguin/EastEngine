#pragma once

namespace eastengine
{
	namespace graphics
	{
		class Camera;

		namespace dx11
		{
			class DeferredRenderer
			{
			public:
				DeferredRenderer();
				~DeferredRenderer();

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