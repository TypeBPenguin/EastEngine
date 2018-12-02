#include "stdafx.h"

#include "Graphics/Graphics.h"

int WINAPI WinMain(HINSTANCE hInstance,    //Main windows function
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nShowCmd)
{
#if defined(DEBUG) || defined(_DEBUG)
#define new new(_CLIENT_BLOCK, __FILE__, __LINE__)
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
	//_CrtSetBreakAlloc(377);
#endif

	try
	{
		const eastengine::graphics::APIs emApi = eastengine::graphics::eDX11;
		eastengine::graphics::Initialize(emApi, 800, 600, false, "DirectX12 Title", "DirectX12 Name");

		const eastengine::graphics::VertexPosTex vertices[] =
		{
			// front face
			{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f } },
			{ { 0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f } },
			{ { -0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f } },
			{ { 0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f } },

			// right side face
			{ { 0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f } },
			{ { 0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f } },
			{ { 0.5f, -0.5f,  0.5f }, { 1.0f, 1.0f } },
			{ { 0.5f,  0.5f, -0.5f }, { 0.0f, 0.0f } },

			// left side face
			{ { -0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f } },
			{ { -0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f } },
			{ { -0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f } },
			{ { -0.5f,  0.5f, -0.5f }, { 1.0f, 0.0f } },

			// back face
			{ { 0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f } },
			{ { -0.5f, -0.5f,  0.5f }, { 1.0f, 1.0f } },
			{ { 0.5f, -0.5f,  0.5f }, { 0.0f, 1.0f } },
			{ { -0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f } },

			// top face
			{ { -0.5f,  0.5f, -0.5f }, { 0.0f, 1.0f } },
			{ { 0.5f,  0.5f,  0.5f }, { 1.0f, 0.0f } },
			{ { 0.5f,  0.5f, -0.5f }, { 1.0f, 1.0f } },
			{ { -0.5f,  0.5f,  0.5f }, { 0.0f, 0.0f } },

			// bottom face
			{ { 0.5f, -0.5f,  0.5f }, { 0.0f, 0.0f } },
			{ { -0.5f, -0.5f, -0.5f }, { 1.0f, 1.0f } },
			{ { 0.5f, -0.5f, -0.5f }, { 0.0f, 1.0f } },
			{ { -0.5f, -0.5f,  0.5f }, { 1.0f, 0.0f } },
		};

		const uint32_t indices[] =
		{
			// front face
			0, 1, 2, // first triangle
			0, 3, 1, // second triangle

			// left face
			4, 5, 6, // first triangle
			4, 7, 5, // second triangle

			// right face
			8, 9, 10, // first triangle
			8, 11, 9, // second triangle

			// back face
			12, 13, 14, // first triangle
			12, 15, 13, // second triangle

			// top face
			16, 17, 18, // first triangle
			16, 19, 17, // second triangle

			// bottom face
			20, 21, 22, // first triangle
			20, 23, 21, // second triangle
		};

		eastengine::graphics::VertexBufferKey vKey = eastengine::graphics::CreateVertexBuffer(reinterpret_cast<const uint8_t*>(vertices), sizeof(vertices), _countof(vertices));
		eastengine::graphics::IndexBufferKey iKey = eastengine::graphics::CreateIndexBuffer(reinterpret_cast<const uint8_t*>(indices), sizeof(indices), _countof(indices));
		eastengine::graphics::TextureKey tKey = eastengine::graphics::CreateTexture("D:\\Projects\\Repos\\EastEngine\\Bin\\Data\\Texture\\gi_flag.tga");
		eastengine::graphics::TextureKey tKey2 = eastengine::graphics::CreateTexture("D:\\Projects\\Repos\\EastEngine\\Bin\\Data\\Texture\\uv_checker.png");

		struct Cube
		{
			eastengine::math::Matrix matWorld;
			eastengine::math::Matrix matRot;
			eastengine::math::float3 f3Position;
		};

		Cube cube1, cube2;
		cube2.f3Position = { 1.5f, 0.f, 0.f };
		cube2.matWorld = eastengine::math::Matrix::CreateTranslation(cube2.f3Position + cube1.f3Position);

		std::chrono::high_resolution_clock::time_point prevTime = std::chrono::high_resolution_clock::now();
		size_t nFrame = 0;
		float fTime = 0.f;

		eastengine::graphics::Run([&]()
		{
			std::chrono::high_resolution_clock::time_point curTime = std::chrono::high_resolution_clock::now();
			
			std::chrono::duration<double> duration = curTime - prevTime;
			float elapsedTime = static_cast<float>(duration.count());

			++nFrame;
			fTime += elapsedTime;

			if (fTime >= 1.f)
			{
				LOG_MESSAGE("FPS : %f", static_cast<float>(nFrame) / fTime);

				fTime -= 1.f;
				nFrame = 0;
			}

			prevTime = curTime;

			using namespace eastengine;

			eastengine::graphics::Cleanup();

			math::Matrix rotXMat = math::Matrix::CreateRotationX(1.f * elapsedTime);
			math::Matrix rotYMat = math::Matrix::CreateRotationY(2.f * elapsedTime);
			math::Matrix rotZMat = math::Matrix::CreateRotationZ(3.f * elapsedTime);

			cube1.matRot = cube1.matRot * rotXMat * rotYMat * rotZMat;

			math::Matrix translationMat;
			translationMat.Translation(cube1.f3Position);

			cube1.matWorld = cube1.matRot * translationMat;

			graphics::PushRenderJob(vKey, iKey, tKey, cube1.matWorld);

			rotXMat = math::Matrix::CreateRotationX(3.f * elapsedTime);
			rotYMat = math::Matrix::CreateRotationY(2.f * elapsedTime);
			rotZMat = math::Matrix::CreateRotationZ(1.f * elapsedTime);

			cube2.matRot = rotZMat * (cube2.matRot * (rotXMat * rotYMat));

			math::Matrix translationOffsetMat;
			translationOffsetMat.Translation(cube2.f3Position);

			math::Matrix scaleMat = math::Matrix::CreateScale(0.5f, 0.5f, 0.5f);

			cube2.matWorld = scaleMat * translationOffsetMat * cube2.matRot * translationMat;

			graphics::PushRenderJob(vKey, iKey, tKey2, cube2.matWorld);

			static int n = 0;
			if (n++ > 20000)
			{
				tKey2.reset();
			}
		});

		eastengine::graphics::Release();
		eastengine::String::Release();
	}
	catch (const std::exception& e)
	{
		OutputDebugString(e.what());
		LOG_ERROR(e.what());
		system("pause");
	}

	return 0;
}