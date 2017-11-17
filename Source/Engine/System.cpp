#include "stdafx.h"
#include "System.h"

#include "FpsChecker.h"
#include "SceneMgr.h"

#include "../CommonLib/CommandLine.h"
#include "../CommonLib/DirectoryMonitor.h"
#include "../CommonLib/Timer.h"

#include "../CommonLib/PipeStream.h"

#include "../DirectX/Device.h"
#include "../LuaSystem/LuaSystem.h"
#include "../Input/InputDevice.h"
#include "../Physics/PhysicsSystem.h"
#include "../SoundSystem/SoundSystem.h"
#include "../GameObject/ActorManager.h"
#include "../UI/UIMgr.h"

#include "../GameObject/ComponentModel.h"

#include <ppl.h>

namespace EastEngine
{
	MainSystem::MainSystem()
		: m_pFpsChecker(nullptr)
		, s_pDirectoryMonitor(nullptr)
		, s_pTimer(nullptr)
		, s_pLuaSystem(nullptr)
		, s_pSceneMgr(nullptr)
		, s_pActorMgr(nullptr)
		, s_pWindows(nullptr)
		, s_pGraphicsSystem(nullptr)
		, s_pInputDevice(nullptr)
		, s_pPhysicsSystem(nullptr)
		, s_pSoundSystem(nullptr)
		, s_pUIMgr(nullptr)
		, m_isFullScreen(false)
		, m_isVsync(false)
		, m_isInit(false)
	{	
	}

	MainSystem::~MainSystem()
	{
		Release();
	}

	bool MainSystem::Init(const String::StringID& strApplicationName, uint32_t nScreenWidth, uint32_t nScreenHeight, bool isFullScreen, bool isVsync)
	{
		if (m_isInit == true)
			return true;

		m_isInit = true;

		s_pDirectoryMonitor = File::DirectoryMonitor::GetInstance();
		if (s_pDirectoryMonitor->Init() == false)
		{
			Release();
			return false;
		}

		m_strApplicationName = strApplicationName;
		m_n2ScreenSize = Math::Int2(nScreenWidth, nScreenHeight);
		m_isFullScreen = isFullScreen;;
		m_isVsync = isVsync;;

		s_pCommandLine = Config::SCommandLine::GetInstance();
		if (s_pCommandLine->Init() == false)
		{
			Release();
			return false;
		}

		s_pTimer = Timer::GetInstance();

		s_pWindows = Windows::WindowsManager::GetInstance();

		if (s_pWindows->Init(strApplicationName.c_str(), nScreenWidth, nScreenHeight, isFullScreen) == false)
		{
			Release();
			return false;
		}

		HWND hWnd = s_pWindows->GetHwnd();
		HINSTANCE hInstance = s_pWindows->GetHInstance();

		//s_pPipeStream = PipeStreamInst;
		//if (s_pPipeStream->InitServer("neEngine_A", 1) == false)
		//{
		//	Release();
		//	return false;
		//}

		s_pGraphicsSystem = Graphics::GraphicsSystem::GetInstance();
		if (s_pGraphicsSystem->Init(hWnd, m_n2ScreenSize.x, m_n2ScreenSize.y, m_isFullScreen, m_isVsync, 1.f) == false)
		{
			Release();
			return false;
		}

		s_pWindows->AddMessageHandler([](HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam) -> bool
		{
			return Graphics::GraphicsSystem::GetInstance()->GetD3D()->HandleMsg(hWnd, nMsg, wParam, lParam);
		});
		
		s_pInputDevice = Input::InputDevice::GetInstance();
		if (s_pInputDevice->Init(hInstance, hWnd) == false)
		{
			Release();
			return false;
		}

		s_pWindows->AddMessageHandler([](HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam) -> bool
		{
			return Input::InputDevice::GetInstance()->HandleMsg(hWnd, nMsg, wParam, lParam);
		});

		s_pPhysicsSystem = Physics::PhysicsSystem::GetInstance();
		if (s_pPhysicsSystem->Init() == false)
		{
			Release();
			return false;
		}

		s_pSoundSystem = Sound::SoundSystem::GetInstance();
		if (s_pSoundSystem->Init() == false)
		{
			Release();
			return false;
		}

		//s_pLuaSystem = LuaSystemInst;
		//if (s_pLuaSystem->Init() == false)
		//{
		//	Release();
		//	return false;
		//}

		s_pSceneMgr = SSceneMgr::GetInstance();
		if (s_pSceneMgr->Init() == false)
		{
			Release();
			return false;
		}

		s_pUIMgr = UI::UIManager::GetInstance();
		if (s_pUIMgr->Init(s_pWindows->GetHwnd()) == false)
		{
			Release();
			return false;
		}
		
		s_pWindows->AddMessageHandler([](HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam) -> bool
		{
			return UI::UIManager::GetInstance()->HandleMsg(hWnd, nMsg, wParam, lParam);
		});

		s_pActorMgr = GameObject::ActorManager::GetInstance();

		m_pFpsChecker = new FpsChecker;
		
		return true;
	}

