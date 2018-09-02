#include "stdafx.h"
#include "GraphicsInterface.h"

namespace eastengine
{
	namespace graphics
	{
		static Options s_options;

		const Options& GetOptions()
		{
			return s_options;
		}

		void SetOptions(const Options& options)
		{
			s_options = options;
		}
	}
}