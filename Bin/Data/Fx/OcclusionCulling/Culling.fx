#ifndef _OCCLUSION_CULLING_
#define _OCCLUSION_CULLING_

#define MAX_INSTANCE_CONSTANTS 256

struct InstDataCulling
{
	float4 world1;
	float4 world2;
	float4 world3;
	float4 idx;
};

float4x4 decodeMatrix(float3x4 encodedMatrix)
{
	return float4x4(float4(encodedMatrix[0].xyz, 0),
		float4(encodedMatrix[1].xyz, 0),
		float4(encodedMatrix[2].xyz, 0),
		float4(encodedMatrix[0].w, encodedMatrix[1].w, encodedMatrix[2].w, 1.f));
}

float4x4 ComputeWorldMatrix(InstDataCulling worldData)
{
	return decodeMatrix(float3x4(worldData.world1, worldData.world2, worldData.world3));
}

cbuffer cbInstData
{
	InstDataCulling	g_Instances[MAX_INSTANCE_CONSTANTS];
}

cbuffer cbMatrix
{
	float4x4 g_matViewProj;
};

struct VS_INPUT
{
	float4 pos : POSITION;
	uint InstanceID : SV_InstanceID;
};

struct PS_INPUT
{
	float4 pos : SV_Position;
	float idx : TEXCOORD0;
};

PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	input.pos.w = 1.f;

	float4x4 matWorld = ComputeWorldMatrix(g_Instances[input.InstanceID]);

	// 정점 위치
	output.pos = mul(input.pos, matWorld);
	output.pos = mul(output.pos, g_matViewProj);

	output.idx  = g_Instances[input.InstanceID].idx.x;

	return output;
}

float4 PS(PS_INPUT input) : SV_Target
{
	return float4(input.idx, 0.f, 0.f, 1.f);
}

technique11 OcclusionCulling
{
	pass Pass_0
	{
		SetVertexShader(CompileShader(vs_5_0, VS()));
		SetPixelShader(CompileShader(ps_5_0, PS()));
	}
}

cbuffer CB
{    
    float2 g_f2ViewportSize;        // Viewport Width and Height in pixels
};

RWStructuredBuffer<float> g_cullingBufferOut;

Texture2D<float> g_texHizMap;
SamplerState g_samHizMap;

//#ifdef SM4
//#define NUM_THREADS_X 768
//#else
//#define NUM_THREADS_X 1024
//#endif
//[numthreads(NUM_THREADS_X, 1, 1)]
[numthreads(1024, 1, 1)]
void CS(uint3 GroupId          : SV_GroupID,
            uint3 DispatchThreadId : SV_DispatchThreadID,
            uint GroupIndex        : SV_GroupIndex)
{
	float dx = 1.f / g_f2ViewportSize.x;
	float dy = 1.f / g_f2ViewportSize.y;

	for (int i = 0; i < g_f2ViewportSize.y; ++i)
	{
		for (int j = 0; j < g_f2ViewportSize.x; ++j)
		{
			//float index = g_texHizMap.SampleLevel(g_samHizMap, float2(j / g_f2ViewportSize.x, i / g_f2ViewportSize.y), 0);
			float index = g_texHizMap.SampleLevel(g_samHizMap, float2((GroupId.x - 1) / g_f2ViewportSize.x, i / g_f2ViewportSize.y), 0);

			if (index > 0.f)
			{
				g_cullingBufferOut[(int)(index - 1.f)] = 1.f;
			}
		}
	}
}

technique11 OcclusionCullingCS
{
	pass Pass_0
	{
		SetComputeShader(CompileShader(cs_5_0, CS()));
	}
}

