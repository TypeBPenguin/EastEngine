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
	std::cout << "fbx ������ �巡�׵�� ���ּ���." << std::endl;
	std::cout << "���ڸ� 2�� �����ؾ��մϴ�." << std::endl;
	std::cout << "ù��° ����" << std::endl;
	std::cout << "-Model : �� ������ *.emod ���Ϸ� ����" << std::endl;
	std::cout << "-Motion : ��� ������ *.emot ���Ϸ� ����" << std::endl;
	std::cout << "-All : �𵨰� ��� ������ *.emod, *.emot ���Ϸ� ����" << std::endl;
	std::cout << "�ι�° ����" << std::endl;
	std::cout << "float : ������ ���� �����մϴ�." << std::endl;
	std::cout << "ex) -Model 0.01 : Model �� 1/100 ������� �����մϴ�." << std::endl;
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
		std::cout << "���ڰ� �ùٸ��� �ʽ��ϴ�." << std::endl << std::endl;

		Show();
		system("pause");

		return 0;
	}

	float fScale = String::ToValue<float>(argc[2]);
	if (Math::IsZero(fScale) == true)
	{
		std::cout << "������ ���� 0�� �� �� �����ϴ�." << std::endl;

		system("pause");

		return 0;
	}

	std::cout << "�ʱ�ȭ ��..." << std::endl;

	if (Windows::WindowsManager::GetInstance()->Init("FBXConvertor", 800, 600, false) == false)
	{
		std::cout << "�ʱ�ȭ ����" << std::endl;
	}

	ShowWindow(Windows::GetHwnd(), SW_HIDE);

	if (Graphics::Device::GetInstance()->Init(Windows::GetHwnd(), 800, 600, false, true) == false)
	{
		std::cout << "�ʱ�ȭ ����" << std::endl;
	}

	Graphics::ModelManager::GetInstance()->Init();
	Graphics::MotionManager::GetInstance()->Init();

	std::cout << "���� ���� : S(����), F(����)" << std::endl;
	std::cout << "���� Ÿ�� : " << strLoadType.c_str() << ", ������ : " << fScale << std::endl;

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

	std::cout << "���� �Ϸ�" << std::endl;
	if ((nFlag & eModel) != 0)
	{
		std::cout << "�� : S(" << listModel.size() << "), F(" << nModelFailedCount << ")" << std::endl;
	}

	if ((nFlag & eMotion) != 0)
	{
		std::cout << "��� : S(" << listMotion.size() << "), F(" << nMotionFailedCount << ")" << std::endl;
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