#pragma once

namespace eastengine
{
	namespace math
	{
		float3 Convert(const btVector3& v);
		float4 Convert(const btVector4& v);
		Quaternion Convert(const btQuaternion& quat);
		Matrix Convert(const btMotionState& motionState);
		Matrix Convert(const btTransform& tm);

		btVector3 ConvertToBt(const float3& v);
		btVector4 ConvertToBt(const float4& v);
		btQuaternion ConvertToBt(const Quaternion& quat);
		btTransform ConvertToBt(const Matrix& matrix);
		btTransform ConvertToBt(const float3& v, const Quaternion& quat);
	}
}