//cbuffer CB
//{
//    matrix g_matView;
//    matrix g_matProjection;
//    matrix g_matViewProjection;
//    
//    float4 g_f4FrustumPlanes[6];    // view-frustum planes in world space (normals face out)
//    
//    float2 g_f2ViewportSize;        // Viewport Width and Height in pixels
//};
//
//// Bounding sphere center (XYZ) and radius (W), world space
//StructuredBuffer<float4> g_cullingBuffer0    : register(t0);
//// Is Visible 1 (Visible) 0 (Culled)
//RWStructuredBuffer<float> g_cullingBufferOut : register(u0);
//
//Texture2D<float> g_texHizMap;
//SamplerState g_samHizMap;
//
//// Computes signed distance between a point and a plane
//// vPlane: Contains plane coefficients (a,b,c,d) where: ax + by + cz = d
//// vPoint: Point to be tested against the plane.
//float DistanceToPlane( float4 vPlane, float3 vPoint )
//{
//    return dot(float4(vPoint, 1), vPlane);
//}
//
//// Frustum cullling on a sphere. Returns > 0 if visible, <= 0 otherwise
//float CullSphere( float4 vPlanes[6], float3 vCenter, float fRadius )
//{
//   float dist01 = min(DistanceToPlane(vPlanes[0], vCenter), DistanceToPlane(vPlanes[1], vCenter));
//   float dist23 = min(DistanceToPlane(vPlanes[2], vCenter), DistanceToPlane(vPlanes[3], vCenter));
//   float dist45 = min(DistanceToPlane(vPlanes[4], vCenter), DistanceToPlane(vPlanes[5], vCenter));
//   
//   return min(min(dist01, dist23), dist45) + fRadius;
//}
//
////#ifdef SM4
////#define NUM_THREADS_X 768
////#else
////#define NUM_THREADS_X 1024
////#endif
////[numthreads(NUM_THREADS_X, 1, 1)]
//[numthreads(1, 1, 1)]
//void CSMain( uint3 GroupId          : SV_GroupID,
//             uint3 DispatchThreadId : SV_DispatchThreadID,
//             uint GroupIndex        : SV_GroupIndex)
//{
//    // Calculate the actual index this thread in this group will be reading from.
//    //int index = GroupIndex + (GroupId.x * NUM_THREADS_X);
//    int index = DispatchThreadId.x;
//    
//    // Bounding sphere center (XYZ) and radius (W), world space
//    float4 Bounds = g_cullingBuffer0[index];
//    
//    // Perform view-frustum test
//    float fVisible = CullSphere(g_f4FrustumPlanes, Bounds.xyz, Bounds.w);
//    
//    if (fVisible > 0)
//    {
//        float3 viewEye = -g_matView._m03_m13_m23;
//        float CameraSphereDistance = distance( viewEye, Bounds.xyz );
//        
//        float3 viewEyeSphereDirection = viewEye - Bounds.xyz;
//        
//        float3 viewUp = g_matView._m01_m11_m21;
//        float3 viewDirection = g_matView._m02_m12_m22;
//        float3 viewRight = normalize(cross(viewEyeSphereDirection, viewUp));
//        
//        // Help deal with the perspective distortion.
//        // http://article.gmane.org/gmane.games.devel.algorithms/21697/
//        float fRadius = CameraSphereDistance * tan(asin(Bounds.w / CameraSphereDistance));
//        
//        // Compute the offsets for the points around the sphere
//        float3 vUpRadius = viewUp * fRadius;
//        float3 vRightRadius = viewRight * fRadius;
//        
//        // Generate the 4 corners of the sphere in world space.
//        float4 vCorner0WS = float4( Bounds.xyz + vUpRadius - vRightRadius, 1 ); // Top-Left
//        float4 vCorner1WS = float4( Bounds.xyz + vUpRadius + vRightRadius, 1 ); // Top-Right
//        float4 vCorner2WS = float4( Bounds.xyz - vUpRadius - vRightRadius, 1 ); // Bottom-Left
//        float4 vCorner3WS = float4( Bounds.xyz - vUpRadius + vRightRadius, 1 ); // Bottom-Right
//        
//        // Project the 4 corners of the sphere into clip space
//        float4 vCorner0CS = mul(g_matViewProjection, vCorner0WS);
//        float4 vCorner1CS = mul(g_matViewProjection, vCorner1WS);
//        float4 vCorner2CS = mul(g_matViewProjection, vCorner2WS);
//        float4 vCorner3CS = mul(g_matViewProjection, vCorner3WS);
//        
//        // Convert the corner points from clip space to normalized device coordinates
//        float2 vCorner0NDC = vCorner0CS.xy / vCorner0CS.w;
//        float2 vCorner1NDC = vCorner1CS.xy / vCorner1CS.w;
//        float2 vCorner2NDC = vCorner2CS.xy / vCorner2CS.w;
//        float2 vCorner3NDC = vCorner3CS.xy / vCorner3CS.w;
//        vCorner0NDC = float2( 0.5, -0.5 ) * vCorner0NDC + float2( 0.5, 0.5 );
//        vCorner1NDC = float2( 0.5, -0.5 ) * vCorner1NDC + float2( 0.5, 0.5 );
//        vCorner2NDC = float2( 0.5, -0.5 ) * vCorner2NDC + float2( 0.5, 0.5 );
//        vCorner3NDC = float2( 0.5, -0.5 ) * vCorner3NDC + float2( 0.5, 0.5 );
//        
//        // In order to have the sphere covering at most 4 texels, we need to use
//        // the entire width of the rectangle, instead of only the radius of the rectangle,
//        // which was the orignal implementation in the ATI paper, it had some edge case
//        // failures I observed from being overly conservative.
//        float fSphereWidthNDC = distance( vCorner0NDC, vCorner1NDC );
//        
//        // Compute the center of the bounding sphere in screen space
//        float3 Cv = mul( g_matView, float4( Bounds.xyz, 1 ) ).xyz;
//        
//        // compute nearest point to camera on sphere, and project it
//        float3 Pv = Cv - normalize( Cv ) * Bounds.w;
//        float4 ClosestSpherePoint = mul( g_matProjection, float4( Pv, 1 ) );
//        
//        // Choose a MIP level in the HiZ map.
//        // The orginal assumed viewport width > height, however I've changed it
//        // to determine the greater of the two.
//        //
//        // This will result in a mip level where the object takes up at most
//        // 2x2 texels such that the 4 sampled points have depths to compare
//        // against.
//        float W = fSphereWidthNDC * max(g_f2ViewportSize.x, g_f2ViewportSize.y);
//        float fLOD = ceil(log2( W ));
//        
//        // fetch depth samples at the corners of the square to compare against
//        float4 vSamples;
//        vSamples.x = g_texHizMap.SampleLevel( g_samHizMap, vCorner0NDC, fLOD );
//        vSamples.y = g_texHizMap.SampleLevel( g_samHizMap, vCorner1NDC, fLOD );
//        vSamples.z = g_texHizMap.SampleLevel( g_samHizMap, vCorner2NDC, fLOD );
//        vSamples.w = g_texHizMap.SampleLevel( g_samHizMap, vCorner3NDC, fLOD );
//        
//        float fMaxSampledDepth = max( max( vSamples.x, vSamples.y ), max( vSamples.z, vSamples.w ) );
//        float fSphereDepth = (ClosestSpherePoint.z / ClosestSpherePoint.w);
//        
//        // cull sphere if the depth is greater than the largest of our ZMap values
//		g_cullingBufferOut[index] = (fSphereDepth > fMaxSampledDepth) ? 0 : 1;
//    }
//    else
//    {
//        // The sphere is outside of the view frustum
//		g_cullingBufferOut[index] = 0;
//    }
//}

#endif