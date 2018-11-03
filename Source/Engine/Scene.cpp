#include "stdafx.h"
#include "Scene.h"

namespace eastengine
{
	IScene::IScene(const string::StringID& strName)
		: m_strName(strName)
	{
	}

	IScene::~IScene()
	{
	}
}