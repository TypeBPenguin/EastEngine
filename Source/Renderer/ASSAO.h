///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Copyright (c) 2016, Intel Corporation
// Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated 
// documentation files (the "Software"), to deal in the Software without restriction, including without limitation 
// the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to the following conditions:
// The above copyright notice and this permission notice shall be included in all copies or substantial portions of 
// the Software.
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, 
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE 
// SOFTWARE.
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// File changes (yyyy-mm-dd)
// 2016-09-07: filip.strugar@intel.com: first commit
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "CommonLib/Singleton.h"

struct ID3D11Device;
struct ID3D11DeviceContext;
struct ID3D11ShaderResourceView;
struct ID3D11RenderTargetView;

class IDevice;
class IDeviceContext;

#pragma pack( push, 8 ) // Make sure we have consistent structure packings

// Disabling adaptive quality uses a tiny bit less memory (21.75Mb instead of 22Mb at 1920x1080) and makes it easier to port
#define INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY

// If enabled, will use ASSAO_Inputs::NormalsWorldToViewspaceMatrix, otherwise it is ignored (compiled out) as it adds approx 3% to the overall cost
#define INTEL_SSAO_ENABLE_NORMAL_WORLD_TO_VIEW_CONVERSION

namespace eastengine
{
	namespace graphics
	{
		class Camera;
		class IRenderTarget;

		// effect visual settings
		struct ASSAO_Settings
		{
			float Radius{ 1.2f };								// [0.0,  ~ ] World (view) space size of the occlusion sphere.
			float ShadowMultiplier{ 1.f };						// [0.0, 5.0] Effect strength linear multiplier
			float ShadowPower{ 1.5f };							// [0.5, 5.0] Effect strength pow modifier
			float ShadowClamp{ 0.98f };							// [0.0, 1.0] Effect max limit (applied after multiplier but before blur)
			float HorizonAngleThreshold{ 0.06f };				// [0.0, 0.2] Limits self-shadowing (makes the sampling area less of a hemisphere, more of a spherical cone, to avoid self-shadowing and various artifacts due to low tessellation and depth buffer imprecision, etc.)
			float FadeOutFrom{ 50.f };							// [0.0,  ~ ] Distance to start start fading out the effect.
			float FadeOutTo{ 300.f };							// [0.0,  ~ ] Distance at which the effect is faded out.
			float AdaptiveQualityLimit{ 0.45f };				// [0.0, 1.0] (only for Quality Level 3)
			int QualityLevel{ 2 };								// [ -1,  3 ] Effect quality{}; -1 - lowest (low, half res checkerboard), 0 - low, 1 - medium, 2 - high, 3 - very high / adaptive{}; each quality level is roughly 2x more costly than the previous, except the q3 which is variable but, in general, above q2.
			int BlurPassCount{ 2 };								// [  0,   6] Number of edge-sensitive smart blur passes to apply. Quality 0 is an exception with only one 'dumb' blur pass used.
			float Sharpness{ 0.98f };							// [0.0, 1.0] (How much to bleed over edges{}; 1: not at all, 0.5: half-half{}; 0.0: completely ignore edges)
			float TemporalSupersamplingAngleOffset{ 0.f };		// [0.0,  PI] Used to rotate sampling kernel{}; If using temporal AA / supersampling, suggested to rotate by ( (frame%3)/3.0*PI ) or similar. Kernel is already symmetrical, which is why we use PI and not 2*PI.
			float TemporalSupersamplingRadiusOffset{ 1.f };		// [0.0, 2.0] Used to scale sampling kernel{}; If using temporal AA / supersampling, suggested to scale by ( 1.0f + (((frame%3)-1.0)/3.0)*0.1 ) or similar.
			float DetailShadowStrength{ 0.5f };					// [0.0, 5.0] Used for high-res detail AO using neighboring depth pixels: adds a lot of detail but also reduces temporal stability (adds aliasing).
		};

		struct ASSAO_Inputs
		{
			// Output scissor rect - used to draw AO effect to a sub-rectangle, for example, for performance reasons when using wider-than-screen depth input to avoid close-to-border artifacts.
			uint32_t ScissorLeft;
			uint32_t ScissorTop;
			uint32_t ScissorRight;
			uint32_t ScissorBottom;

			// Custom viewports not supported yet; this is here for future support. ViewportWidth and ViewportHeight must match or be smaller than source depth and normalmap sizes.
			uint32_t ViewportX;
			uint32_t ViewportY;
			uint32_t ViewportWidth;
			uint32_t ViewportHeight;

			// Requires a projection matrix created with xxxPerspectiveFovLH or equivalent, not tested (yet) for right-handed
			// coordinates and will likely break for ortho projections.
			math::Matrix ProjectionMatrix;

#ifdef INTEL_SSAO_ENABLE_NORMAL_WORLD_TO_VIEW_CONVERSION
			// In case normals are in world space, matrix used to convert them to viewspace
			math::Matrix NormalsWorldToViewspaceMatrix;
#endif

