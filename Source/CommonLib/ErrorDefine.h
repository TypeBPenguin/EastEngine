#pragma once

#pragma warning(disable:4100)
#pragma warning(disable:4201)

// �Ʒ��� c++ 17 ������ ���̻� ���� ���ϴ� ����� ����� �ڵ���� ������ ó������ �ʵ��� �ϴ� ����
// �ν�Ʈ�� colony ���� ���̺귯������ �߻��ϰ� �ֱ� ������
// ���߿� ������ ������ ������ �������� �ϰ� �ش� ������ �����ֵ��� ����
//#define _SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING
//#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECATION_WARNING

#if defined(_CRTDBG_MAP_ALLOC) && (defined(DEBUG) || defined(_DEBUG))
#define _CRTDBG_MAP_ALLOC	// �޸� ���� ����
#endif

#define WIN32_LEAN_AND_MEAN             // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.
#define NOMINMAX

#include <stdlib.h>
#include <crtdbg.h>