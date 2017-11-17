#include "stdafx.h"
#include "ISpriteFont.h"

#include "D3DInterface.h"
#include "_SpriteFont.h"

namespace EastEngine
{
	namespace Graphics
	{
		ISpriteFont::ISpriteFont()
		{
		}

		ISpriteFont::~ISpriteFont()
		{
		}

		ISpriteFont* ISpriteFont::Create(const char* strFileName)
		{
			return new CSpriteFont(strFileName);
		}

		ISpriteFont* ISpriteFont::Create(const uint8_t* pDataBlob, uint32_t nDataSize)
		{
			return new CSpriteFont(pDataBlob, nDataSize);
		}

		ISpriteFont* ISpriteFont::Create(const std::shared_ptr<ITexture>& pTexture, const Glyph* pGlyphs, uint32_t nGlyphCount, float fLineSpacing)
		{
			return new CSpriteFont(pTexture, pGlyphs, nGlyphCount, fLineSpacing);
		}
	}
}