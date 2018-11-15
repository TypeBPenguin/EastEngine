#include "stdafx.h"
#include "MathConvertor.h"

namespace eastengine
{
	namespace math
	{
		float3 Convert(const btVector3& v)
		{
			return { v.x(), v.y(), v.z() };
		}

		float4 Convert(const btVector4& v)
		{
			return { v.x(), v.y(), v.z(), v.w() };
		}

		Quaternion Convert(const btQuaternion& quat)
		{
			return { quat.x(), quat.y(), quat.z(), quat.w() };
		}

		Matrix Convert(const btMotionState& motionState)
		{
			btTransform worldTransform;
			motionState.getWorldTransform(worldTransform);

			return Convert(worldTransform);
		}

		Matrix Convert(const btTransform& tm)
		{
			Matrix matWorld;
			btVector3 R = tm.getBasis().getColumn(0);
			btVector3 U = tm.getBasis().getColumn(1);
			btVector3 L = tm.getBasis().getColumn(2);
			btVector3 P = tm.getOrigin();

			matWorld._11 = R.x(); matWorld._12 = R.y(); matWorld._13 = R.z(); matWorld._14 = 0.f;
			matWorld._21 = U.x(); matWorld._22 = U.y(); matWorld._23 = U.z(); matWorld._24 = 0.f;
			matWorld._31 = L.x(); matWorld._32 = L.y(); matWorld._33 = L.z(); matWorld._34 = 0.f;
			matWorld._41 = P.x(); matWorld._42 = P.y(); matWorld._43 = P.z(); matWorld._44 = 1.f;

			return matWorld;
		}

		btVector3 ConvertToBt(const float3& v)
		{
			btVector3 ret;
			ret.setValue(v.x, v.y, v.z);
			return ret;
		}

		btVector4 ConvertToBt(const float4& v)
		{
			btVector4 ret;
			ret.setValue(v.x, v.y, v.z, v.w);
			return ret;
		}

		btQuaternion ConvertToBt(const Quaternion& quat)
		{
			btQuaternion ret;
			ret.setValue(quat.x, quat.y, quat.z, quat.w);
			return ret;
		}

		btTransform ConvertToBt(const Matrix& matrix)
		{
			float3 vPos, vScale;
			Quaternion quat;

			matrix.Decompose(vScale, quat, vPos);

			btVector3 btOrigin = ConvertToBt(vPos);
			btQuaternion btQuat = ConvertToBt(quat);

			btTransform tm;
			tm.setIdentity();
			tm.setRotation(btQuat);
			tm.setOrigin(btOrigin);

			return tm;
		}

		btTransform ConvertToBt(const float3& v, const Quaternion& quat)
		{
			btVector3 btOrigin = ConvertToBt(v);
			btQuaternion btQuat = ConvertToBt(quat);

			btTransform tm;
			tm.setIdentity();
			tm.setRotation(btQuat);
			tm.setOrigin(btOrigin);

			return tm;
		}
	}
}