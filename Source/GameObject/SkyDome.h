#pragma once

namespace EastEngine
{
	namespace Graphics
	{
		class IVertexBuffer;
		class IIndexBuffer;
		class ITexture;
	}

	namespace GameObject
	{
		class SkyDome
		{
		private:
			SkyDome();
			~SkyDome();

		public:
			static SkyDome* Create(const char* strSetupFile);
			static void Destroy(SkyDome** ppSkyDome);

			void Update(float fElapsedTime);

		public:
			const Math::Color& GetColorApex() { return m_colorApex; }
			void SetColorApex(const Math::Color& color) { m_colorApex = color; }

			const Math::Color& GetColorCenter() { return m_colorCenter; }
			void SetColorCenter(const Math::Color& color) { m_colorCenter = color; }

			void SetPos(const Math::Vector3& f3Pos) { m_f3Position = f3Pos; }

		private:
			bool loadSetup(const char* strSetupFile);
			bool initSkyDome();

		private:
			Math::Color m_colorApex;
			Math::Color m_colorCenter;

			struct Model
			{
				Graphics::IVertexBuffer* pVertexBuffer = nullptr;
				Graphics::IIndexBuffer* pIndexBuffer = nullptr;

				Math::Vector3 vRot;
				Math::Matrix matWorld;
			};

			enum Type
			{
				eSky = 0,
				eEffect,
				eCloud,

				TypeCount
			};
			std::array<Model, TypeCount> m_skyDomes;

			std::shared_ptr<Graphics::ITexture>	m_pTexSkyEffect;
			std::vector<std::shared_ptr<Graphics::ITexture>> m_vecTexCloud;

			uint32_t m_nCurCloudIdx;
			uint32_t m_nPrevColudIdx;
			float m_fNextChangeTIme;
			float m_fBlendTime;
			float m_fUpdateTime;

			Math::Vector3 m_f3Position;
		};
	}
}