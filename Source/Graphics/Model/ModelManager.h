#pragma once

#include "CommonLib/Singleton.h"

#include "ModelInterface.h"

namespace est
{
	namespace graphics
	{
		class Model;
		class ModelInstance;
		class Motion;

		class ModelManager : public Singleton<ModelManager>
		{
			friend Singleton<ModelManager>;
		private:
			ModelManager();
			virtual ~ModelManager();

		public:
			void Update();
			void Cleanup(float elapsedTime);

		public:
			// Model
			void AsyncLoadModel(IModel* pModel, const ModelLoader& loader);

			// FilePath or ModelName
			IModel* AllocateModel(const string::StringID& strKey);
			IModelInstance* AllocateModelInstance(Model* pModel);
			bool DestroyModelInstance(IModelInstance** ppModelInstance);

			// FilePath or ModelName
			IModel* GetModel(const string::StringID& strKey) const;

		public:
			// FilePath or ModelName
			MotionPtr AllocateMotion(const string::StringID& strKey);
			bool DestroyMotion(MotionPtr* ppMotion);

			// FilePath or ModelName
			MotionPtr GetMotion(const string::StringID& strKey);
			MotionPtr GetMotion(const size_t index);

			size_t GetMotionCount() const;

		public:
			bool LoadModelFBX(Model* pModel, const wchar_t* filepath, float scale, bool isFlipZ);
			bool LoadMotionFBX(Motion* pMotion, const wchar_t* filepath, float scale);

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}