#include "stdafx.h"
#include "Scene.h"

namespace est
{
	IScene::IScene(const string::StringID& name)
		: m_name(name)
	{
	}

	IScene::~IScene()
	{
	}
}