#pragma once

#include "GraphicsInterface.h"

// Disabling adaptive quality uses a tiny bit less memory (21.75Mb instead of 22Mb at 1920x1080) and makes it easier to port
#define INTEL_SSAO_ENABLE_ADAPTIVE_QUALITY
// doesn't support dx12 yet

// If enabled, will use ASSAO_Inputs::NormalsWorldToViewspaceMatrix, otherwise it is ignored (compiled out) as it adds approx 3% to the overall cost
#define INTEL_SSAO_ENABLE_NORMAL_WORLD_TO_VIEW_CONVERSION

#define SSA_STRINGIZIZER( x ) SSA_STRINGIZIZER_( x )
#define SSA_STRINGIZIZER_( x ) #x

// ** WARNING ** if changing any of the slot numbers, please remember to update the corresponding shader code!
#define SSAO_SAMPLERS_SLOT0				0
#define SSAO_SAMPLERS_SLOT1				1
#define SSAO_SAMPLERS_SLOT2				2
#define SSAO_SAMPLERS_SLOT3				3
#define SSAO_NORMALMAP_OUT_UAV_SLOT		4
#define SSAO_CONSTANTS_BUFFERSLOT		0
#define SSAO_TEXTURE_SLOT0				0
#define SSAO_TEXTURE_SLOT1				1
#define SSAO_TEXTURE_SLOT2				2
#define SSAO_TEXTURE_SLOT3				3
#define SSAO_TEXTURE_SLOT4				4
#define SSAO_LOAD_COUNTER_UAV_SLOT		4

#define SSAO_MAX_TAPS					32
#define SSAO_ADAPTIVE_TAP_BASE_COUNT	5
#define SSAO_ADAPTIVE_TAP_FLEXIBLE_COUNT (SSAO_MAX_TAPS-SSAO_ADAPTIVE_TAP_BASE_COUNT)
#define SSAO_DEPTH_MIP_LEVELS			4

#ifdef INTEL_SSAO_ENABLE_NORMAL_WORLD_TO_VIEW_CONVERSION
#define SSAO_ENABLE_NORMAL_WORLD_TO_VIEW_CONVERSION 1
#else
#define SSAO_ENABLE_NORMAL_WORLD_TO_VIEW_CONVERSION 0
#endif

namespace eastengine
{
	namespace graphics
	{
		struct IAssaoInputs
		{
			// Output scissor rect - used to draw AO effect to a sub-rectangle, for example, for performance reasons when using wider-than-screen depth input to avoid close-to-border artifacts.
			uint32_t ScissorLeft{ 0 };
			uint32_t ScissorTop{ 0 };
			uint32_t ScissorRight{ 0 };
			uint32_t ScissorBottom{ 0 };

			// Custom viewports not supported yet; this is here for future support. ViewportWidth and ViewportHeight must match or be smaller than source depth and normalmap sizes.
			uint32_t ViewportX{ 0 };
			uint32_t ViewportY{ 0 };
			uint32_t ViewportWidth{ 0 };
			uint32_t ViewportHeight{ 0 };

			// Requires a projection matrix created with xxxPerspectiveFovLH or equivalent, not tested (yet) for right-handed
			// coordinates and will likely break for ortho projections.
			math::Matrix ProjectionMatrix;

#ifdef INTEL_SSAO_ENABLE_NORMAL_WORLD_TO_VIEW_CONVERSION
			// In case normals are in world space, matrix used to convert them to viewspace
			math::Matrix NormalsWorldToViewspaceMatrix;
#endif

			bool MatricesRowMajorOrder{ true };

			// Used for expanding UINT normals from [0, 1] to [-1, 1] if needed.
			float NormalsUnpackMul{ 2.f };
			float NormalsUnpackAdd{ -1.f };
		};

		class IAssaoEffect
		{
		public:
			IAssaoEffect() = default;
			virtual ~IAssaoEffect() = default;

		public:
			virtual void PreAllocateVideoMemory(const IAssaoInputs* inputs) = 0;
			virtual void DeleteAllocatedVideoMemory() = 0;

			// Returns currently allocated video memory in bytes; only valid after PreAllocateVideoMemory / Draw calls
			virtual uint32_t GetAllocatedVideoMemory() = 0;

			virtual void GetVersion(int& major, int& minor) = 0;

			// Apply the SSAO effect to the currently selected render target using provided settings and platform-dependent inputs
			virtual void Draw(const Options::AssaoConfig& config, const IAssaoInputs* inputs) = 0;
		};
	}
}