#pragma once

#include "CommonLib/Singleton.h"

#include "ModelLoader.h"
#include "MotionLoader.h"

namespace eastengine
{
	namespace graphics
	{
		class Model;
		class IModel;
		class ModelInstance;
		class IModelInstance;

		class Motion;
		class IMotion;

		class ModelManager : public Singleton<ModelManager>
		{
			friend Singleton<ModelManager>;
		private:
			ModelManager();
			virtual ~ModelManager();

		public:
			void Update();
			void Flush(float fElapsedTime);

		public:
			// Model
			void AsyncLoadModel(IModel* pModel, const ModelLoader& loader);

			// FilePath or ModelName
			IModel* AllocateModel(const String::StringID& strKey);
			IModelInstance* AllocateModelInstance(Model* pModel);
			bool DestroyModelInstance(ModelInstance** ppModelInstance);

			// FilePath or ModelName
			IModel* GetModel(const String::StringID& strKey) const;

		public:
			// FilePath or ModelName
			IMotion* AllocateMotion(const String::StringID& strKey);

			// FilePath or ModelName
			IMotion* GetMotion(const String::StringID& strKey);
			IMotion* GetMotion(const size_t nIndex);

			size_t GetMotionCount() const;

		public:
			bool LoadModelFBX(Model* pModel, const char* strFilePath, float fScale, bool isFlipZ);
			bool LoadMotionFBX(Motion* pMotion, const char* strFilePath, float fScale);

		private:
			class Impl;
			std::unique_ptr<Impl> m_pImpl;
		};
	}
}