#include "stdafx.h"
#include "DepthStencil.h"

namespace EastEngine
{
	namespace Graphics
	{
		static int s_nDepthStencilCount = 0;

		DepthStencil::DepthStencil()
			: m_pTexture(nullptr)
			, m_pDepthStencilView(nullptr)
		{
		}

		DepthStencil::~DepthStencil()
		{
			m_pTexture.reset();

			SafeRelease(m_pDepthStencilView);
		}

		bool DepthStencil::Init(const DepthStencilDesc& depthStencilDesc)
		{
			String::StringID strName;
			strName.Format("EastEngine_DepthStencil_%d", s_nDepthStencilCount);

			m_pTexture = ITexture::Create(strName, depthStencilDesc);
			if (m_pTexture == nullptr)
				return false;

			if (FAILED(GetDevice()->CreateDepthStencilView(m_pTexture->GetTexture2D(), depthStencilDesc.GetDSVDescPtr(), &m_pDepthStencilView)))
			{
				m_pTexture.reset();
				return false;
			}

			++s_nDepthStencilCount;

			return true;
		}
	}
}