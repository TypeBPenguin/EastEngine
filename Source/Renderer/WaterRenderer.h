#pragma once

#include "Renderer.h"

namespace EastEngine
{
	namespace Graphics
	{
		class IEffect;
		class IEffectTech;

		class Camera;
		class WaterSimulator;

		// Quadtree structures & routines
		struct QuadNode
		{
			Math::Vector2 bottom_left;
			float length;
			int lod;

			int sub_node[4];
		};

		struct QuadRenderParam
		{
			uint32_t num_inner_verts;
			uint32_t num_inner_faces;
			uint32_t inner_start_index;

			uint32_t num_boundary_verts;
			uint32_t num_boundary_faces;
			uint32_t boundary_start_index;
		};

		class WaterRenderer : public IRenderer
		{
		public:
			WaterRenderer();
			virtual ~WaterRenderer();

		public:
			virtual bool Init(const Math::Viewport& viewport) override;

			virtual void Render(uint32_t nRenderGroupFlag) override;
			virtual void Flush() override;

		private:
			void ClearEffect(IDeviceContext* pd3dDeviceContext, IEffectTech* pEffectTech);

			void simulate(float fTime);
			void render(float fTime);

		private:
			bool isLeaf(const QuadNode& quad_node) { return (quad_node.sub_node[0] == -1 && quad_node.sub_node[1] == -1 && quad_node.sub_node[2] == -1 && quad_node.sub_node[3] == -1); }
			int buildNodeList(QuadNode& quad_node, Camera* pCamera);
			bool checkNodeVisibility(const QuadNode& quad_node, Camera* pCamera);
			float estimateGridCoverage(const QuadNode& quad_node, Camera* pCamera, float screen_area);

			int searchLeaf(const std::vector<QuadNode>& node_list, const Math::Vector2& point);
			QuadRenderParam& selectMeshPattern(const QuadNode& quad_node);

			// create a triangle strip mesh for ocean surface.
			void createSurfaceMesh();
			// create color/fresnel lookup table.
			void createFresnelMap();
			// create perlin noise texture for far-sight rendering
			void loadTextures();

			// Generate boundary mesh for a patch. Return the number of generated indices
			int generateBoundaryMesh(int left_degree, int right_degree, int bottom_degree, int top_degree,
				const Math::Rect& vert_rect, uint32_t* output);

			// Generate boundary mesh for a patch. Return the number of generated indices
			int generateInnerMesh(const Math::Rect& vert_rect, uint32_t* output);

		private:
			IEffect* m_pEffect;

			WaterSimulator* m_pWaterSimulator;

			// Mesh grid dimension, must be 2^n. 4x4 ~ 256x256
			int m_nMeshDim;
			// Side length of square shaped mesh patch
			float m_fPatchLength;
			// Dimension of displacement map
			int m_nDisplaceMapDim;
			// Subdivision thredshold. Any quad covers more pixels than this value needs to be subdivided.
			float m_fUpperGridCoverage;
			// Draw distance = m_PatchLength * 2^m_FurthestCover
			int m_nFurthestCover;

			// Shading properties:
			// Two colors for waterbody and sky color
			Math::Vector3 m_f3SkyColor;
			Math::Vector3 m_f3WaterbodyColor;
			// Blending term for sky cubemap
			float m_fSkyBlending;

			// Perlin wave parameters
			float m_fPerlinSize;
			float m_fPerlinSpeed;
			Math::Vector3 m_f3PerlinAmplitude;
			Math::Vector3 m_f3PerlinGradient;
			Math::Vector3 m_f3PerlinOctave;
			Math::Vector2 m_f2WindDir;

			Math::Vector3 m_f3BendParam;

			// Sunspot parameters
			Math::Vector3 m_f3SunDir;
			Math::Vector3 m_f3SunColor;
			float m_fShineness;

			QuadRenderParam m_mesh_patterns[9][3][3][3][3];
			int m_nLods;

			IVertexBuffer* m_pVertexBuffer;
			IIndexBuffer* m_pIndexBuffer;

			// Color look up 1D texture
			std::shared_ptr<ITexture> m_pFresnelMap;

			// Distant perlin wave
			std::shared_ptr<ITexture> m_pSRV_Perlin;

			// Environment maps
			std::shared_ptr<ITexture> m_pSRV_ReflectCube;

			std::vector<QuadNode> m_vecRenderNode;

			ISamplerState* m_pSamplerHeight;
			ISamplerState* m_pSamplerCube;
			ISamplerState* m_pSamplerGradient;
			ISamplerState* m_pSamplerPerlin;
			ISamplerState* m_pSamplerFresnel;

			IBlendState* m_pBlendState;
		};
	}
}