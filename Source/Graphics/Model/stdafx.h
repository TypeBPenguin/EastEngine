// stdafx.h : ���� ��������� ���� ��������� �ʴ�
// ǥ�� �ý��� ���� ���� �Ǵ� ������Ʈ ���� ���� ������
// ��� �ִ� ���� �����Դϴ�.
//

#pragma once

#include "CommonLib/ErrorDefine.h"

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // ���� ������ �ʴ� ������ Windows ������� �����մϴ�.

#include "CommonLib/CommonLib.h"
#include "Graphics/Implement/Graphics.h"

#include <concurrent_queue.h>

#define FBXSDK_NEW_API

#pragma warning(push)
#pragma warning( disable : 4616 6011 )
#include <fbxsdk.h>
#pragma warning(pop)

#include <DirectXMath.h>
#include <DirectXPackedVector.h>
#include <DirectXCollision.h>

#undef USE_OPENEXR

#include "ExternLib/FBXExporter/ExportObjects/ExportXmlParser.h"
#include "ExternLib/FBXExporter/ExportObjects/ExportPath.h"
#include "ExternLib/FBXExporter/ExportObjects/ExportMaterial.h"
#include "ExternLib/FBXExporter/ExportObjects/ExportObjects.h"