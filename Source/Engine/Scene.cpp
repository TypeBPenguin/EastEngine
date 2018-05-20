#include "stdafx.h"
#include "Scene.h"

namespace eastengine
{
	IScene::IScene(const String::StringID& strName)
		: m_strName(strName)
	{
	}

	IScene::~IScene()
	{
	}
}