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

		// ���� �Ʒ��� ������� �� ������?
		// �ڵ� ������ �� ���������� ��
		// ��뼺�� ������Ʈ�� ���� ������ ����ϸ� �Ʒ�
		/*void EnableOption(const String::StringID& strOptionName, bool isEnable);
		bool IsEanbleOption(const String::StringID& strOptionName);

		void SetScalarOption();
		int GetScalarOption();

		void SetStringOption();
		const String::StringID& GetStringOption();*/
	}
}