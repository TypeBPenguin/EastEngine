#include "stdafx.h"
#include <iostream>

#include "CommonLib/FileUtil.h"

#include "Graphics/Graphics.h"

#include "Model/ModelManager.h"

using namespace eastengine;

enum
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

	try
	{
		graphics::Initialize(graphics::APIs::eDX11, 800, 600, false, "FBXConvertor", "FBXConvertor");
		ShowWindow(graphics::GetHwnd(), SW_HIDE);
	}
	catch (...)
	{
	}

	graphics::ModelManager::GetInstance();

	std::cout << "추출 시작 : S(성공), F(실패)" << std::endl;
	std::cout << "추출 타입 : " << strLoadType.c_str() << ", 스케일 : " << fScale << std::endl;

	std::vector<graphics::IModel*> models;
	std::vector<graphics::IMotion*> motions;

	int modelFailedCount = 0;
	int motionFailedCount = 0;

	for (int i = 1; i < argn; ++i)
	{
		if ((nFlag & eModel) != 0)
		{
			graphics::ModelLoader loader;
			loader.InitFBX(file::GetFileNameWithoutExtension(argc[i]).c_str(), argc[i], fScale);
			graphics::IModel* pModel = graphics::IModel::Create(loader, false);

			if (pModel->GetState() == graphics::IResource::eComplete)
			{
				std::string strPath = file::GetFilePath(argc[i]);
				strPath.append(file::GetFileNameWithoutExtension(argc[i]));
				strPath.append(".emod");
				if (graphics::IModel::SaveFile(pModel, strPath.c_str()) == true)
				{
					std::cout << "Model[S] : " << argc[i] << std::endl;
					models.emplace_back(pModel);
				}
				else
				{
					std::cout << "Model[F] : " << argc[i] << std::endl;
					++modelFailedCount;

					DeleteFile(strPath.c_str());
				}
			}
			else
			{
				std::cout << "Model[F] : " << argc[i] << std::endl;
				++modelFailedCount;
			}
		}

		if ((nFlag & eMotion) != 0)
		{
			graphics::MotionLoader loader;
			loader.InitFBX(file::GetFileNameWithoutExtension(argc[i]).c_str(), argc[i], fScale);
			graphics::IMotion* pMotion = graphics::IMotion::Create(loader);

			if (pMotion->GetState() == graphics::IResource::eComplete)
			{
				std::string strPath = file::GetFilePath(argc[i]);
				strPath.append(file::GetFileNameWithoutExtension(argc[i]));
				strPath.append(".emot");
				if (graphics::IMotion::SaveFile(pMotion, strPath.c_str()) == true)
				{
					std::cout << "Motion[S] : " << argc[i] << std::endl;
					motions.emplace_back(pMotion);
				}
				else
				{
					std::cout << "Motion[F] : " << argc[i] << std::endl;
					++motionFailedCount;

					DeleteFile(strPath.c_str());
				}
			}
			else
			{
				std::cout << "Motion[F] : " << argc[i] << std::endl;
				++motionFailedCount;
			}
		}
	}

	std::cout << "추출 완료" << std::endl;
	if ((nFlag & eModel) != 0)
	{
		std::cout << "모델 : S(" << models.size() << "), F(" << modelFailedCount << ")" << std::endl;
	}

	if ((nFlag & eMotion) != 0)
	{
		std::cout << "모션 : S(" << motions.size() << "), F(" << motionFailedCount << ")" << std::endl;
	}

	graphics::ModelManager::DestroyInstance();

	graphics::Release();

	string::Release();

	system("pause");

    return 0;
}