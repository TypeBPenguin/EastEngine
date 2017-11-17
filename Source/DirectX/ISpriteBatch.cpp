#include "stdafx.h"
#include "ISpriteBatch.h"

#include "_SpriteBatch.h"

namespace EastEngine
{
	namespace Graphics
	{
		ISpriteBatch::ISpriteBatch()
		{
		}

		ISpriteBatch::~ISpriteBatch()
		{
		}

		ISpriteBatch* ISpriteBatch::Create(ID3D11DeviceContext* pd3dDeviceContext)
		{
			CSpriteBatch* pSpriteBatch = new CSpriteBatch(pd3dDeviceContext);
			return pSpriteBatch;
		}
	}
}