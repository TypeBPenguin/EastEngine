// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �Ǵ� ������Ʈ ���� ���� ������
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
#include "Graphics/Graphics.h"

#include <ppl.h>
#include <concurrent_queue.h>