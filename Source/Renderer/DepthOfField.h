#pragma once

#include "CommonLib/Singleton.h"

namespace eastengine
{
	namespace graphics
	{
		class Camera;

		class IEffect;
		class IEffectTech;

		class ISamplerState;
		
		class DepthOfField : public Singleton<DepthOfField>
		{
			friend Singleton<DepthOfField>;
		public:
			struct Setting
			{
				float fFocalDistnace = 5.f;
				float fFocalWidth = 20.f;
			};

		private:
			DepthOfField();
			virtual ~DepthOfField();

		public:
			bool Init();
			void Release();

		public:
			bool Apply(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, IRenderTarget* pResult, IRenderTarget* pSource, const std::shared_ptr<ITexture>& pDepth);

		public:
			Setting& GetSetting() { return m_setting; }

		private:
			void ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pTech);

		private:
			bool m_isInit;

			Setting m_setting;

			IEffect* m_pEffect;

			ISamplerState* m_pSamplerPoint;
			ISamplerState* m_pSamplerLinear;
		};
	}
}