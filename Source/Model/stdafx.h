// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �Ǵ� ������Ʈ ���� ���� ������
// ��� �ִ� ���� �����Դϴ�.
//

#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.

#ifdef _DEBUG
#define _SCL_SECURE_NO_WARNINGS
#endif

#include <boost/pool/object_pool.hpp>

#include <ppltasks.h>

#include "CommonLib/CommonLib.h"
#include "DirectX/D3DInterface.h"
