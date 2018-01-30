#include "stdafx.h"
#include <iostream>

#include "CommonLib/FileUtil.h"

#include "Windows/Windows.h"
#include "DirectX/Device.h"
#include "Model/ModelManager.h"
#include "Model/MotionManager.h"

using namespace EastEngine;

enum EmFlag
{
	eModel = 1,
	eMotion = 2,
};

int main(int argn, char** argc)
{
	if (argn <= 1)
	{
		std::cout << "fbx 파일을 드래그드랍 해주세요." << std::endl;
		system("pause");

		return 0;
	}

	std::cout << "추출할 데이터를 선택해주세요." << std::endl;
	std::cout << "1. Model" << std::endl;
	std::cout << "2. Motion" << std::endl;
	std::cout << "3. Model & Motion" << std::endl;

	int nSelect = 0;
	std::cin >> nSelect;
	std::cout << std::endl;

	int nFlag = 0;
	std::string strLoadType;

	switch (nSelect)
	{
	case 1:
		nFlag = eModel;
		strLoadType = "Model";
		break;
	case 2:
		nFlag = eMotion;
		strLoadType = "Motion";
		break;
	case 3:
		nFlag = eModel | eMotion;
		strLoadType = "All";
		break;
	default:
		std::cout << "인자가 올바르지 않습니다." << std::endl << std::endl;
		system("pause");

		return 0;
	}

	std::cout << "스케일 값을 입력해주세요." << std::endl;

	float fScale = 1.f;
	std::cin >> fScale;
	std::cout << std::endl;

	if (fScale <= 0.f)
	{
		std::cout << "스케일 값은 0 이하가 될 수 없습니다." << std::endl;

		system("pause");

		return 0;
	}

	std::cout << "초기화 중..." << std::endl;

	if (Windows::WindowsManager::GetInstance()->Init("FBXConvertor", 800, 600, false) == false)
	{
		std::cout << "초기화 실패" << std::endl;
	}

	ShowWindow(Windows::GetHwnd(), SW_HIDE);

	if (Graphics::Device::GetInstance()->Init(Windows::GetHwnd(), 800, 600, false, true) == false)
	{
		std::cout << "초기화 실패" << std::endl;
	}

	Graphics::ModelManager::GetInstance();
	Graphics::MotionManager::GetInstance()->Init();

	std::cout << "추출 시작 : S(성공), F(실패)" << std::endl;
	std::cout << "추출 타입 : " << strLoadType.c_str() << ", 스케일 : " << fScale << std::endl;

	std::list<Graphics::IModel*> listModel;
	std::list<Graphics::IMotion*> listMotion;

	int nModelFailedCount = 0;
	int nMotionFailedCount = 0;

	for (int i = 1; i < argn; ++i)
	{
		if ((nFlag & eModel) != 0)
		{
			Graphics::ModelLoader loader;
			loader.InitFBX(File::GetFileNameWithoutExtension(argc[i]).c_str(), argc[i], fScale);
			Graphics::IModel* pModel = Graphics::IModel::Create(loader, false);

			if (pModel->GetLoadState() == Graphics::EmLoadState::eComplete)
			{
				std::string strPath = File::GetFilePath(argc[i]);
				strPath.append(File::GetFileNameWithoutExtension(argc[i]));
				strPath.append(".emod");
				if (Graphics::IModel::SaveToFile(pModel, strPath.c_str()) == true)
				{
					std::cout << "Model[S] : " << argc[i] << std::endl;
					listModel.emplace_back(pModel);
				}
				else
				{
					std::cout << "Model[F] : " << argc[i] << std::endl;
					++nModelFailedCount;

					DeleteFile(strPath.c_str());
				}
			}
			else
			{
				std::cout << "Model[F] : " << argc[i] << std::endl;
				++nModelFailedCount;
			}
		}

		if ((nFlag & eMotion) != 0)
		{
			Graphics::MotionLoader loader;
			loader.InitFBX(File::GetFileNameWithoutExtension(argc[i]).c_str(), argc[i], fScale);
			Graphics::IMotion* pMotion = Graphics::IMotion::Create(loader);

			if (pMotion->GetLoadState() == Graphics::EmLoadState::eComplete)
			{
				std::string strPath = File::GetFilePath(argc[i]);
				strPath.append(File::GetFileNameWithoutExtension(argc[i]));
				strPath.append(".emot");
				if (Graphics::IMotion::SaveToFile(pMotion, strPath.c_str()) == true)
				{
					std::cout << "Motion[S] : " << argc[i] << std::endl;
					listMotion.emplace_back(pMotion);
				}
				else
				{
					std::cout << "Motion[F] : " << argc[i] << std::endl;
					++nMotionFailedCount;

					DeleteFile(strPath.c_str());
				}
			}
			else
			{
				std::cout << "Motion[F] : " << argc[i] << std::endl;
				++nMotionFailedCount;
			}
		}
	}

	std::cout << "추출 완료" << std::endl;
	if ((nFlag & eModel) != 0)
	{
		std::cout << "모델 : S(" << listModel.size() << "), F(" << nModelFailedCount << ")" << std::endl;
	}

	if ((nFlag & eMotion) != 0)
	{
		std::cout << "모션 : S(" << listMotion.size() << "), F(" << nMotionFailedCount << ")" << std::endl;
	}

	Graphics::ModelManager::DestroyInstance();

	Graphics::MotionManager::GetInstance()->Release();
	Graphics::MotionManager::DestroyInstance();

	Graphics::Device::GetInstance()->Release();
	Graphics::Device::DestroyInstance();

	Windows::WindowsManager::GetInstance()->Release();
	Windows::WindowsManager::DestroyInstance();

	String::Release();

	system("pause");

    return 0;
}