			bool MatricesRowMajorOrder;

			// Used for expanding UINT normals from [0, 1] to [-1, 1] if needed.
			float NormalsUnpackMul;
			float NormalsUnpackAdd;

			bool DrawOpaque;

		protected:
			ASSAO_Inputs()
				: ScissorLeft(0)
				, ScissorTop(0)
				, ScissorRight(0)
				, ScissorBottom(0)
				, ViewportX(0)
				, ViewportY(0)
				, ViewportWidth(0)
				, ViewportHeight(0)
				, MatricesRowMajorOrder(true)
				, DrawOpaque(false)
				, NormalsUnpackMul(2.0f)
				, NormalsUnpackAdd(-1.0f)
			{
			}
		public:
			virtual ~ASSAO_Inputs() {}
		};

		//
		struct ASSAO_CreateDesc
		{
			ID3D11Device* Device = nullptr;
			const void* ShaderData = nullptr;
			std::size_t ShaderDataSize = 0;

			// data pointed to by shaderData is only used during a call to ASSAO_Effect::CreateInstance
			ASSAO_CreateDesc(ID3D11Device* device, const void* shaderData, std::size_t shaderDataSize) : Device(device), ShaderData(shaderData), ShaderDataSize(shaderDataSize) {}
		};

		// 
		struct ASSAO_InputsDX11 : ASSAO_Inputs
		{
			IDeviceContext* DeviceContext{ nullptr };

			// Hardware screen depths
			//  - R32_FLOAT (R32_TYPELESS) or R24_UNORM_X8_TYPELESS (R24G8_TYPELESS) texture formats are supported.
			//  - Multisampling not yet supported.
			//  - Decoded using provided ProjectionMatrix.
			//  - For custom decoding see PSPrepareDepths/PSPrepareDepthsAndNormals where they get converted to linear viewspace storage.
			ID3D11ShaderResourceView* DepthSRV;

			// Viewspace normals (optional) 
			//  - If NULL, normals are generated from the depth buffer, otherwise provided normals are used for AO. ASSAO is less
			//    costly when input normals are provided, and has a more defined effect. However, aliasing in normals can result in 
			//    aliasing/flickering in the effect so, in some rare cases, normals generated from the depth buffer can look better.
			//  - _FLOAT or _UNORM texture formats are supported.
			//  - Input normals are expected to be in viewspace, encoded in [0, 1] with "encodedNormal.xyz = (normal.xyz* 0.5 + 0.5)" 
			//    or similar. Decode is done in LoadNormal() function with "normal.xyz = (encodedNormal.xyz* 2.0 - 1.0)", which can be
			//    easily modified for any custom decoding.
			//  - Use SSAO_SMOOTHEN_NORMALS for additional normal smoothing to reduce aliasing/flickering. This, however, also reduces
			//    high detail AO and increases cost.
			ID3D11ShaderResourceView* NormalSRV;

			// If not NULL, instead writing into currently bound render target, Draw will use this. Current render target will be restored 
			// to what it was originally after the Draw call.
			ID3D11RenderTargetView* OverrideOutputRTV;

			ASSAO_InputsDX11()
			{
				DeviceContext = nullptr;
				DepthSRV = nullptr;
				NormalSRV = nullptr;
				OverrideOutputRTV = nullptr;
			}
		};

#pragma pack( pop )

		class ASSAO_Effect
		{
		protected:
			ASSAO_Effect() {}
			virtual ~ASSAO_Effect() {}

		public:
			virtual void PreAllocateVideoMemory(const ASSAO_Inputs* inputs) = 0;
			virtual void DeleteAllocatedVideoMemory() = 0;

			// Returns currently allocated video memory in bytes; only valid after PreAllocateVideoMemory / Draw calls
			virtual unsigned int GetAllocatedVideoMemory() = 0;

			virtual void GetVersion(int& major, int& minor) = 0;

			// Apply the SSAO effect to the currently selected render target using provided settings and platform-dependent inputs
			virtual void Draw(const ASSAO_Settings& settings, const ASSAO_Inputs* inputs) = 0;

		public:
			static ASSAO_Effect* CreateInstance(const ASSAO_CreateDesc& createDesc);
			static void DestroyInstance(ASSAO_Effect* effectInstance);
		};

		class ASSAO : public Singleton<ASSAO>
		{
			friend Singleton<ASSAO>;
		private:
			ASSAO();
			virtual ~ASSAO();
		
		public:
			bool Init();
			void Release();
		
		public:
			void Apply(IDevice* pDevice, IDeviceContext* pDeviceContext, Camera* pCamera, IRenderTarget* pResult);

			ASSAO_Settings& GetSettings() { return m_settings; }

		private:
			ASSAO_Effect* m_pEffect;
			ASSAO_Settings m_settings;
		};
	}
}