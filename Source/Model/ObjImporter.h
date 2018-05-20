#pragma once

#include "CommonLib/Singleton.h"

namespace eastengine
{
	namespace file
	{
		class Stream;
	}

	namespace graphics
	{
		struct LODReductionRate;
		class IMaterial;
		class IModel;

		struct FaceType
		{
			math::UInt3 vIdx;
			math::UInt3 tIdx;
			math::UInt3 nIdx;
		};

		namespace EmObjVertex
		{
			enum Type
			{
				eNone = 0,
				eVertex = 1,
				eTexcoord = 1 << 1,
				eNormal = 1 << 2,
			};
		}

		struct ObjSubModel
		{
			std::string strMtlName;

			std::vector<FaceType>	vecFaceType;
		};

		struct ObjGroupData
		{
			std::string strGroupName;

			std::vector<ObjSubModel> vecSubModel;

			ObjGroupData()
			{
				Clear();
			}

			ObjSubModel& GetCurSubModel() { return vecSubModel.back(); }

			void Clear()
			{
				vecSubModel.clear();

				ObjSubModel defaultSubModel;
				vecSubModel.push_back(defaultSubModel);
			}
		};

		struct ObjectData
		{
			String::StringID strObjName;

			std::vector<math::Vector3> vecVertex;
			std::vector<math::Vector2> vecTexcoord;
			std::vector<math::Vector3> vecNormal;

			std::vector<ObjGroupData> vecGroupData;

			uint32_t emObjImportType = EmObjVertex::eNone;

			ObjectData()
			{
				Clear();
			}

			ObjGroupData& GetCurGroupData() { return vecGroupData.back(); }

			bool Empty() const
			{
				return vecVertex.empty() && vecTexcoord.empty() && vecNormal.empty() && vecGroupData.empty();
			}

			void Clear()
			{
				vecVertex.clear();
				vecTexcoord.clear();
				vecNormal.clear();
				vecGroupData.clear();

				ObjGroupData defaultGroup;
				vecGroupData.push_back(defaultGroup);

				emObjImportType = EmObjVertex::eNone;
			}
		};

		class MtlImporter;

		class ObjImporter : public Singleton<ObjImporter>
		{
		public:
			ObjImporter();
			~ObjImporter();

			bool LoadModel(IModel* pModel, const char* strFileName, const float fScaleFactor, uint32_t nLodMax = 0, const LODReductionRate* pLodReductionRate = nullptr);
			void ClearData();

		public:
			const ObjectData& GetObjectData() { return m_objData; }
			IMaterial* GetMaterial(const String::StringID& strName);

		private:
			bool loadModelData(file::Stream& file, const float fScaleFactor);

			bool buildModel(IModel* pIModel, uint32_t nLodMax, const LODReductionRate* pLodReductionRate = nullptr);
			
		private:
			ObjectData m_objData;

			std::unique_ptr<MtlImporter> m_pMtlImporter;
		};
	}
}