	void MainSystem::Release()
	{
		if (m_isInit == false)
			return;

		SafeDelete(m_pFpsChecker);

		SafeRelease(s_pSceneMgr);
		SSceneMgr::DestroyInstance();

		SafeRelease(s_pActorMgr);
		GameObject::ActorManager::DestroyInstance();

		SafeRelease(s_pUIMgr);
		UI::UIManager::DestroyInstance();

		SafeRelease(s_pSoundSystem);
		Sound::SoundSystem::DestroyInstance();

		SafeRelease(s_pPhysicsSystem);
		Physics::PhysicsSystem::DestroyInstance();

		SafeRelease(s_pInputDevice);
		Input::InputDevice::DestroyInstance();

		SafeRelease(s_pGraphicsSystem);
		Graphics::GraphicsSystem::DestroyInstance();

		SafeRelease(s_pLuaSystem);
		Lua::LuaSystem::DestroyInstance();

		//if (s_pPipeStream != nullptr)
		//{
		//	CPipeData data(EmPipeStream::PIPE_AS_CLOSE);
		//	s_pPipeStream->PushMessage(data);
		//}
		//
		//SafeRelease(s_pPipeStream);
		//PipeStreamRelease;

		SafeRelease(s_pWindows);
		Windows::WindowsManager::DestroyInstance();

		s_pTimer = nullptr;
		Timer::DestroyInstance();

		s_pCommandLine = nullptr;
		Config::SCommandLine::DestroyInstance();

		SafeRelease(s_pDirectoryMonitor);
		File::DirectoryMonitor::DestroyInstance();

		String::Release();

		m_isInit = true;
	}

