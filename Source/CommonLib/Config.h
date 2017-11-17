#pragma once

#define DeclConfigVarBool(name)			\
	void Enable##name(bool isEnable);	\
	bool IsEnable##name();				\

#define DeclConfigVarScalar(name)		\
	void SetScalar##name(float fValue);	\
	float GetScalar##name();			\

#define DeclConfigVarString(name)								\
	void SetString##name(const String::StringID& strOption);	\
	const String::StringID& GetString##name();					\

namespace EastEngine
{
	namespace Config
	{
		DeclConfigVarBool(ShowBoundingBox);
		DeclConfigVarBool(OcclusionCulling);
		DeclConfigVarBool(SSAO);
		DeclConfigVarBool(Shadow);
		DeclConfigVarBool(FXAA);

		DeclConfigVarBool(DepthOfField);
		DeclConfigVarBool(HDRFilter);
		DeclConfigVarBool(Tessellation);
		DeclConfigVarBool(Wireframe);

		// 위랑 아래랑 어느쪽이 더 좋을까?
		// 코드 안전성 및 유지보수는 위
		// 사용성과 프로젝트간 계층 구조를 고려하면 아래
		/*void EnableOption(const String::StringID& strOptionName, bool isEnable);
		bool IsEanbleOption(const String::StringID& strOptionName);

		void SetScalarOption();
		int GetScalarOption();

		void SetStringOption();
		const String::StringID& GetStringOption();*/
	}
}