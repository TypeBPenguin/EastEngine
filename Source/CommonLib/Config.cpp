#include "stdafx.h"
#include "Config.h"

#define ImplConfigVarBool(name, defaultValue)							\
	static bool g_isEnable##name = defaultValue;						\
	void Enable##name(bool isEnable) { g_isEnable##name = isEnable; }	\
	bool IsEnable##name() { return g_isEnable##name; }					\

#define ImplConfigVarScalar(name, defaultValue)							\
	static float g_fScalar##name = defaultValue;						\
	void SetScalar##name(float fValue) { g_fScalar##name = fValue; }	\
	float GetScalar##name() { return g_fScalar##name; }					\

#define ImplConfigVarString(name, defaultValue)												\
	static String::StringID g_str##name = defaultValue;										\
	void SetString##name(const String::StringID& strOption) { g_str##name = strOption; }	\
	const String::StringID& GetString##name() { return g_str##name; }						\

namespace EastEngine
{
	namespace Config
	{
		ImplConfigVarBool(ShowBoundingBox, false);
		ImplConfigVarBool(OcclusionCulling, false);
		ImplConfigVarBool(SSAO, false);
		ImplConfigVarBool(Shadow, false);
		ImplConfigVarBool(FXAA, false);

		ImplConfigVarBool(DepthOfField, false);

		ImplConfigVarBool(HDRFilter, false);
		ImplConfigVarBool(Tessellation, false);
		ImplConfigVarBool(Wireframe, false);
	}
}