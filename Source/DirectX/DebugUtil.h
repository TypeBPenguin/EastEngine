#pragma once

namespace EastEngine
{
	namespace Graphics
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