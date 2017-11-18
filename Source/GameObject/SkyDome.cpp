#include "stdafx.h"
#include "SkyDome.h"

#include "XmlParser/XmlParser.h"
#include "CommonLib/FileUtil.h"

#include "SkyDome.inc"

#include "Model/GeometryModel.h"

namespace EastEngine
{
	namespace GameObject
	{
		SkyDome::SkyDome()
			: m_nCurCloudIdx(0)
			, m_nPrevColudIdx(0)
			, m_fNextChangeTIme(0.f)
			, m_fBlendTime(0.f)
			, m_fUpdateTime(0.f)
			, m_pTexSkyEffect(nullptr)
		{
		}

		SkyDome::~SkyDome()
		{
			m_pTexSkyEffect.reset();

			for (auto& pSkyDome : m_skyDomes)
			{
				SafeDelete(pSkyDome.pVertexBuffer);
				SafeDelete(pSkyDome.pIndexBuffer);
			}
		}

		SkyDome* SkyDome::Create(const char* strSetupFile)
		{
			SkyDome* pSky = new SkyDome;
			if (pSky->loadSetup(strSetupFile) == false)
			{
				SafeDelete(pSky);
				return false;
			}

			/*if (initSkyDome() == false)
			{
			Release();
			return false;
			}*/

			Graphics::GeometryModel::CreateGeoSphere(&pSky->m_skyDomes[eSky].pVertexBuffer, &pSky->m_skyDomes[eSky].pIndexBuffer, 10000.f, 4, true);
			Graphics::GeometryModel::CreateGeoSphere(&pSky->m_skyDomes[eEffect].pVertexBuffer, &pSky->m_skyDomes[eEffect].pIndexBuffer, 9800.f, 4, true);
			Graphics::GeometryModel::CreateGeoSphere(&pSky->m_skyDomes[eCloud].pVertexBuffer, &pSky->m_skyDomes[eCloud].pIndexBuffer, 9600.f, 4, true);

			pSky->m_fBlendTime = Math::Random<float>(5.f, 10.f);
			pSky->m_fNextChangeTIme = pSky->m_fBlendTime + Math::Random<float>(10.f, 20.f);

			return pSky;
		}

		void SkyDome::Destroy(SkyDome** ppSkyDome)
		{
			if (ppSkyDome == nullptr || *ppSkyDome == nullptr)
				return;

			SafeDelete(*ppSkyDome);
		}

		void SkyDome::Update(float fElapsedTime)
		{
			m_skyDomes[eSky].matWorld = Math::Matrix::Compose(Math::Vector3::One, Math::Quaternion::Identity, m_f3Position);
			m_skyDomes[eEffect].matWorld = Math::Matrix::Compose(Math::Vector3::One, Math::Quaternion::Identity, m_f3Position);

			Graphics::RenderSubsetSky renderSubsetSky(m_skyDomes[eSky].pVertexBuffer, m_skyDomes[eSky].pIndexBuffer, &m_skyDomes[eSky].matWorld, &m_colorApex, &m_colorCenter);
			Graphics::RendererManager::GetInstance()->AddRender(renderSubsetSky);

			Graphics::RenderSubsetSkyEffect renderSubsetSkyEffect(m_skyDomes[eEffect].pVertexBuffer, m_skyDomes[eEffect].pIndexBuffer, &m_skyDomes[eEffect].matWorld, m_pTexSkyEffect);
			Graphics::RendererManager::GetInstance()->AddRender(renderSubsetSkyEffect);

			m_fUpdateTime += fElapsedTime;
			if (m_fUpdateTime >= m_fNextChangeTIme)
			{
				m_fUpdateTime -= m_fNextChangeTIme;

				m_nPrevColudIdx = m_nCurCloudIdx;
				m_nCurCloudIdx = Math::Random<int>(0, m_vecTexCloud.size() - 1);
			}

			m_skyDomes[eCloud].vRot.y += fElapsedTime * 0.1f;
			if (m_skyDomes[eCloud].vRot.y >= Math::PI * 2.f)
			{
				m_skyDomes[eCloud].vRot.y -= Math::PI * 2.f;
			}

			float fBlend = 0.f;
			if (m_fUpdateTime <= m_fBlendTime)
			{
				fBlend = m_fUpdateTime / m_fBlendTime;
			}

			Math::Quaternion quat = Math::Quaternion::CreateFromYawPitchRoll(m_skyDomes[eCloud].vRot.y, m_skyDomes[eCloud].vRot.x, m_skyDomes[eCloud].vRot.z);
			m_skyDomes[eCloud].matWorld = Math::Matrix::Compose(Math::Vector3::One, quat, m_f3Position);

			//RenderSubsetSkyCloud renderSubset(m_skyDomes[CLOUD].pVertexBuffer, m_skyDomes[CLOUD].pIndexBuffer, &m_skyDomes[SKY].matWorld, m_vecTexCloud[m_nCurCloudIdx], m_vecTexCloud[m_nPrevColudIdx], fBlend);
			//AloRendererMgrInst->AddRender(renderSubset);
		}

