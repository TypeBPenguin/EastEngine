#include "stdafx.h"
#include "Resource.h"

namespace EastEngine
{
	namespace Graphics
	{
		Resource::Resource()
			: m_nLife(0)
			, m_emLoadState(EmLoadState::eReady)
		{
		}

		Resource::~Resource()
		{
		}
	}
}