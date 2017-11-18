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

void Show()
{
	std::cout << "fbx 파일을 드래그드랍 해주세요." << std::endl;
	std::cout << "인자를 2개 설정해야합니다." << std::endl;
	std::cout << "첫번째 인자" << std::endl;
	std::cout << "-Model : 모델 정보를 *.emod 파일로 추출" << std::endl;
	std::cout << "-Motion : 모션 정보를 *.emot 파일로 추출" << std::endl;
	std::cout << "-All : 모델과 모션 정보를 *.emod, *.emot 파일로 추출" << std::endl;
	std::cout << "두번째 인자" << std::endl;
	std::cout << "float : 스케일 값을 설정합니다." << std::endl;
	std::cout << "ex) -Model 0.01 : Model 을 1/100 사이즈로 추출합니다." << std::endl;
}

int main(int argn, char** argc)
{
	if (argn <= 1)
	{
		Show();
		system("pause");

		return 0;
	}

	int nFlag = 0;
	std::string strLoadType;
	if (String::IsEqualsNoCase(argc[1], "-Model") == true)
	{
		nFlag = eModel;
		strLoadType = "Model";
	}
	else if (String::IsEqualsNoCase(argc[1], "-Motion") == true)
	{
		nFlag = eMotion;
		strLoadType = "Motion";
	}
	else if (String::IsEqualsNoCase(argc[1], "-All") == true)
	{
		nFlag = eModel | eMotion;
		strLoadType = "All";
	}
	else
	{
		std::cout << "인자가 올바르지 않습니다." << std::endl << std::endl;

		Show();
		system("pause");

		return 0;
	}

	float fScale = String::ToValue<float>(argc[2]);
	if (Math::IsZero(fScale) == true)
	{
		std::cout << "스케일 값은 0이 될 수 없습니다." << std::endl;

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

	Graphics::ModelManager::GetInstance()->Init();
	Graphics::MotionManager::GetInstance()->Init();

	std::cout << "추출 시작 : S(성공), F(실패)" << std::endl;
	std::cout << "추출 타입 : " << strLoadType.c_str() << ", 스케일 : " << fScale << std::endl;

	std::list<Graphics::IModel*> listModel;
	std::list<Graphics::IMotion*> listMotion;

	int nModelFailedCount = 0;
	int nMotionFailedCount = 0;

	for (int i = 3; i < argn; ++i)
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

	Graphics::ModelManager::GetInstance()->Release();
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