		bool SkyDome::loadSetup(const char* strSetupFile)
		{
			XML::CXmlDoc doc;
			if (doc.LoadFile(strSetupFile) == false)
				return false;

			XML::CXmlNode node = doc.GetFirstChild("Class");
			if (node.IsVaild() == false)
				return false;

			XML::CXmlElement element = node.GetFirstChildElement("Sky");

			if (element.IsVaild() == false)
				return false;

			auto vecSlice = String::Tokenizer(element.Attribute("ColorApex"), " ");
			if (vecSlice.empty())
				return false;

			float* pfColor = &m_colorApex.r;

			uint32_t nSize = vecSlice.size();
			for (uint32_t i = 0; i < nSize; ++i)
			{
				if (vecSlice[i].empty())
					continue;

				pfColor[i] = String::ToValue<float>(vecSlice[i].c_str());
			}

			vecSlice.clear();

			vecSlice = String::Tokenizer(element.Attribute("ColorCenter"), " ");
			if (vecSlice.empty())
				return false;

			pfColor = &m_colorCenter.r;

			nSize = vecSlice.size();
			for (uint32_t i = 0; i < nSize; ++i)
			{
				if (vecSlice[i].empty())
					continue;

				pfColor[i] = static_cast<float>(atof(vecSlice[i].c_str()));
			}

			vecSlice.clear();

			for (XML::CXmlElement childElement = element.FirstChildElement();
				childElement.IsVaild() == true; childElement = childElement.NextSibling())
			{
				if (String::IsEquals(childElement.GetName(), "Effect"))
				{
					const char* strSkyEffectFile = childElement.Attribute("File");
					if (strSkyEffectFile == nullptr)
						continue;

					const char* strPath = childElement.Attribute("Path");
					if (strPath == nullptr)
						continue;

					std::string strFilePath = File::GetDataPath();
					strFilePath.append(strPath);
					strFilePath.append(strSkyEffectFile);

					m_pTexSkyEffect = Graphics::ITexture::Create(strSkyEffectFile, strFilePath);
				}
				else if (String::IsEquals(childElement.GetName(), "Cloud"))
				{
					const char* strSkyCloudFile = childElement.Attribute("File");
					if (strSkyCloudFile == nullptr)
						continue;

					const char* strPath = childElement.Attribute("Path");
					if (strPath == nullptr)
						continue;

					std::string strFilePath = File::GetDataPath();
					strFilePath.append(strPath);
					strFilePath.append(strSkyCloudFile);

					m_vecTexCloud.push_back(Graphics::ITexture::Create(strSkyCloudFile, strFilePath));
				}
			}

			return true;
		}

		bool SkyDome::initSkyDome()
		{
			uint32_t nVertexCount = sizeof(SkyDomePatches) / sizeof(SkyDomePatches[0]);

			std::vector<Graphics::VertexPos> vecVertex;
			vecVertex.resize(nVertexCount);

			std::vector<uint32_t> vecIndex;
			vecIndex.resize(nVertexCount);

			for (uint32_t i = 0; i < nVertexCount; ++i)
			{
				vecVertex[i].pos.x = SkyDomePatches[i].fValue[0];
				vecVertex[i].pos.y = SkyDomePatches[i].fValue[1];
				vecVertex[i].pos.z = SkyDomePatches[i].fValue[2];

				//vecVertex[i].uv.x = SkyDomePatches[i].fValue[3];
				//vecVertex[i].uv.y = SkyDomePatches[i].fValue[4];

				//vecVertex[i].normal.x = SkyDomePatches[i].fValue[5];
				//vecVertex[i].normal.y = SkyDomePatches[i].fValue[6];
				//vecVertex[i].normal.z = SkyDomePatches[i].fValue[7];

				vecIndex[i] = i;
			}

			/*CVertexBuffer* pVertexBuffer = new CVertexBuffer;
			if (pVertexBuffer->Init(Device::GetInstance()->GetDevice(), vecVertex.GetFormat(), vecVertex.GetBufferData()) == false)
			{
				SafeDelete(pVertexBuffer);
				Release();
				return false;
			}

			m_pVertexBuffer = pVertexBuffer;

			CIndexBuffer* pIndexBuffer = new CIndexBuffer;
			if (pIndexBuffer->Init(Device::GetInstance()->GetDevice(), vecIndex.GetFormat(), vecIndex.GetBufferData()) == false)
			{
				SafeDelete(pIndexBuffer);
				Release();
				return false;
			}

			m_pIndexBuffer = pIndexBuffer;*/

			return true;
		}
	}
}