	void MainSystem::Run()
	{
		MSG msg;
		Memory::Clear(&msg, sizeof(msg));

		bool isDone = false;

		// 유저로부터 종료 메세지를 받을 때까지 루프
		while (isDone == false)
		{
			if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}

			// 윈도우에서 어플리케이션의 종료를 요청하는 경우 종료
			if (msg.message == WM_QUIT)
			{
				isDone = true;
			}
			else
			{
				s_pTimer->Tick();
				float fElapsedTime = s_pTimer->GetDeltaTime();
				
				m_pFpsChecker->Update(fElapsedTime);

				s_pWindows->ProcessMessages();

				flush(fElapsedTime);

				update(fElapsedTime);
				render();
			}
		}
	}

	bool MainSystem::HandleMsg(HWND hWnd, uint32_t nMsg, WPARAM wParam, LPARAM lParam)
	{
		return false;
	}

	float MainSystem::GetFPS()
	{
		return m_pFpsChecker->GetFps();
	}

	void MainSystem::flush(float fElapsedTime)
	{
		s_pGraphicsSystem->Flush(fElapsedTime);
	}

	void MainSystem::update(float fElapsedTime)
	{
		s_pDirectoryMonitor->Update();
		
		s_pInputDevice->Update(fElapsedTime);
		
		s_pSoundSystem->Update(fElapsedTime);
		s_pPhysicsSystem->Update(fElapsedTime);

		//Concurrency::parallel_invoke
		//(
		//	[&] { s_pDirectoryMonitor->Update(); },
		//	[&] { s_pInputDevice->Update(fElapsedTime); },
		//	[&] { s_pSoundSystem->Update(fElapsedTime); },
		//	[&] { s_pPhysicsSystem->Update(fElapsedTime); }
		//);

		s_pActorMgr->Update(fElapsedTime);

		s_pGraphicsSystem->Update(fElapsedTime);
		s_pUIMgr->Update(fElapsedTime);

		s_pSceneMgr->Update(fElapsedTime);
	}

	void MainSystem::render()
	{
		s_pGraphicsSystem->BeginScene(0.f, 0.f, 0.f, 1.f);

		s_pGraphicsSystem->Render();

		s_pGraphicsSystem->EndScene();
	}
	
	void MainSystem::processPipeMessage()
	{
		//CPipeData pipeData;
		//if (s_pPipeStream->PopMessage(pipeData) == true)
		//{
		//	switch (pipeData.GetHeader().nDataType)
		//	{
		//	case EmPipeStream::PIPE_SA_CONNECT:
		//	{
		//		const char* strClientName = pipeData.ReadString();

		//		if (s_pPipeStream->InitClient() == false)
		//		{
		//			_LOG("실패닷");
		//		}
		//		else
		//		{
		//			CPipeData data(EmPipeStream::PIPE_AS_CONNECT);
		//			s_pPipeStream->PushMessage(data);

		//			_LOG("Connect PipeClient : %s", strClientName);
		//		}
		//	}
		//	break;
		//	case EmPipeStream::PIPE_SA_DISCONNECT:
		//	{
		//		_LOG("DisConnect PipeClient");

		//		s_pPipeStream->CloseClient();
		//	}
		//	break;
		//	case EmPipeStream::PIPE_SA_REQUEST_ACTOR_LIST:
		//	{
		//		_LOG("Request Actor List");

		//		CPipeData actorListData(EmPipeStream::PIPE_AS_ANSWER_ACTOR_LIST, 4096);
		//		auto& listActor = s_pActorMgr->GetActorList();
		//		actorListData.Write(listActor.size());

		//		for (auto& pActor : listActor)
		//		{
		//			actorListData.WriteString(pActor->GetActorName().c_str());
		//		}
		//		s_pPipeStream->PushMessage(actorListData);
		//	}
		//	break;
		//	case EmPipeStream::PIPE_SA_REQUEST_ACTOR_DATA:
		//	{
		//		const char* strName = pipeData.ReadString();
		//		auto pActor = s_pActorMgr->GetActor(strName);
		//		if (pActor == nullptr)
		//			return;

		//		GameObject::CComponentModel* pCompModel = nullptr;
		//		pActor->GetComponent(pCompModel);

		//		if (pCompModel == nullptr || pCompModel->GetModel()->IsLoadComplete() == true)
		//			return;

		//		Graphics::VecModelNode& vecNode = pCompModel->GetModel()->GetModelNodeList();
		//		if (vecNode.empty())
		//			break;

		//		auto pModel = pCompModel->GetModel();
		//		
		//		auto WriteVector3 = [](CPipeData& pipeData, const Vector3& f3)
		//		{
		//			pipeData.Write(f3.x);
		//			pipeData.Write(f3.y);
		//			pipeData.Write(f3.z);
		//		};

		//		auto WriteVector4 = [](CPipeData& pipeData, const Vector4& f4)
		//		{
		//			pipeData.Write(f4.x);
		//			pipeData.Write(f4.y);
		//			pipeData.Write(f4.z);
		//			pipeData.Write(f4.w);
		//		};

		//		auto WriteQuat = [](CPipeData& pipeData, const Quaternion& quat)
		//		{
		//			pipeData.Write(quat.x);
		//			pipeData.Write(quat.y);
		//			pipeData.Write(quat.z);
		//			pipeData.Write(quat.w);
		//		};

		//		CPipeData nodeData(EmPipeStream::PIPE_AS_ANSWER_ACTOR_DATA);
		//		WriteVector3(nodeData, pModel->GetBasePos());
		//		WriteVector3(nodeData, pModel->GetBaseScale());
		//		WriteQuat(nodeData, pModel->GetBaseRot());

		//		nodeData.WriteString(pModel->GetModelName().c_str());
		//		nodeData.WriteString(pModel->GetFilePath().c_str());

		//		std::function<void(VecModelNode&, CPipeData&)> SetModelNodeInfo = [&](VecModelNode& vecNode, CPipeData& nodeData)
		//		{
		//			if (vecNode.empty())
		//			{
		//				nodeData.Write(-1);
		//				return;
		//			}

		//			uint32_t nSize = vecNode.size();
		//			nodeData.Write(nSize);

		//			for (uint32_t i = 0; i < nSize; ++i)
		//			{
		//				nodeData.Write((int)(vecNode[i]->GetType()));
		//				nodeData.Write(vecNode[i]->IsVisible());

		//				/*nodeData.WriteString(vecNode[i]->GetNodeName().c_str());
		//				nodeData.WriteString(vecNode[i]->GetParentNodeName().c_str());
		//				nodeData.Write(vecNode[i]->GetMaterial().size());

		//				for (auto& pMaterial : vecNode[i]->GetMaterial())
		//				{
		//					nodeData.WriteString(pMaterial->GetName().c_str());
		//					nodeData.WriteString(pMaterial->GetTexPath());

		//					WriteVector4(nodeData, pMaterial->GetAlbedoColor());
		//					WriteVector4(nodeData, pMaterial->GetEmissiveColor());
		//					WriteVector4(nodeData, pMaterial->GetDisRoughMetEmi());
		//					WriteVector4(nodeData, pMaterial->GetSurSpecTintAniso());
		//					WriteVector4(nodeData, pMaterial->GetSheenTintClearcoatGloss());

		//					nodeData.Write(pMaterial->IsTessellation());

		//					nodeData.WriteString(pMaterial->GetTexAlbedoName().c_str());
		//					nodeData.WriteString(pMaterial->GetTexMaskName().c_str());
		//					nodeData.WriteString(pMaterial->GetTexNormalMapName().c_str());
		//					nodeData.WriteString(pMaterial->GetTexSpecularColorName().c_str());
		//					nodeData.WriteString(pMaterial->GetTexDisplacementName().c_str());
		//					nodeData.WriteString(pMaterial->GetTexRoughnessName().c_str());
		//					nodeData.WriteString(pMaterial->GetTexMetalicName().c_str());
		//					nodeData.WriteString(pMaterial->GetTexEmissiveName().c_str());
		//					nodeData.WriteString(pMaterial->GetTexSurfaceName().c_str());
		//					nodeData.WriteString(pMaterial->GetTexSpecularName().c_str());
		//					nodeData.WriteString(pMaterial->GetTexSpecularTintName().c_str());
		//					nodeData.WriteString(pMaterial->GetTexAnisotropicName().c_str());
		//					nodeData.WriteString(pMaterial->GetTexSheenName().c_str());
		//					nodeData.WriteString(pMaterial->GetTexSheenTintName().c_str());
		//					nodeData.WriteString(pMaterial->GetTexClearcoatName().c_str());
		//					nodeData.WriteString(pMaterial->GetTexClearcoatGlossName().c_str());

		//					nodeData.Write(pMaterial->GetMaterialType());
		//					nodeData.Write(pMaterial->GetRenderType());
		//				}*/

		//				SetModelNodeInfo(vecNode[i]->GetChildList(), nodeData);
		//			}
		//		};

		//		SetModelNodeInfo(vecNode, nodeData);

		//		s_pPipeStream->PushMessage(nodeData);
		//	}
		//	break;
		//	case EmPipeStream::PIPE_SA_REQUEST_MODEL_DATA:
		//	{
		//		const char* strActorName = pipeData.ReadString();
		//		const char* strNodeName = pipeData.ReadString();

		//	}
		//	break;
		//	case EmPipeStream::PIPE_SA_MODELLOAD:
		//	{
		//		const char* strFileName = pipeData.ReadString();
		//	
		//		std::shared_ptr<GameObject::Actor> pActor = ActorMgrInst->CreateActor(File::GetFileNameWithoutExtension(strFileName).c_str());
		//		GameObject::CComponentModel* pCompModel = nullptr;
		//		pActor->AddComponent(pCompModel);
		//	
		//		std::shared_ptr<IModel> pModel = ModelMgrInst->LoadModelFromFBXFile(pActor->GetActorName(), strFileName, 100.f);
		//		pCompModel->Init(pModel);

		//		if (pModel->IsLoadComplete() == false)
		//		{
		//			pActor->SetNeedDelete(true);
		//		}
		//	}
		//	break;
		//	case EmPipeStream::PIPE_SA_TEST:
		//	{
		//		int n1 = pipeData.Read<int>();
		//		int n2 = pipeData.Read<int>();
		//		int n3 = pipeData.Read<int>();

		//		_LOG("%d, %d, %d", n1, n2, n3);

		//		CPipeData data(EmPipeStream::PIPE_AS_TEST);
		//		data.Write(123);
		//		data.Write(2345);
		//		data.Write(34567);
		//		data.Write(456789);
		//		data.Write(5678901);
		//		s_pPipeStream->PushMessage(data);
		//	}
		//	break;
		//	}
		//}
	}
}