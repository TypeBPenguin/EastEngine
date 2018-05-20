#pragma once

namespace eastengine
{
	namespace graphics
	{
		class IVertexBuffer;
		class IIndexBuffer;

		namespace Debug
		{
			void Init();
			void Release();

			IVertexBuffer* GetLineBoxVertexBuffer();
			IIndexBuffer* GetLineBoxIndexBuffer();
		}
	}
}