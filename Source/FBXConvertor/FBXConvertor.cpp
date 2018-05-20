#include "stdafx.h"
#include <iostream>

#include "CommonLib/FileUtil.h"

#include "Windows/Windows.h"
#include "DirectX/Device.h"
#include "Model/ModelManager.h"
#include "Model/MotionManager.h"

using namespace eastengine;

enum EmFlag
{
	eModel = 1,
	eMotion = 2,
};

int main(int argn, char** argc)
{
	if (argn <= 1)
	{
		std::cout << "fbx ������ �巡�׵�� ���ּ���." << std::endl;
		system("pause");

		return 0;
	}

	std::cout << "������ �����͸� �������ּ���." << std::endl;
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
		std::cout << "���ڰ� �ùٸ��� �ʽ��ϴ�." << std::endl << std::endl;
		system("pause");

		return 0;
	}

	std::cout << "������ ���� �Է����ּ���." << std::endl;

	float fScale = 1.f;
	std::cin >> fScale;
	std::cout << std::endl;

	if (fScale <= 0.f)
	{
		std::cout << "������ ���� 0 ���ϰ� �� �� �����ϴ�." << std::endl;

		system("pause");

		return 0;
	}

	std::cout << "�ʱ�ȭ ��..." << std::endl;

	if (Windows::WindowsManager::GetInstance()->Initialize("FBXConvertor", 800, 600, false) == false)
	{
		std::cout << "�ʱ�ȭ ����" << std::endl;
	}

	ShowWindow(Windows::GetHwnd(), SW_HIDE);

	if (graphics::Device::GetInstance()->Initialize(Windows::GetHwnd(), 800, 600, false, true) == false)
	{
		std::cout << "�ʱ�ȭ ����" << std::endl;
	}

	graphics::ModelManager::GetInstance();
	graphics::MotionManager::GetInstance();

	std::cout << "���� ���� : S(����), F(����)" << std::endl;
	std::cout << "���� Ÿ�� : " << strLoadType.c_str() << ", ������ : " << fScale << std::endl;

	std::list<graphics::IModel*> listModel;
	std::list<graphics::IMotion*> listMotion;

	int nModelFailedCount = 0;
	int nMotionFailedCount = 0;

	for (int i = 1; i < argn; ++i)
	{
		if ((nFlag & eModel) != 0)
		{
			graphics::ModelLoader loader;
			loader.InitFBX(file::GetFileNameWithoutExtension(argc[i]).c_str(), argc[i], fScale);
			graphics::IModel* pModel = graphics::IModel::Create(loader, false);

			if (pModel->GetState() == graphics::EmLoadState::eComplete)
			{
				std::string strPath = file::GetFilePath(argc[i]);
				strPath.append(file::GetFileNameWithoutExtension(argc[i]));
				strPath.append(".emod");
				if (graphics::IModel::SaveToFile(pModel, strPath.c_str()) == true)
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
			graphics::MotionLoader loader;
			loader.InitFBX(file::GetFileNameWithoutExtension(argc[i]).c_str(), argc[i], fScale);
			graphics::IMotion* pMotion = graphics::IMotion::Create(loader);

			if (pMotion->GetState() == graphics::EmLoadState::eComplete)
			{
				std::string strPath = file::GetFilePath(argc[i]);
				strPath.append(file::GetFileNameWithoutExtension(argc[i]));
				strPath.append(".emot");
				if (graphics::IMotion::SaveToFile(pMotion, strPath.c_str()) == true)
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

	graphics::ModelManager::DestroyInstance();
	graphics::MotionManager::DestroyInstance();

	graphics::Device::GetInstance()->Release();
	graphics::Device::DestroyInstance();

	Windows::WindowsManager::DestroyInstance();

	String::Release();

	system("pause");

    return 0;
}