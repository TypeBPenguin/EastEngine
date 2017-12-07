// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �� ������Ʈ ���� ���� ������
// ��� �ִ� ���� �����Դϴ�.
//

#pragma once

#include "CommonLib/ErrorDefine.h"

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.

#ifdef _DEBUG
#define _SCL_SECURE_NO_WARNINGS
#endif

#include "CommonLib/CommonLib.h"
#include "Model/ModelManager.h"
#include "Renderer/RendererManager.h"
#include "Physics/PhysicsSystem.h"
