#include "stdafx.h"
#include "Math.h"

#include <DirectXMath.h>
#include <DirectXPackedVector.h>

using DirectX::operator*;
using DirectX::operator+;

#define STEREOSCALE 1.77777f
#define INVSTEREOSCALE (1.f / STEREOSCALE)

namespace est
{
	namespace math
	{
		std::random_device random_device;
		std::mt19937_64 mt19937_64(random_device());

		bool NearEqual(float S1, float S2, float Epsilon)
		{
			return (fabsf(S1 - S2) <= Epsilon);
		}

		float ModAngle(float Value)
		{
			return DirectX::XMScalarModAngle(Value);
		}

		float Sin(float Value)
		{
			return DirectX::XMScalarSin(Value);
		}

		float SinEst(float Value)
		{
			return DirectX::XMScalarSinEst(Value);
		}

		float Cos(float Value)
		{
			return DirectX::XMScalarCos(Value);
		}

		float CosEst(float Value)
		{
			return DirectX::XMScalarCosEst(Value);
		}

		void SinCos(_Out_ float* pSin, _Out_ float* pCos, float Value)
		{
			return DirectX::XMScalarSinCos(pSin, pCos, Value);
		}

		void SinCosEst(_Out_ float* pSin, _Out_ float* pCos, float Value)
		{
			return DirectX::XMScalarSinCosEst(pSin, pCos, Value);
		}

		float ASin(float Value)
		{
			return DirectX::XMScalarASin(Value);
		}

		float ASinEst(float Value)
		{
			return DirectX::XMScalarASinEst(Value);
		}

		float ACos(float Value)
		{
			return DirectX::XMScalarACos(Value);
		}

		float ACosEst(float Value)
		{
			return DirectX::XMScalarACosEst(Value);
		}

		float2 CompressNormal(const float3& f3Normal)
		{
			float2 ret = float2(f3Normal.x, f3Normal.y) / float2(f3Normal.z + 1.f);
			ret *= INVSTEREOSCALE;

			return ret;
		}

		float3 DeCompressNormal(const float2& f2Normal)
		{
			float3 nn;
			nn.x = f2Normal.x * STEREOSCALE;
			nn.y = f2Normal.y * STEREOSCALE;
			nn.z = 1.f;

			float g = 2.f / nn.Dot(nn);

			float3 n;
			n.x = g * nn.x;
			n.y = g * nn.y;
			n.z = g - 1.f;

			n.Normalize();

			return n;
		}

		float3 CalcTangent(const float3& f3Normal)
		{
			float3 c1 = f3Normal.Cross(float3(0.f, 0.f, 1.f));
			float3 c2 = f3Normal.Cross(float3(0.f, 1.f, 0.f));

			if (c1.LengthSquared() > c2.LengthSquared())
			{
				c1.Normalize();
				return c1;
			}
			else
			{
				c2.Normalize();
				return c2;
			}
		}

		float3 CalcBinormal(const float3& f3Normal, const float3& f3Tangent)
		{
			float3 binormal = f3Normal.Cross(f3Tangent);
			binormal.Normalize();

			return binormal;
		}

		UByte4::UByte4(float _x, float _y, float _z, float _w)
		{
			using namespace DirectX;
			PackedVector::XMStoreUByte4(reinterpret_cast<PackedVector::XMUBYTE4*>(this), XMVectorSet(_x, _y, _z, _w));
		}
		UByte4::UByte4(_In_reads_(4) const float *pArray)
		{
			using namespace DirectX;
			PackedVector::XMStoreUByte4(reinterpret_cast<PackedVector::XMUBYTE4*>(this), XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(pArray)));
		}

		int2::int2() : x(0), y(0) {}
		int2::int2(int x) : x(x), y(x) {}
		int2::int2(int x, int y) : x(x), y(y) {}
		int2::int2(const int2& V) { this->x = V.x; this->y = V.y; }

		const int2 int2::Zero = { 0, 0 };
		const int2 int2::One = { 1, 1 };
		const int2 int2::UnitX = { 1, 0 };
		const int2 int2::UnitY = { 1, 0 };

		int3::int3() : x(0), y(0), z(0) {}
		int3::int3(int x) : x(x), y(x), z(x) {}
		int3::int3(int x, int y, int z) : x(x), y(y), z(z) {}
		int3::int3(const int3& V) { this->x = V.x; this->y = V.y; this->z = V.z; }

		const int3 int3::Zero = { 0, 0, 0 };
		const int3 int3::One = { 1, 1, 1 };
		const int3 int3::UnitX = { 1, 0, 0 };
		const int3 int3::UnitY = { 0, 1, 0 };
		const int3 int3::UnitZ = { 0, 0, 1 };

		int4::int4() : x(0), y(0), z(0), w(0) {}
		int4::int4(int x) : x(x), y(x), z(x), w(x) {}
		int4::int4(int x, int y, int z, int w) : x(x), y(y), z(z), w(w) {}
		int4::int4(const int4& V) { this->x = V.x; this->y = V.y; this->z = V.z; this->w = V.w; }

		const int4 int4::Zero = { 0, 0, 0, 0 };
		const int4 int4::One = { 1, 1, 1, 1 };
		const int4 int4::UnitX = { 1, 0, 0, 0 };
		const int4 int4::UnitY = { 0, 1, 0, 0 };
		const int4 int4::UnitZ = { 0, 0, 1, 0 };
		const int4 int4::UnitW = { 0, 0, 0, 1 };

		uint2::uint2() : x(0), y(0) {}
		uint2::uint2(uint32_t x) : x(x), y(x) {}
		uint2::uint2(uint32_t x, uint32_t y) : x(x), y(y) {}
		uint2::uint2(const uint2& V) { this->x = V.x; this->y = V.y; }

		const uint2 uint2::Zero = { 0, 0 };
		const uint2 uint2::One = { 1, 1 };
		const uint2 uint2::UnitX = { 1, 0 };
		const uint2 uint2::UnitY = { 1, 0 };

		uint3::uint3() : x(0), y(0), z(0) {}
		uint3::uint3(uint32_t x) : x(x), y(x), z(x) {}
		uint3::uint3(uint32_t x, uint32_t y, uint32_t z) : x(x), y(y), z(z) {}
		uint3::uint3(const uint3& V) { this->x = V.x; this->y = V.y; this->z = V.z; }

		const uint3 uint3::Zero = { 0, 0, 0 };
		const uint3 uint3::One = { 1, 1, 1 };
		const uint3 uint3::UnitX = { 1, 0, 0 };
		const uint3 uint3::UnitY = { 0, 1, 0 };
		const uint3 uint3::UnitZ = { 0, 0, 1 };

		uint4::uint4() : x(0), y(0), z(0), w(0) {}
		uint4::uint4(uint32_t x) : x(x), y(x), z(x), w(x) {}
		uint4::uint4(uint32_t x, uint32_t y, uint32_t z, uint32_t w) : x(x), y(y), z(z), w(w) {}
		uint4::uint4(const uint4& V) { this->x = V.x; this->y = V.y; this->z = V.z; this->w = V.w; }

		const uint4 uint4::Zero = { 0, 0, 0, 0 };
		const uint4 uint4::One = { 1, 1, 1, 1 };
		const uint4 uint4::UnitX = { 1, 0, 0, 0 };
		const uint4 uint4::UnitY = { 0, 1, 0, 0 };
		const uint4 uint4::UnitZ = { 0, 0, 1, 0 };
		const uint4 uint4::UnitW = { 0, 0, 0, 1 };

		/****************************************************************************
		*
		* float2
		*
		****************************************************************************/

		float2::float2() : x(0.f), y(0.f) {}
		float2::float2(float x) : x(x), y(x) {}
		float2::float2(float x, float y) : x(x), y(y) {}
		float2::float2(_In_reads_(2) const float *pArray) : x(pArray[0]), y(pArray[1]) {}
		float2::float2(const __m128& V) { using namespace DirectX; XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(this), V); }
		float2::float2(const float2& V) { this->x = V.x; this->y = V.y; }

		float2::operator __m128() const
		{
			using namespace DirectX;

			return XMLoadFloat2(reinterpret_cast<const XMFLOAT2*>(this));
		}

		//------------------------------------------------------------------------------
		// Comparision operators
		//------------------------------------------------------------------------------

		bool float2::operator == (const float2& V) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			return XMVector2Equal(v1, v2);
		}

		bool float2::operator != (const float2& V) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			return XMVector2NotEqual(v1, v2);
		}

		//------------------------------------------------------------------------------
		// Assignment operators
		//------------------------------------------------------------------------------

		float2& float2::operator+= (const float2& V)
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			XMVECTOR X = XMVectorAdd(v1, v2);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(this), X);
			return *this;
		}

		float2& float2::operator-= (const float2& V)
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			XMVECTOR X = XMVectorSubtract(v1, v2);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(this), X);
			return *this;
		}

		float2& float2::operator*= (const float2& V)
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			XMVECTOR X = XMVectorMultiply(v1, v2);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(this), X);
			return *this;
		}

		float2& float2::operator*= (float S)
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVectorScale(v1, S);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(this), X);
			return *this;
		}

		float2& float2::operator/= (float S)
		{
			using namespace DirectX;
			assert(S != 0.0f);
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVectorScale(v1, 1.f / S);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(this), X);
			return *this;
		}

		//------------------------------------------------------------------------------
		// Binary operators
		//------------------------------------------------------------------------------

		float2 operator+ (const float2& V1, const float2& V2)
		{
			using namespace DirectX;
			XMVECTOR v1 = V1;
			XMVECTOR v2 = V2;
			XMVECTOR X = XMVectorAdd(v1, v2);
			float2 R;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&R), X);
			return R;
		}

		float2 operator- (const float2& V1, const float2& V2)
		{
			using namespace DirectX;
			XMVECTOR v1 = V1;
			XMVECTOR v2 = V2;
			XMVECTOR X = XMVectorSubtract(v1, v2);
			float2 R;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&R), X);
			return R;
		}

		float2 operator* (const float2& V1, const float2& V2)
		{
			using namespace DirectX;
			XMVECTOR v1 = V1;
			XMVECTOR v2 = V2;
			XMVECTOR X = XMVectorMultiply(v1, v2);
			float2 R;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&R), X);
			return R;
		}

		float2 operator* (const float2& V, float S)
		{
			using namespace DirectX;
			XMVECTOR v1 = V;
			XMVECTOR X = XMVectorScale(v1, S);
			float2 R;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&R), X);
			return R;
		}

		float2 operator/ (const float2& V1, const float2& V2)
		{
			using namespace DirectX;
			XMVECTOR v1 = V1;
			XMVECTOR v2 = V2;
			XMVECTOR X = XMVectorDivide(v1, v2);
			float2 R;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&R), X);
			return R;
		}

		float2 operator/ (const float2& V1, float S)
		{
			using namespace DirectX;
			XMVECTOR v1 = V1;
			XMVECTOR X = XMVectorScale(v1, 1.f / S);
			float2 R;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&R), X);
			return R;
		}

		float2 operator* (float S, const float2& V)
		{
			using namespace DirectX;
			XMVECTOR v1 = V;
			XMVECTOR X = XMVectorScale(v1, S);
			float2 R;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&R), X);
			return R;
		}

		//------------------------------------------------------------------------------
		// Vector operations
		//------------------------------------------------------------------------------

		bool float2::InBounds(const float2& Bounds) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = Bounds;
			return XMVector2InBounds(v1, v2);
		}

		float float2::Length() const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVector2Length(v1);
			return XMVectorGetX(X);
		}

		float float2::LengthSquared() const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVector2LengthSq(v1);
			return XMVectorGetX(X);
		}

		float float2::Dot(const float2& V) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			XMVECTOR X = XMVector2Dot(v1, v2);
			return XMVectorGetX(X);
		}

		void float2::Cross(const float2& V, float2& result) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			XMVECTOR R = XMVector2Cross(v1, v2);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), R);
		}

		float2 float2::Cross(const float2& V) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			XMVECTOR R = XMVector2Cross(v1, v2);

			float2 result;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), R);
			return result;
		}

		void float2::Normalize()
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVector2Normalize(v1);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(this), X);
		}

		void float2::Normalize(float2& result) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVector2Normalize(v1);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
		}

		void float2::Clamp(const float2& vmin, const float2& vmax)
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = vmin;
			XMVECTOR v3 = vmax;
			XMVECTOR X = XMVectorClamp(v1, v2, v3);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(this), X);
		}

		void float2::Clamp(const float2& vmin, const float2& vmax, float2& result) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = vmin;
			XMVECTOR v3 = vmax;
			XMVECTOR X = XMVectorClamp(v1, v2, v3);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
		}

		//------------------------------------------------------------------------------
		// Static functions
		//------------------------------------------------------------------------------

		float float2::Distance(const float2& v1, const float2& v2)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR V = XMVectorSubtract(x2, x1);
			XMVECTOR X = XMVector2Length(V);
			return XMVectorGetX(X);
		}

		float float2::DistanceSquared(const float2& v1, const float2& v2)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR V = XMVectorSubtract(x2, x1);
			XMVECTOR X = XMVector2LengthSq(V);
			return XMVectorGetX(X);
		}

		void float2::Min(const float2& v1, const float2& v2, float2& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorMin(x1, x2);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
		}

		float2 float2::Min(const float2& v1, const float2& v2)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorMin(x1, x2);

			float2 result;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
			return result;
		}

		void float2::Max(const float2& v1, const float2& v2, float2& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorMax(x1, x2);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
		}

		float2 float2::Max(const float2& v1, const float2& v2)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorMax(x1, x2);

			float2 result;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
			return result;
		}

		void float2::Lerp(const float2& v1, const float2& v2, float t, float2& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorLerp(x1, x2, t);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
		}

		float2 float2::Lerp(const float2& v1, const float2& v2, float t)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorLerp(x1, x2, t);

			float2 result;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
			return result;
		}

		void float2::SmoothStep(const float2& v1, const float2& v2, float t, float2& result)
		{
			using namespace DirectX;
			t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t);  // Clamp value to 0 to 1
			t = t*t*(3.f - 2.f*t);
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorLerp(x1, x2, t);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
		}

		float2 float2::SmoothStep(const float2& v1, const float2& v2, float t)
		{
			using namespace DirectX;
			t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t);  // Clamp value to 0 to 1
			t = t*t*(3.f - 2.f*t);
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorLerp(x1, x2, t);

			float2 result;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
			return result;
		}

		void float2::Barycentric(const float2& v1, const float2& v2, const float2& v3, float f, float g, float2& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR x3 = v3;
			XMVECTOR X = XMVectorBaryCentric(x1, x2, x3, f, g);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
		}

		float2 float2::Barycentric(const float2& v1, const float2& v2, const float2& v3, float f, float g)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR x3 = v3;
			XMVECTOR X = XMVectorBaryCentric(x1, x2, x3, f, g);

			float2 result;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
			return result;
		}

		void float2::CatmullRom(const float2& v1, const float2& v2, const float2& v3, const float2& v4, float t, float2& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR x3 = v3;
			XMVECTOR x4 = v4;
			XMVECTOR X = XMVectorCatmullRom(x1, x2, x3, x4, t);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
		}

		float2 float2::CatmullRom(const float2& v1, const float2& v2, const float2& v3, const float2& v4, float t)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR x3 = v3;
			XMVECTOR x4 = v4;
			XMVECTOR X = XMVectorCatmullRom(x1, x2, x3, x4, t);

			float2 result;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
			return result;
		}

		void float2::Hermite(const float2& v1, const float2& t1, const float2& v2, const float2& t2, float t, float2& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = t1;
			XMVECTOR x3 = v2;
			XMVECTOR x4 = t2;
			XMVECTOR X = XMVectorHermite(x1, x2, x3, x4, t);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
		}

		float2 float2::Hermite(const float2& v1, const float2& t1, const float2& v2, const float2& t2, float t)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = t1;
			XMVECTOR x3 = v2;
			XMVECTOR x4 = t2;
			XMVECTOR X = XMVectorHermite(x1, x2, x3, x4, t);

			float2 result;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
			return result;
		}

		void float2::Reflect(const float2& ivec, const float2& nvec, float2& result)
		{
			using namespace DirectX;
			XMVECTOR i = ivec;
			XMVECTOR n = nvec;
			XMVECTOR X = XMVector2Reflect(i, n);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
		}

		float2 float2::Reflect(const float2& ivec, const float2& nvec)
		{
			using namespace DirectX;
			XMVECTOR i = ivec;
			XMVECTOR n = nvec;
			XMVECTOR X = XMVector2Reflect(i, n);

			float2 result;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
			return result;
		}

		void float2::Refract(const float2& ivec, const float2& nvec, float refractionIndex, float2& result)
		{
			using namespace DirectX;
			XMVECTOR i = ivec;
			XMVECTOR n = nvec;
			XMVECTOR X = XMVector2Refract(i, n, refractionIndex);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
		}

		float2 float2::Refract(const float2& ivec, const float2& nvec, float refractionIndex)
		{
			using namespace DirectX;
			XMVECTOR i = ivec;
			XMVECTOR n = nvec;
			XMVECTOR X = XMVector2Refract(i, n, refractionIndex);

			float2 result;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
			return result;
		}

		void float2::Transform(const float2& v, const Quaternion& quat, float2& result)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMVECTOR q = quat;
			XMVECTOR X = XMVector3Rotate(v1, q);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
		}

		float2 float2::Transform(const float2& v, const Quaternion& quat)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMVECTOR q = quat;
			XMVECTOR X = XMVector3Rotate(v1, q);

			float2 result;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
			return result;
		}

		void float2::Transform(const float2& v, const Matrix& m, float2& result)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVECTOR X = XMVector2TransformCoord(v1, M);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
		}

		float2 float2::Transform(const float2& v, const Matrix& m)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVECTOR X = XMVector2TransformCoord(v1, M);

			float2 result;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
			return result;
		}

		_Use_decl_annotations_
			void float2::Transform(const float2* varray, size_t count, const Matrix& m, float2* resultArray)
		{
			using namespace DirectX;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVector2TransformCoordStream(reinterpret_cast<XMFLOAT2*>(resultArray), sizeof(XMFLOAT2), reinterpret_cast<const XMFLOAT2*>(varray), sizeof(XMFLOAT2), count, M);
		}

		void float2::Transform(const float2& v, const Matrix& m, float4& result)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVECTOR X = XMVector2Transform(v1, M);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
		}

		_Use_decl_annotations_
			void float2::Transform(const float2* varray, size_t count, const Matrix& m, float4* resultArray)
		{
			using namespace DirectX;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVector2TransformStream(reinterpret_cast<XMFLOAT4*>(resultArray), sizeof(XMFLOAT4), reinterpret_cast<const XMFLOAT2*>(varray), sizeof(XMFLOAT2), count, M);
		}

		void float2::TransformNormal(const float2& v, const Matrix& m, float2& result)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVECTOR X = XMVector2TransformNormal(v1, M);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
		}

		float2 float2::TransformNormal(const float2& v, const Matrix& m)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVECTOR X = XMVector2TransformNormal(v1, M);

			float2 result;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
			return result;
		}

		_Use_decl_annotations_
			void float2::TransformNormal(const float2* varray, size_t count, const Matrix& m, float2* resultArray)
		{
			using namespace DirectX;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVector2TransformNormalStream(reinterpret_cast<XMFLOAT2*>(resultArray), sizeof(XMFLOAT2), reinterpret_cast<const XMFLOAT2*>(varray), sizeof(XMFLOAT2), count, M);
		}

		const float2 float2::Zero = { 0.f, 0.f };
		const float2 float2::One = { 1.f, 1.f };
		const float2 float2::UnitX = { 1.f, 0.f };
		const float2 float2::UnitY = { 0.f, 1.f };

		/****************************************************************************
		*
		* float3
		*
		****************************************************************************/

		float3::float3() : x(0.f), y(0.f), z(0.f) {}
		float3::float3(float x) : x(x), y(x), z(x) {}
		float3::float3(float x, float y, float z) : x(x), y(y), z(z) {}
		float3::float3(_In_reads_(3) const float *pArray) : x(pArray[0]), y(pArray[1]), z(pArray[2]) {}
		float3::float3(const __m128& V) { using namespace DirectX; XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(this), V); }
		float3::float3(const float3& V) { this->x = V.x; this->y = V.y; this->z = V.z; }

		float3::operator __m128() const
		{
			using namespace DirectX;

			return XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(this));
		}

		//------------------------------------------------------------------------------
		// Comparision operators
		//------------------------------------------------------------------------------

		bool float3::operator == (const float3& V) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			return XMVector3Equal(v1, v2);
		}

		bool float3::operator != (const float3& V) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			return XMVector3NotEqual(v1, v2);
		}

		//------------------------------------------------------------------------------
		// Assignment operators
		//------------------------------------------------------------------------------

		float3& float3::operator+= (const float3& V)
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			XMVECTOR X = XMVectorAdd(v1, v2);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(this), X);
			return *this;
		}

		float3& float3::operator-= (const float3& V)
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			XMVECTOR X = XMVectorSubtract(v1, v2);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(this), X);
			return *this;
		}

		float3& float3::operator*= (const float3& V)
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			XMVECTOR X = XMVectorMultiply(v1, v2);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(this), X);
			return *this;
		}

		float3& float3::operator/= (const float3& V)
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			XMVECTOR X = XMVectorDivide(v1, v2);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(this), X);
			return *this;
		}

		float3& float3::operator*= (float S)
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVectorScale(v1, S);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(this), X);
			return *this;
		}

		float3& float3::operator/= (float S)
		{
			using namespace DirectX;
			assert(S != 0.0f);
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVectorScale(v1, 1.f / S);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(this), X);
			return *this;
		}

		//------------------------------------------------------------------------------
		// Urnary operators
		//------------------------------------------------------------------------------

		float3 float3::operator- () const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVectorNegate(v1);
			float3 R;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&R), X);
			return R;
		}

		//------------------------------------------------------------------------------
		// Binary operators
		//------------------------------------------------------------------------------

		float3 operator+ (const float3& V1, const float3& V2)
		{
			using namespace DirectX;
			XMVECTOR v1 = V1;
			XMVECTOR v2 = V2;
			XMVECTOR X = XMVectorAdd(v1, v2);
			float3 R;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&R), X);
			return R;
		}

		float3 operator- (const float3& V1, const float3& V2)
		{
			using namespace DirectX;
			XMVECTOR v1 = V1;
			XMVECTOR v2 = V2;
			XMVECTOR X = XMVectorSubtract(v1, v2);
			float3 R;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&R), X);
			return R;
		}

		float3 operator* (const float3& V1, const float3& V2)
		{
			using namespace DirectX;
			XMVECTOR v1 = V1;
			XMVECTOR v2 = V2;
			XMVECTOR X = XMVectorMultiply(v1, v2);
			float3 R;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&R), X);
			return R;
		}

		float3 operator* (const float3& V, float S)
		{
			using namespace DirectX;
			XMVECTOR v1 = V;
			XMVECTOR X = XMVectorScale(v1, S);
			float3 R;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&R), X);
			return R;
		}

		float3 operator/ (const float3& V1, const float3& V2)
		{
			using namespace DirectX;
			XMVECTOR v1 = V1;
			XMVECTOR v2 = V2;
			XMVECTOR X = XMVectorDivide(v1, v2);
			float3 R;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&R), X);
			return R;
		}

		float3 operator/ (const float3& V, float S)
		{
			using namespace DirectX;
			XMVECTOR v1 = V;
			XMVECTOR v2 = XMVectorSet(S, S, S, S);
			XMVECTOR X = XMVectorDivide(v1, v2);
			float3 R;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&R), X);
			return R;
		}

		float3 operator* (float S, const float3& V)
		{
			using namespace DirectX;
			XMVECTOR v1 = V;
			XMVECTOR X = XMVectorScale(v1, S);
			float3 R;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&R), X);
			return R;
		}

		//------------------------------------------------------------------------------
		// Vector operations
		//------------------------------------------------------------------------------

		bool float3::InBounds(const float3& Bounds) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = Bounds;
			return XMVector3InBounds(v1, v2);
		}

		float float3::Length() const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVector3Length(v1);
			return XMVectorGetX(X);
		}

		float float3::LengthSquared() const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVector3LengthSq(v1);
			return XMVectorGetX(X);
		}

		float float3::Dot(const float3& V) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			XMVECTOR X = XMVector3Dot(v1, v2);
			return XMVectorGetX(X);
		}

		void float3::Cross(const float3& V, float3& result) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			XMVECTOR R = XMVector3Cross(v1, v2);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), R);
		}

		float3 float3::Cross(const float3& V) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			XMVECTOR R = XMVector3Cross(v1, v2);

			float3 result;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), R);
			return result;
		}

		void float3::Normalize()
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVector3Normalize(v1);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(this), X);
		}

		void float3::Normalize(float3& result) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVector3Normalize(v1);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
		}

		void float3::Clamp(const float3& vmin, const float3& vmax)
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = vmin;
			XMVECTOR v3 = vmax;
			XMVECTOR X = XMVectorClamp(v1, v2, v3);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(this), X);
		}

		void float3::Clamp(const float3& vmin, const float3& vmax, float3& result) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = vmin;
			XMVECTOR v3 = vmax;
			XMVECTOR X = XMVectorClamp(v1, v2, v3);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
		}

		//------------------------------------------------------------------------------
		// Static functions
		//------------------------------------------------------------------------------

		float float3::Distance(const float3& v1, const float3& v2)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR V = XMVectorSubtract(x2, x1);
			XMVECTOR X = XMVector3Length(V);
			return XMVectorGetX(X);
		}

		float float3::DistanceSquared(const float3& v1, const float3& v2)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR V = XMVectorSubtract(x2, x1);
			XMVECTOR X = XMVector3LengthSq(V);
			return XMVectorGetX(X);
		}

		void float3::Min(const float3& v1, const float3& v2, float3& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorMin(x1, x2);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
		}

		float3 float3::Min(const float3& v1, const float3& v2)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorMin(x1, x2);

			float3 result;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
			return result;
		}

		void float3::Max(const float3& v1, const float3& v2, float3& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorMax(x1, x2);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
		}

		float3 float3::Max(const float3& v1, const float3& v2)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorMax(x1, x2);

			float3 result;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
			return result;
		}

		void float3::Lerp(const float3& v1, const float3& v2, float t, float3& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorLerp(x1, x2, t);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
		}

		float3 float3::Lerp(const float3& v1, const float3& v2, float t)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorLerp(x1, x2, t);

			float3 result;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
			return result;
		}

		void float3::SmoothStep(const float3& v1, const float3& v2, float t, float3& result)
		{
			using namespace DirectX;
			t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t);  // Clamp value to 0 to 1
			t = t*t*(3.f - 2.f*t);
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorLerp(x1, x2, t);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
		}

		float3 float3::SmoothStep(const float3& v1, const float3& v2, float t)
		{
			using namespace DirectX;
			t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t);  // Clamp value to 0 to 1
			t = t*t*(3.f - 2.f*t);
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorLerp(x1, x2, t);

			float3 result;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
			return result;
		}

		void float3::Barycentric(const float3& v1, const float3& v2, const float3& v3, float f, float g, float3& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR x3 = v3;;
			XMVECTOR X = XMVectorBaryCentric(x1, x2, x3, f, g);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
		}

		float3 float3::Barycentric(const float3& v1, const float3& v2, const float3& v3, float f, float g)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR x3 = v3;;
			XMVECTOR X = XMVectorBaryCentric(x1, x2, x3, f, g);

			float3 result;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
			return result;
		}

		void float3::CatmullRom(const float3& v1, const float3& v2, const float3& v3, const float3& v4, float t, float3& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR x3 = v3;;
			XMVECTOR x4 = v4;;
			XMVECTOR X = XMVectorCatmullRom(x1, x2, x3, x4, t);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
		}

		float3 float3::CatmullRom(const float3& v1, const float3& v2, const float3& v3, const float3& v4, float t)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR x3 = v3;;
			XMVECTOR x4 = v4;;
			XMVECTOR X = XMVectorCatmullRom(x1, x2, x3, x4, t);

			float3 result;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
			return result;
		}

		void float3::Hermite(const float3& v1, const float3& t1, const float3& v2, const float3& t2, float t, float3& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = t1;
			XMVECTOR x3 = v2;
			XMVECTOR x4 = t2;
			XMVECTOR X = XMVectorHermite(x1, x2, x3, x4, t);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
		}

		float3 float3::Hermite(const float3& v1, const float3& t1, const float3& v2, const float3& t2, float t)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = t1;
			XMVECTOR x3 = v2;
			XMVECTOR x4 = t2;
			XMVECTOR X = XMVectorHermite(x1, x2, x3, x4, t);

			float3 result;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
			return result;
		}

		void float3::Reflect(const float3& ivec, const float3& nvec, float3& result)
		{
			using namespace DirectX;
			XMVECTOR i = ivec;
			XMVECTOR n = nvec;
			XMVECTOR X = XMVector3Reflect(i, n);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
		}

		float3 float3::Reflect(const float3& ivec, const float3& nvec)
		{
			using namespace DirectX;
			XMVECTOR i = ivec;
			XMVECTOR n = nvec;
			XMVECTOR X = XMVector3Reflect(i, n);

			float3 result;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
			return result;
		}

		void float3::Refract(const float3& ivec, const float3& nvec, float refractionIndex, float3& result)
		{
			using namespace DirectX;
			XMVECTOR i = ivec;
			XMVECTOR n = nvec;
			XMVECTOR X = XMVector3Refract(i, n, refractionIndex);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
		}

		float3 float3::Refract(const float3& ivec, const float3& nvec, float refractionIndex)
		{
			using namespace DirectX;
			XMVECTOR i = ivec;
			XMVECTOR n = nvec;
			XMVECTOR X = XMVector3Refract(i, n, refractionIndex);

			float3 result;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
			return result;
		}

		void float3::Transform(const float3& v, const Quaternion& quat, float3& result)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMVECTOR q = quat;
			XMVECTOR X = XMVector3Rotate(v1, q);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
		}

		float3 float3::Transform(const float3& v, const Quaternion& quat)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMVECTOR q = quat;
			XMVECTOR X = XMVector3Rotate(v1, q);

			float3 result;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
			return result;
		}

		void float3::Transform(const float3& v, const Matrix& m, float3& result)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVECTOR X = XMVector3TransformCoord(v1, M);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
		}

		float3 float3::Transform(const float3& v, const Matrix& m)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVECTOR X = XMVector3TransformCoord(v1, M);

			float3 result;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
			return result;
		}

		_Use_decl_annotations_
			void float3::Transform(const float3* varray, size_t count, const Matrix& m, float3* resultArray)
		{
			using namespace DirectX;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVector3TransformCoordStream(reinterpret_cast<XMFLOAT3*>(resultArray), sizeof(XMFLOAT3), reinterpret_cast<const XMFLOAT3*>(varray), sizeof(XMFLOAT3), count, M);
		}

		void float3::Transform(const float3& v, const Matrix& m, float4& result)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVECTOR X = XMVector3Transform(v1, M);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
		}

		_Use_decl_annotations_
			void float3::Transform(const float3* varray, size_t count, const Matrix& m, float4* resultArray)
		{
			using namespace DirectX;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVector3TransformStream(reinterpret_cast<XMFLOAT4*>(resultArray), sizeof(XMFLOAT4), reinterpret_cast<const XMFLOAT3*>(varray), sizeof(XMFLOAT3), count, M);
		}

		void float3::TransformNormal(const float3& v, const Matrix& m, float3& result)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVECTOR X = XMVector3TransformNormal(v1, M);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
		}

		float3 float3::TransformNormal(const float3& v, const Matrix& m)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVECTOR X = XMVector3TransformNormal(v1, M);

			float3 result;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
			return result;
		}

		_Use_decl_annotations_
			void float3::TransformNormal(const float3* varray, size_t count, const Matrix& m, float3* resultArray)
		{
			using namespace DirectX;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVector3TransformNormalStream(reinterpret_cast<XMFLOAT3*>(resultArray), sizeof(XMFLOAT3), reinterpret_cast<const XMFLOAT3*>(varray), sizeof(XMFLOAT3), count, M);
		}

		float3 float3::FresnelTerm(const float3& v1, const float3& v2)
		{
			using namespace DirectX;
			XMVECTOR V1 = v1;
			XMVECTOR V2 = v2;

			float3 result;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), XMFresnelTerm(V1, V2));

			return result;
		}

		const float3 float3::Zero = { 0.f, 0.f, 0.f };
		const float3 float3::One = { 1.f, 1.f, 1.f };
		const float3 float3::UnitX = { 1.f, 0.f, 0.f };
		const float3 float3::UnitY = { 0.f, 1.f, 0.f };
		const float3 float3::UnitZ = { 0.f, 0.f, 1.f };
		const float3 float3::Up = { 0.f, 1.f, 0.f };
		const float3 float3::Down = { 0.f, -1.f, 0.f };
		const float3 float3::Right = { 1.f, 0.f, 0.f };
		const float3 float3::Left = { -1.f, 0.f, 0.f };
		const float3 float3::Forward = { 0.f, 0.f, 1.f };
		const float3 float3::Backward = { 0.f, 0.f, -1.f };

		/****************************************************************************
		*
		* float4
		*
		****************************************************************************/

		float4::float4() : x(0.f), y(0.f), z(0.f), w(0.f) {}
		float4::float4(float x) : x(x), y(x), z(x), w(x) {}
		float4::float4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
		float4::float4(_In_reads_(4) const float *pArray) : x(pArray[0]), y(pArray[1]), z(pArray[2]), w(pArray[3]) {}
		float4::float4(const __m128& V) { using namespace DirectX; XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), V); }
		float4::float4(const float4& V) { this->x = V.x; this->y = V.y; this->z = V.z; this->w = V.w; }

		float4::operator __m128() const
		{
			using namespace DirectX;

			return XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(this));
		}

		//------------------------------------------------------------------------------
		// Comparision operators
		//------------------------------------------------------------------------------

		bool float4::operator == (const float4& V) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			return XMVector4Equal(v1, v2);
		}

		bool float4::operator != (const float4& V) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			return XMVector4NotEqual(v1, v2);
		}

		//------------------------------------------------------------------------------
		// Assignment operators
		//------------------------------------------------------------------------------

		float4& float4::operator+= (const float4& V)
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			XMVECTOR X = XMVectorAdd(v1, v2);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), X);
			return *this;
		}

		float4& float4::operator-= (const float4& V)
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			XMVECTOR X = XMVectorSubtract(v1, v2);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), X);
			return *this;
		}

		float4& float4::operator*= (const float4& V)
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			XMVECTOR X = XMVectorMultiply(v1, v2);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), X);
			return *this;
		}

		float4& float4::operator*= (float S)
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVectorScale(v1, S);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), X);
			return *this;
		}

		float4& float4::operator/= (float S)
		{
			using namespace DirectX;
			assert(S != 0.0f);
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVectorScale(v1, 1.f / S);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), X);
			return *this;
		}

		//------------------------------------------------------------------------------
		// Urnary operators
		//------------------------------------------------------------------------------

		float4 float4::operator- () const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVectorNegate(v1);
			float4 R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R), X);
			return R;
		}

		//------------------------------------------------------------------------------
		// Binary operators
		//------------------------------------------------------------------------------

		float4 operator+ (const float4& V1, const float4& V2)
		{
			using namespace DirectX;
			XMVECTOR v1 = V1;
			XMVECTOR v2 = V2;
			XMVECTOR X = XMVectorAdd(v1, v2);
			float4 R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R), X);
			return R;
		}

		float4 operator- (const float4& V1, const float4& V2)
		{
			using namespace DirectX;
			XMVECTOR v1 = V1;
			XMVECTOR v2 = V2;
			XMVECTOR X = XMVectorSubtract(v1, v2);
			float4 R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R), X);
			return R;
		}

		float4 operator* (const float4& V1, const float4& V2)
		{
			using namespace DirectX;
			XMVECTOR v1 = V1;
			XMVECTOR v2 = V2;
			XMVECTOR X = XMVectorMultiply(v1, v2);
			float4 R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R), X);
			return R;
		}

		float4 operator* (const float4& V, float S)
		{
			using namespace DirectX;
			XMVECTOR v1 = V;
			XMVECTOR X = XMVectorScale(v1, S);
			float4 R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R), X);
			return R;
		}

		float4 operator/ (const float4& V1, const float4& V2)
		{
			using namespace DirectX;
			XMVECTOR v1 = V1;
			XMVECTOR v2 = V2;
			XMVECTOR X = XMVectorDivide(v1, v2);
			float4 R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R), X);
			return R;
		}

		float4 operator/ (const float4& V, float S)
		{
			using namespace DirectX;

			XMVECTOR v1 = V;
			XMVECTOR v2 = XMVectorSet(S, S, S, S);
			XMVECTOR X = XMVectorDivide(v1, v2);
			float4 R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R), X);
			return R;
		}

		float4 operator* (float S, const float4& V)
		{
			using namespace DirectX;
			XMVECTOR v1 = V;
			XMVECTOR X = XMVectorScale(v1, S);
			float4 R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R), X);
			return R;
		}

		//------------------------------------------------------------------------------
		// Vector operations
		//------------------------------------------------------------------------------

		bool float4::InBounds(const float4& Bounds) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = Bounds;
			return XMVector4InBounds(v1, v2);
		}

		float float4::Length() const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVector4Length(v1);
			return XMVectorGetX(X);
		}

		float float4::LengthSquared() const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVector4LengthSq(v1);
			return XMVectorGetX(X);
		}

		float float4::Dot(const float4& V) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			XMVECTOR X = XMVector4Dot(v1, v2);
			return XMVectorGetX(X);
		}

		void float4::Cross(const float4& v1, const float4& v2, float4& result) const
		{
			using namespace DirectX;
			XMVECTOR x1 = *this;
			XMVECTOR x2 = v1;
			XMVECTOR x3 = v2;
			XMVECTOR R = XMVector4Cross(x1, x2, x3);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), R);
		}

		float4 float4::Cross(const float4& v1, const float4& v2) const
		{
			using namespace DirectX;
			XMVECTOR x1 = *this;
			XMVECTOR x2 = v1;
			XMVECTOR x3 = v2;
			XMVECTOR R = XMVector4Cross(x1, x2, x3);

			float4 result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), R);
			return result;
		}

		void float4::Normalize()
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVector4Normalize(v1);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), X);
		}

		void float4::Normalize(float4& result) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVector4Normalize(v1);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
		}

		void float4::Clamp(const float4& vmin, const float4& vmax)
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = vmin;
			XMVECTOR v3 = vmax;
			XMVECTOR X = XMVectorClamp(v1, v2, v3);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), X);
		}

		void float4::Clamp(const float4& vmin, const float4& vmax, float4& result) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = vmin;
			XMVECTOR v3 = vmax;
			XMVECTOR X = XMVectorClamp(v1, v2, v3);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
		}

		//------------------------------------------------------------------------------
		// Static functions
		//------------------------------------------------------------------------------

		float float4::Distance(const float4& v1, const float4& v2)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR V = XMVectorSubtract(x2, x1);
			XMVECTOR X = XMVector4Length(V);
			return XMVectorGetX(X);
		}

		float float4::DistanceSquared(const float4& v1, const float4& v2)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR V = XMVectorSubtract(x2, x1);
			XMVECTOR X = XMVector4LengthSq(V);
			return XMVectorGetX(X);
		}

		void float4::Min(const float4& v1, const float4& v2, float4& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorMin(x1, x2);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
		}

		float4 float4::Min(const float4& v1, const float4& v2)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorMin(x1, x2);

			float4 result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
			return result;
		}

		void float4::Max(const float4& v1, const float4& v2, float4& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorMax(x1, x2);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
		}

		float4 float4::Max(const float4& v1, const float4& v2)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorMax(x1, x2);

			float4 result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
			return result;
		}

		void float4::Lerp(const float4& v1, const float4& v2, float t, float4& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorLerp(x1, x2, t);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
		}

		float4 float4::Lerp(const float4& v1, const float4& v2, float t)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorLerp(x1, x2, t);

			float4 result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
			return result;
		}

		void float4::SmoothStep(const float4& v1, const float4& v2, float t, float4& result)
		{
			using namespace DirectX;
			t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t);  // Clamp value to 0 to 1
			t = t*t*(3.f - 2.f*t);
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorLerp(x1, x2, t);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
		}

		float4 float4::SmoothStep(const float4& v1, const float4& v2, float t)
		{
			using namespace DirectX;
			t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t);  // Clamp value to 0 to 1
			t = t*t*(3.f - 2.f*t);
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorLerp(x1, x2, t);

			float4 result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
			return result;
		}

		void float4::Barycentric(const float4& v1, const float4& v2, const float4& v3, float f, float g, float4& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR x3 = v3;
			XMVECTOR X = XMVectorBaryCentric(x1, x2, x3, f, g);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
		}

		float4 float4::Barycentric(const float4& v1, const float4& v2, const float4& v3, float f, float g)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR x3 = v3;
			XMVECTOR X = XMVectorBaryCentric(x1, x2, x3, f, g);

			float4 result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
			return result;
		}

		void float4::CatmullRom(const float4& v1, const float4& v2, const float4& v3, const float4& v4, float t, float4& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR x3 = v3;
			XMVECTOR x4 = v4;
			XMVECTOR X = XMVectorCatmullRom(x1, x2, x3, x4, t);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
		}

		float4 float4::CatmullRom(const float4& v1, const float4& v2, const float4& v3, const float4& v4, float t)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR x3 = v3;
			XMVECTOR x4 = v4;
			XMVECTOR X = XMVectorCatmullRom(x1, x2, x3, x4, t);

			float4 result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
			return result;
		}

		void float4::Hermite(const float4& v1, const float4& t1, const float4& v2, const float4& t2, float t, float4& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = t1;
			XMVECTOR x3 = v2;
			XMVECTOR x4 = t2;
			XMVECTOR X = XMVectorHermite(x1, x2, x3, x4, t);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
		}

		float4 float4::Hermite(const float4& v1, const float4& t1, const float4& v2, const float4& t2, float t)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = t1;
			XMVECTOR x3 = v2;
			XMVECTOR x4 = t2;
			XMVECTOR X = XMVectorHermite(x1, x2, x3, x4, t);

			float4 result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
			return result;
		}

		void float4::Reflect(const float4& ivec, const float4& nvec, float4& result)
		{
			using namespace DirectX;
			XMVECTOR i = ivec;
			XMVECTOR n = nvec;
			XMVECTOR X = XMVector4Reflect(i, n);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
		}

		float4 float4::Reflect(const float4& ivec, const float4& nvec)
		{
			using namespace DirectX;
			XMVECTOR i = ivec;
			XMVECTOR n = nvec;
			XMVECTOR X = XMVector4Reflect(i, n);

			float4 result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
			return result;
		}

		void float4::Refract(const float4& ivec, const float4& nvec, float refractionIndex, float4& result)
		{
			using namespace DirectX;
			XMVECTOR i = ivec;
			XMVECTOR n = nvec;
			XMVECTOR X = XMVector4Refract(i, n, refractionIndex);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
		}

		float4 float4::Refract(const float4& ivec, const float4& nvec, float refractionIndex)
		{
			using namespace DirectX;
			XMVECTOR i = ivec;
			XMVECTOR n = nvec;
			XMVECTOR X = XMVector4Refract(i, n, refractionIndex);

			float4 result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
			return result;
		}

		void float4::Transform(const float2& v, const Quaternion& quat, float4& result)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMVECTOR q = quat;
			XMVECTOR X = XMVector3Rotate(v1, q);
			X = XMVectorSelect(g_XMIdentityR3, X, g_XMSelect1110); // result.w = 1.f
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
		}

		float4 float4::Transform(const float2& v, const Quaternion& quat)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMVECTOR q = quat;
			XMVECTOR X = XMVector3Rotate(v1, q);
			X = XMVectorSelect(g_XMIdentityR3, X, g_XMSelect1110); // result.w = 1.f

			float4 result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
			return result;
		}

		void float4::Transform(const float3& v, const Quaternion& quat, float4& result)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMVECTOR q = quat;
			XMVECTOR X = XMVector3Rotate(v1, q);
			X = XMVectorSelect(g_XMIdentityR3, X, g_XMSelect1110); // result.w = 1.f
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
		}

		float4 float4::Transform(const float3& v, const Quaternion& quat)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMVECTOR q = quat;
			XMVECTOR X = XMVector3Rotate(v1, q);
			X = XMVectorSelect(g_XMIdentityR3, X, g_XMSelect1110); // result.w = 1.f

			float4 result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
			return result;
		}

		void float4::Transform(const float4& v, const Quaternion& quat, float4& result)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMVECTOR q = quat;
			XMVECTOR X = XMVector3Rotate(v1, q);
			X = XMVectorSelect(v1, X, g_XMSelect1110); // result.w = v.w
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
		}

		float4 float4::Transform(const float4& v, const Quaternion& quat)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMVECTOR q = quat;
			XMVECTOR X = XMVector3Rotate(v1, q);
			X = XMVectorSelect(v1, X, g_XMSelect1110); // result.w = v.w

			float4 result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
			return result;
		}

		void float4::Transform(const float4& v, const Matrix& m, float4& result)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVECTOR X = XMVector4Transform(v1, M);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
		}

		float4 float4::Transform(const float4& v, const Matrix& m)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVECTOR X = XMVector4Transform(v1, M);

			float4 result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
			return result;
		}

		_Use_decl_annotations_
			void float4::Transform(const float4* varray, size_t count, const Matrix& m, float4* resultArray)
		{
			using namespace DirectX;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVector4TransformStream(reinterpret_cast<XMFLOAT4*>(resultArray), sizeof(XMFLOAT4), reinterpret_cast<const XMFLOAT4*>(varray), sizeof(XMFLOAT4), count, M);
		}

		const float4 float4::Zero = { 0.f, 0.f, 0.f, 0.f };
		const float4 float4::One = { 1.f, 1.f, 1.f, 1.f };
		const float4 float4::UnitX = { 1.f, 0.f, 0.f, 0.f };
		const float4 float4::UnitY = { 0.f, 1.f, 0.f, 0.f };
		const float4 float4::UnitZ = { 0.f, 0.f, 1.f, 0.f };
		const float4 float4::UnitW = { 0.f, 0.f, 0.f, 1.f };

		/****************************************************************************
		*
		* Matrix
		*
		****************************************************************************/

		Matrix::Matrix()
			: _11(1.f), _12(0.f), _13(0.f), _14(0.f)
			, _21(0.f), _22(1.f), _23(0.f), _24(0.f)
			, _31(0.f), _32(0.f), _33(1.f), _34(0.f)
			, _41(0.f), _42(0.f), _43(0.f), _44(1.f)
		{}
		Matrix::Matrix(float m00, float m01, float m02, float m03,
			float m10, float m11, float m12, float m13,
			float m20, float m21, float m22, float m23,
			float m30, float m31, float m32, float m33)
			: _11(m00), _12(m01), _13(m02), _14(m03)
			, _21(m10), _22(m11), _23(m12), _24(m13)
			, _31(m20), _32(m21), _33(m22), _34(m23)
			, _41(m30), _42(m31), _43(m32), _44(m33)
		{}
		Matrix::Matrix(const float3& r0, const float3& r1, const float3& r2)
			: _11(r0.x), _12(r0.y), _13(r0.z), _14(0.f)
			, _21(r1.x), _22(r1.y), _23(r1.z), _24(0.f)
			, _31(r2.x), _32(r2.y), _33(r2.z), _34(0.f)
			, _41(0.f), _42(0.f), _43(0.f), _44(0.f)
		{}
		Matrix::Matrix(const float4& r0, const float4& r1, const float4& r2, const float4& r3)
			: _11(r0.x), _12(r0.y), _13(r0.z), _14(r0.w)
			, _21(r1.x), _22(r1.y), _23(r1.z), _24(r1.w)
			, _31(r2.x), _32(r2.y), _33(r2.z), _34(r2.w)
			, _41(r3.x), _42(r3.y), _43(r3.z), _44(r3.w)
		{}
		Matrix::Matrix(const Matrix& M) { memcpy_s(this, sizeof(float) * 16, &M, sizeof(Matrix)); }

		//------------------------------------------------------------------------------
		// Comparision operators
		//------------------------------------------------------------------------------

		bool Matrix::operator == (const Matrix& M) const
		{
			using namespace DirectX;
			XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_11));
			XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_21));
			XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_31));
			XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_41));

			XMVECTOR y1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._11));
			XMVECTOR y2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._21));
			XMVECTOR y3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._31));
			XMVECTOR y4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._41));

			return (XMVector4Equal(x1, y1)
				&& XMVector4Equal(x2, y2)
				&& XMVector4Equal(x3, y3)
				&& XMVector4Equal(x4, y4)) != 0;
		}

		bool Matrix::operator != (const Matrix& M) const
		{
			using namespace DirectX;
			XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_11));
			XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_21));
			XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_31));
			XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_41));

			XMVECTOR y1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._11));
			XMVECTOR y2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._21));
			XMVECTOR y3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._31));
			XMVECTOR y4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._41));

			return (XMVector4NotEqual(x1, y1)
				|| XMVector4NotEqual(x2, y2)
				|| XMVector4NotEqual(x3, y3)
				|| XMVector4NotEqual(x4, y4)) != 0;
		}

		//------------------------------------------------------------------------------
		// Assignment operators
		//------------------------------------------------------------------------------

		Matrix& Matrix::operator+= (const Matrix& M)
		{
			using namespace DirectX;
			XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_11));
			XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_21));
			XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_31));
			XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_41));

			XMVECTOR y1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._11));
			XMVECTOR y2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._21));
			XMVECTOR y3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._31));
			XMVECTOR y4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._41));

			x1 = XMVectorAdd(x1, y1);
			x2 = XMVectorAdd(x2, y2);
			x3 = XMVectorAdd(x3, y3);
			x4 = XMVectorAdd(x4, y4);

			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_11), x1);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_21), x2);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_31), x3);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_41), x4);
			return *this;
		}

		Matrix& Matrix::operator-= (const Matrix& M)
		{
			using namespace DirectX;
			XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_11));
			XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_21));
			XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_31));
			XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_41));

			XMVECTOR y1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._11));
			XMVECTOR y2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._21));
			XMVECTOR y3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._31));
			XMVECTOR y4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._41));

			x1 = XMVectorSubtract(x1, y1);
			x2 = XMVectorSubtract(x2, y2);
			x3 = XMVectorSubtract(x3, y3);
			x4 = XMVectorSubtract(x4, y4);

			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_11), x1);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_21), x2);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_31), x3);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_41), x4);
			return *this;
		}

		Matrix& Matrix::operator*= (const Matrix& M)
		{
			using namespace DirectX;
			XMMATRIX M1 = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(this));
			XMMATRIX M2 = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMMATRIX X = XMMatrixMultiply(M1, M2);
			XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(this), X);
			return *this;
		}

		Matrix& Matrix::operator*= (float S)
		{
			using namespace DirectX;
			XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_11));
			XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_21));
			XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_31));
			XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_41));

			x1 = XMVectorScale(x1, S);
			x2 = XMVectorScale(x2, S);
			x3 = XMVectorScale(x3, S);
			x4 = XMVectorScale(x4, S);

			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_11), x1);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_21), x2);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_31), x3);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_41), x4);
			return *this;
		}

		Matrix& Matrix::operator/= (float S)
		{
			using namespace DirectX;
			assert(S != 0.f);
			XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_11));
			XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_21));
			XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_31));
			XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_41));

			float rs = 1.f / S;

			x1 = XMVectorScale(x1, rs);
			x2 = XMVectorScale(x2, rs);
			x3 = XMVectorScale(x3, rs);
			x4 = XMVectorScale(x4, rs);

			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_11), x1);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_21), x2);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_31), x3);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_41), x4);
			return *this;
		}

		Matrix& Matrix::operator/= (const Matrix& M)
		{
			using namespace DirectX;
			XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_11));
			XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_21));
			XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_31));
			XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<XMFLOAT4*>(&_41));

			XMVECTOR y1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._11));
			XMVECTOR y2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._21));
			XMVECTOR y3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._31));
			XMVECTOR y4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._41));

			x1 = XMVectorDivide(x1, y1);
			x2 = XMVectorDivide(x2, y2);
			x3 = XMVectorDivide(x3, y3);
			x4 = XMVectorDivide(x4, y4);

			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_11), x1);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_21), x2);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_31), x3);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&_41), x4);
			return *this;
		}

		//------------------------------------------------------------------------------
		// Urnary operators
		//------------------------------------------------------------------------------

		Matrix Matrix::operator- () const
		{
			using namespace DirectX;
			XMVECTOR v1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_11));
			XMVECTOR v2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_21));
			XMVECTOR v3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_31));
			XMVECTOR v4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&_41));

			v1 = XMVectorNegate(v1);
			v2 = XMVectorNegate(v2);
			v3 = XMVectorNegate(v3);
			v4 = XMVectorNegate(v4);

			Matrix R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._11), v1);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._21), v2);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._31), v3);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._41), v4);
			return R;
		}

		//------------------------------------------------------------------------------
		// Binary operators
		//------------------------------------------------------------------------------

		Matrix operator+ (const Matrix& M1, const Matrix& M2)
		{
			using namespace DirectX;
			XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._11));
			XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._21));
			XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._31));
			XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._41));

			XMVECTOR y1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._11));
			XMVECTOR y2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._21));
			XMVECTOR y3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._31));
			XMVECTOR y4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._41));

			x1 = XMVectorAdd(x1, y1);
			x2 = XMVectorAdd(x2, y2);
			x3 = XMVectorAdd(x3, y3);
			x4 = XMVectorAdd(x4, y4);

			Matrix R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._11), x1);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._21), x2);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._31), x3);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._41), x4);
			return R;
		}

		Matrix operator- (const Matrix& M1, const Matrix& M2)
		{
			using namespace DirectX;
			XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._11));
			XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._21));
			XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._31));
			XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._41));

			XMVECTOR y1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._11));
			XMVECTOR y2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._21));
			XMVECTOR y3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._31));
			XMVECTOR y4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._41));

			x1 = XMVectorSubtract(x1, y1);
			x2 = XMVectorSubtract(x2, y2);
			x3 = XMVectorSubtract(x3, y3);
			x4 = XMVectorSubtract(x4, y4);

			Matrix R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._11), x1);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._21), x2);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._31), x3);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._41), x4);
			return R;
		}

		Matrix operator* (const Matrix& M1, const Matrix& M2)
		{
			using namespace DirectX;
			XMMATRIX m1 = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&M1));
			XMMATRIX m2 = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&M2));
			XMMATRIX X = XMMatrixMultiply(m1, m2);

			Matrix R;
			XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&R), X);
			return R;
		}

		Matrix operator* (const Matrix& M, float S)
		{
			using namespace DirectX;
			XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._11));
			XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._21));
			XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._31));
			XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._41));

			x1 = XMVectorScale(x1, S);
			x2 = XMVectorScale(x2, S);
			x3 = XMVectorScale(x3, S);
			x4 = XMVectorScale(x4, S);

			Matrix R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._11), x1);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._21), x2);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._31), x3);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._41), x4);
			return R;
		}

		Matrix operator/ (const Matrix& M, float S)
		{
			using namespace DirectX;
			assert(S != 0.f);

			XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._11));
			XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._21));
			XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._31));
			XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._41));

			float rs = 1.f / S;

			x1 = XMVectorScale(x1, rs);
			x2 = XMVectorScale(x2, rs);
			x3 = XMVectorScale(x3, rs);
			x4 = XMVectorScale(x4, rs);

			Matrix R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._11), x1);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._21), x2);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._31), x3);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._41), x4);
			return R;
		}

		Matrix operator/ (const Matrix& M1, const Matrix& M2)
		{
			using namespace DirectX;
			XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._11));
			XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._21));
			XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._31));
			XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._41));

			XMVECTOR y1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._11));
			XMVECTOR y2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._21));
			XMVECTOR y3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._31));
			XMVECTOR y4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._41));

			x1 = XMVectorDivide(x1, y1);
			x2 = XMVectorDivide(x2, y2);
			x3 = XMVectorDivide(x3, y3);
			x4 = XMVectorDivide(x4, y4);

			Matrix R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._11), x1);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._21), x2);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._31), x3);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._41), x4);
			return R;
		}

		Matrix operator* (float S, const Matrix& M)
		{
			using namespace DirectX;

			XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._11));
			XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._21));
			XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._31));
			XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M._41));

			x1 = XMVectorScale(x1, S);
			x2 = XMVectorScale(x2, S);
			x3 = XMVectorScale(x3, S);
			x4 = XMVectorScale(x4, S);

			Matrix R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._11), x1);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._21), x2);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._31), x3);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R._41), x4);
			return R;
		}

		//------------------------------------------------------------------------------
		// Matrix operations
		//------------------------------------------------------------------------------

		bool Matrix::Decompose(float3& scale, Quaternion& rotation, float3& translation) const
		{
			using namespace DirectX;

			XMVECTOR s, r, t;
			XMMATRIX matrix = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(this));

			if (XMMatrixDecompose(&s, &r, &t, matrix) == false)
				return false;

			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&scale), s);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&rotation), r);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&translation), t);

			return true;
		}

		Matrix Matrix::Compose(const float3& scale, const Quaternion& rotation, const float3& translation)
		{
			Matrix result;
			Compose(scale, rotation, translation, result);
			return result;
		}

		void Matrix::Compose(const float3& scale, const Quaternion& rotation, const float3& translation, Matrix& result)
		{
			using namespace DirectX;

			XMVECTOR s = scale;
			XMVECTOR r = rotation;
			XMVECTOR t = translation;

			XMMATRIX m = XMMatrixAffineTransformation(s, g_XMZero, r, t);

			XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&result), m);
		}

		Matrix Matrix::Transpose() const
		{
			using namespace DirectX;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(this));
			Matrix R;
			XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&R), XMMatrixTranspose(M));
			return R;
		}

		void Matrix::Transpose(Matrix& result) const
		{
			using namespace DirectX;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(this));
			XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&result), XMMatrixTranspose(M));
		}

		Matrix Matrix::Invert() const
		{
			using namespace DirectX;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(this));
			Matrix R;
			XMVECTOR det;
			XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&R), XMMatrixInverse(&det, M));
			return R;
		}

		void Matrix::Invert(Matrix& result) const
		{
			using namespace DirectX;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(this));
			XMVECTOR det;
			XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&result), XMMatrixInverse(&det, M));
		}

		float Matrix::Determinant() const
		{
			using namespace DirectX;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(this));
			return XMVectorGetX(XMMatrixDeterminant(M));
		}

		//------------------------------------------------------------------------------
		// Static functions
		//------------------------------------------------------------------------------

		_Use_decl_annotations_
			Matrix Matrix::CreateBillboard(const float3& object, const float3& cameraPosition, const float3& cameraUp, const float3* cameraForward)
		{
			using namespace DirectX;
			XMVECTOR O = object;
			XMVECTOR C = cameraPosition;
			XMVECTOR Z = XMVectorSubtract(O, C);

			XMVECTOR N = XMVector3LengthSq(Z);
			if (XMVector3Less(N, g_XMEpsilon))
			{
				if (cameraForward)
				{
					XMVECTOR F = *cameraForward;
					Z = XMVectorNegate(F);
				}
				else
				{
					Z = g_XMNegIdentityR2;
				}
			}
			else
			{
				Z = XMVector3Normalize(Z);
			}

			XMVECTOR up = cameraUp;;
			XMVECTOR X = XMVector3Cross(up, Z);
			X = XMVector3Normalize(X);

			XMVECTOR Y = XMVector3Cross(Z, X);

			XMMATRIX M;
			M.r[0] = X;
			M.r[1] = Y;
			M.r[2] = Z;
			M.r[3] = XMVectorSetW(O, 1.f);

			Matrix R;
			XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&R), M);
			return R;
		}

		_Use_decl_annotations_
			Matrix Matrix::CreateConstrainedBillboard(const float3& object, const float3& cameraPosition, const float3& rotateAxis,
				const float3* cameraForward, const float3* objectForward)
		{
			using namespace DirectX;

			static const XMVECTORF32 s_minAngle = { 0.99825467075f, 0.99825467075f, 0.99825467075f, 0.99825467075f }; // 1.0 - XMConvertToRadians( 0.1f );

			XMVECTOR O = object;
			XMVECTOR C = cameraPosition;
			XMVECTOR faceDir = XMVectorSubtract(O, C);

			XMVECTOR N = XMVector3LengthSq(faceDir);
			if (XMVector3Less(N, g_XMEpsilon))
			{
				if (cameraForward)
				{
					XMVECTOR F = *cameraForward;
					faceDir = XMVectorNegate(F);
				}
				else
					faceDir = g_XMNegIdentityR2;
			}
			else
			{
				faceDir = XMVector3Normalize(faceDir);
			}

			XMVECTOR Y = rotateAxis;
			XMVECTOR X, Z;

			XMVECTOR dot = XMVectorAbs(XMVector3Dot(Y, faceDir));
			if (XMVector3Greater(dot, s_minAngle))
			{
				if (objectForward)
				{
					Z = *objectForward;
					dot = XMVectorAbs(XMVector3Dot(Y, Z));
					if (XMVector3Greater(dot, s_minAngle))
					{
						dot = XMVectorAbs(XMVector3Dot(Y, g_XMNegIdentityR2));
						Z = (XMVector3Greater(dot, s_minAngle)) ? g_XMIdentityR0 : g_XMNegIdentityR2;
					}
				}
				else
				{
					dot = XMVectorAbs(XMVector3Dot(Y, g_XMNegIdentityR2));
					Z = (XMVector3Greater(dot, s_minAngle)) ? g_XMIdentityR0 : g_XMNegIdentityR2;
				}

				X = XMVector3Cross(Y, Z);
				X = XMVector3Normalize(X);

				Z = XMVector3Cross(X, Y);
				Z = XMVector3Normalize(Z);
			}
			else
			{
				X = XMVector3Cross(Y, faceDir);
				X = XMVector3Normalize(X);

				Z = XMVector3Cross(X, Y);
				Z = XMVector3Normalize(Z);
			}

			XMMATRIX M;
			M.r[0] = X;
			M.r[1] = Y;
			M.r[2] = Z;
			M.r[3] = XMVectorSetW(O, 1.f);

			Matrix R;
			XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&R), M);
			return R;
		}

		Matrix Matrix::CreateTranslation(const float3& position)
		{
			using namespace DirectX;
			Matrix R;
			XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&R), XMMatrixTranslation(position.x, position.y, position.z));
			return R;
		}

		Matrix Matrix::CreateTranslation(float x, float y, float z)
		{
			using namespace DirectX;
			Matrix R;
			XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&R), XMMatrixTranslation(x, y, z));
			return R;
		}

		Matrix Matrix::CreateScale(const float3& scales)
		{
			using namespace DirectX;
			Matrix R;
			XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&R), XMMatrixScaling(scales.x, scales.y, scales.z));
			return R;
		}

		Matrix Matrix::CreateScale(float xs, float ys, float zs)
		{
			using namespace DirectX;
			Matrix R;
			XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&R), XMMatrixScaling(xs, ys, zs));
			return R;
		}

		Matrix Matrix::CreateScale(float scale)
		{
			using namespace DirectX;
			Matrix R;
			XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&R), XMMatrixScaling(scale, scale, scale));
			return R;
		}

		Matrix Matrix::CreateRotationX(float radians)
		{
			using namespace DirectX;
			Matrix R;
			XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&R), XMMatrixRotationX(radians));
			return R;
		}

		Matrix Matrix::CreateRotationY(float radians)
		{
			using namespace DirectX;
			Matrix R;
			XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&R), XMMatrixRotationY(radians));
			return R;
		}

		Matrix Matrix::CreateRotationZ(float radians)
		{
			using namespace DirectX;
			Matrix R;
			XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&R), XMMatrixRotationZ(radians));
			return R;
		}

		Matrix Matrix::CreateFromAxisAngle(const float3& axis, float angle)
		{
			using namespace DirectX;
			Matrix R;
			XMVECTOR a = axis;
			XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&R), XMMatrixRotationAxis(a, angle));
			return R;
		}

		Matrix Matrix::CreatePerspectiveFieldOfView(float fov, float aspectRatio, float nearPlane, float farPlane)
		{
			using namespace DirectX;
			Matrix R;
			XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&R), XMMatrixPerspectiveFovLH(fov, aspectRatio, nearPlane, farPlane));
			return R;
		}

		Matrix Matrix::CreatePerspective(float width, float height, float nearPlane, float farPlane)
		{
			using namespace DirectX;
			Matrix R;
			XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&R), XMMatrixPerspectiveLH(width, height, nearPlane, farPlane));
			return R;
		}

		Matrix Matrix::CreatePerspectiveOffCenter(float left, float right, float bottom, float top, float nearPlane, float farPlane)
		{
			using namespace DirectX;
			Matrix R;
			XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&R), XMMatrixPerspectiveOffCenterLH(left, right, bottom, top, nearPlane, farPlane));
			return R;
		}

		Matrix Matrix::CreateOrthographic(float width, float height, float zNearPlane, float zFarPlane)
		{
			using namespace DirectX;
			Matrix R;
			XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&R), XMMatrixOrthographicLH(width, height, zNearPlane, zFarPlane));
			return R;
		}

		Matrix Matrix::CreateOrthographicOffCenter(float left, float right, float bottom, float top, float zNearPlane, float zFarPlane)
		{
			using namespace DirectX;
			Matrix R;
			XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&R), XMMatrixOrthographicOffCenterLH(left, right, bottom, top, zNearPlane, zFarPlane));
			return R;
		}

		Matrix Matrix::CreateLookAt(const float3& eye, const float3& target, const float3& up)
		{
			using namespace DirectX;
			Matrix R;
			XMVECTOR eyev = eye;
			XMVECTOR targetv = target;
			XMVECTOR upv = up;
			XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&R), XMMatrixLookAtLH(eyev, targetv, upv));
			return R;
		}

		Matrix Matrix::CreateWorld(const float3& position, const float3& forward, const float3& up)
		{
			using namespace DirectX;
			XMVECTOR zaxis = XMVector3Normalize(XMVectorNegate(forward));
			XMVECTOR yaxis = up;
			XMVECTOR xaxis = XMVector3Normalize(XMVector3Cross(yaxis, zaxis));
			yaxis = XMVector3Cross(zaxis, xaxis);

			Matrix R;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&R._11), xaxis);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&R._21), yaxis);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&R._31), zaxis);
			R._14 = R._24 = R._34 = 0.f;
			R._41 = position.x; R._42 = position.y; R._43 = position.z;
			R._44 = 1.f;
			return R;
		}

		Matrix Matrix::CreateFromQuaternion(const Quaternion& rotation)
		{
			using namespace DirectX;
			Matrix R;
			XMVECTOR quatv = rotation;
			XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&R), XMMatrixRotationQuaternion(quatv));
			return R;
		}

		Matrix Matrix::CreateFromYawPitchRoll(float yaw, float pitch, float roll)
		{
			using namespace DirectX;
			Matrix R;
			XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&R), XMMatrixRotationRollPitchYaw(pitch, yaw, roll));
			return R;
		}

		Matrix Matrix::CreateShadow(const float3& lightDir, const Plane& plane)
		{
			using namespace DirectX;
			Matrix R;
			XMVECTOR light = lightDir;
			XMVECTOR planev = plane;
			XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&R), XMMatrixShadow(planev, light));
			return R;
		}

		Matrix Matrix::CreateReflection(const Plane& plane)
		{
			using namespace DirectX;
			Matrix R;
			XMVECTOR planev = plane;
			XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&R), XMMatrixReflect(planev));
			return R;
		}

		void Matrix::Lerp(const Matrix& M1, const Matrix& M2, float t, Matrix& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._11));
			XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._21));
			XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._31));
			XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._41));

			XMVECTOR y1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._11));
			XMVECTOR y2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._21));
			XMVECTOR y3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._31));
			XMVECTOR y4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._41));

			x1 = XMVectorLerp(x1, y1, t);
			x2 = XMVectorLerp(x2, y2, t);
			x3 = XMVectorLerp(x3, y3, t);
			x4 = XMVectorLerp(x4, y4, t);

			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result._11), x1);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result._21), x2);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result._31), x3);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result._41), x4);
		}

		Matrix Matrix::Lerp(const Matrix& M1, const Matrix& M2, float t)
		{
			using namespace DirectX;
			XMVECTOR x1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._11));
			XMVECTOR x2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._21));
			XMVECTOR x3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._31));
			XMVECTOR x4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M1._41));

			XMVECTOR y1 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._11));
			XMVECTOR y2 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._21));
			XMVECTOR y3 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._31));
			XMVECTOR y4 = XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(&M2._41));

			x1 = XMVectorLerp(x1, y1, t);
			x2 = XMVectorLerp(x2, y2, t);
			x3 = XMVectorLerp(x3, y3, t);
			x4 = XMVectorLerp(x4, y4, t);

			Matrix result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result._11), x1);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result._21), x2);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result._31), x3);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result._41), x4);
			return result;
		}

		void Matrix::Transform(const Matrix& M, const Quaternion& rotation, Matrix& result)
		{
			using namespace DirectX;
			XMVECTOR quatv = rotation;

			XMMATRIX M0 = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&M));
			XMMATRIX M1 = XMMatrixRotationQuaternion(quatv);

			XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&result), XMMatrixMultiply(M0, M1));
		}

		Matrix Matrix::Transform(const Matrix& M, const Quaternion& rotation)
		{
			using namespace DirectX;
			XMVECTOR quatv = rotation;

			XMMATRIX M0 = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&M));
			XMMATRIX M1 = XMMatrixRotationQuaternion(quatv);

			Matrix result;
			XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&result), XMMatrixMultiply(M0, M1));
			return result;
		}

		const Matrix Matrix::Identity =
		{
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, 1.f, 0.f,
			0.f, 0.f, 0.f, 1.f
		};

		const Matrix Matrix::ZConversion =
		{
			1.f, 0.f, 0.f, 0.f,
			0.f, 1.f, 0.f, 0.f,
			0.f, 0.f, -1.f, 0.f,
			0.f, 0.f, 0.f, 1.f
		};

		/****************************************************************************
		*
		* Plane
		*
		****************************************************************************/

		Plane::Plane() : x(0.f), y(1.f), z(0.f), w(0.f) {}
		Plane::Plane(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
		Plane::Plane(const float3& normal, float d) : x(normal.x), y(normal.y), z(normal.z), w(d) {}
		Plane::Plane(const float3& point1, const float3& point2, const float3& point3)
		{
			using namespace DirectX;
			XMVECTOR P0 = point1;
			XMVECTOR P1 = point2;
			XMVECTOR P2 = point3;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), XMPlaneFromPoints(P0, P1, P2));
		}
		Plane::Plane(const float3& point, const float3& normal)
		{
			using namespace DirectX;
			XMVECTOR P = point;
			XMVECTOR N = normal;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), XMPlaneFromPointNormal(P, N));
		}
		Plane::Plane(const float4& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
		Plane::Plane(_In_reads_(4) const float *pArray) : x(pArray[0]), y(pArray[1]), z(pArray[2]), w(pArray[3]) {}
		Plane::Plane(const __m128& V) { using namespace DirectX; XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), V); }

		Plane::operator __m128() const
		{
			using namespace DirectX;

			return XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(this));
		}

		//------------------------------------------------------------------------------
		// Comparision operators
		//------------------------------------------------------------------------------

		bool Plane::operator == (const Plane& p) const
		{
			using namespace DirectX;
			XMVECTOR p1 = *this;
			XMVECTOR p2 = p;
			return XMPlaneEqual(p1, p2);
		}

		bool Plane::operator != (const Plane& p) const
		{
			using namespace DirectX;
			XMVECTOR p1 = *this;
			XMVECTOR p2 = p;
			return XMPlaneNotEqual(p1, p2);
		}

		//------------------------------------------------------------------------------
		// Plane operations
		//------------------------------------------------------------------------------

		void Plane::Normalize()
		{
			using namespace DirectX;
			XMVECTOR p = *this;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), XMPlaneNormalize(p));
		}

		void Plane::Normalize(Plane& result) const
		{
			using namespace DirectX;
			XMVECTOR p = *this;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), XMPlaneNormalize(p));
		}

		float Plane::Dot(const float4& v) const
		{
			using namespace DirectX;
			XMVECTOR p = *this;
			XMVECTOR v0 = v;
			return XMVectorGetX(XMPlaneDot(p, v0));
		}

		float Plane::DotCoordinate(const float3& position) const
		{
			using namespace DirectX;
			XMVECTOR p = *this;
			XMVECTOR v0 = position;
			return XMVectorGetX(XMPlaneDotCoord(p, v0));
		}

		float Plane::DotNormal(const float3& normal) const
		{
			using namespace DirectX;
			XMVECTOR p = *this;
			XMVECTOR n0 = normal;
			return XMVectorGetX(XMPlaneDotNormal(p, n0));
		}

		//------------------------------------------------------------------------------
		// Static functions
		//------------------------------------------------------------------------------

		void Plane::Transform(const Plane& plane, const Matrix& M, Plane& result)
		{
			using namespace DirectX;
			XMVECTOR p = plane;
			XMMATRIX m0 = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&M));
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), XMPlaneTransform(p, m0));
		}

		Plane Plane::Transform(const Plane& plane, const Matrix& M)
		{
			using namespace DirectX;
			XMVECTOR p = plane;
			XMMATRIX m0 = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&M));

			Plane result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), XMPlaneTransform(p, m0));
			return result;
		}

		void Plane::Transform(const Plane& plane, const Quaternion& rotation, Plane& result)
		{
			using namespace DirectX;
			XMVECTOR p = plane;
			XMVECTOR q = rotation;
			XMVECTOR X = XMVector3Rotate(p, q);
			X = XMVectorSelect(p, X, g_XMSelect1110); // result.d = plane.d
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
		}

		Plane Plane::Transform(const Plane& plane, const Quaternion& rotation)
		{
			using namespace DirectX;
			XMVECTOR p = plane;
			XMVECTOR q = rotation;
			XMVECTOR X = XMVector3Rotate(p, q);
			X = XMVectorSelect(p, X, g_XMSelect1110); // result.d = plane.d

			Plane result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
			return result;
		}

		/****************************************************************************
		*
		* Quaternion
		*
		****************************************************************************/

		Quaternion::Quaternion() : x(0.f), y(0.f), z(0.f), w(1.f) {}
		Quaternion::Quaternion(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
		Quaternion::Quaternion(const float3& v, float scalar) : x(v.x), y(v.y), z(v.z), w(scalar) {}
		Quaternion::Quaternion(const float4& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
		Quaternion::Quaternion(_In_reads_(4) const float *pArray) : x(pArray[0]), y(pArray[1]), z(pArray[2]), w(pArray[3]) {}
		Quaternion::Quaternion(const __m128& V) { using namespace DirectX; XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), V); }

		Quaternion::operator __m128() const
		{
			using namespace DirectX;

			return XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(this));
		}

		//------------------------------------------------------------------------------
		// Comparision operators
		//------------------------------------------------------------------------------

		bool Quaternion::operator == (const Quaternion& q) const
		{
			using namespace DirectX;
			XMVECTOR q1 = *this;
			XMVECTOR q2 = q;
			return XMQuaternionEqual(q1, q2);
		}

		bool Quaternion::operator != (const Quaternion& q) const
		{
			using namespace DirectX;
			XMVECTOR q1 = *this;
			XMVECTOR q2 = q;
			return XMQuaternionNotEqual(q1, q2);
		}

		//------------------------------------------------------------------------------
		// Assignment operators
		//------------------------------------------------------------------------------

		Quaternion& Quaternion::operator+= (const Quaternion& q)
		{
			using namespace DirectX;
			XMVECTOR q1 = *this;
			XMVECTOR q2 = q;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), XMVectorAdd(q1, q2));
			return *this;
		}

		Quaternion& Quaternion::operator-= (const Quaternion& q)
		{
			using namespace DirectX;
			XMVECTOR q1 = *this;
			XMVECTOR q2 = q;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), XMVectorSubtract(q1, q2));
			return *this;
		}

		Quaternion& Quaternion::operator*= (const Quaternion& q)
		{
			using namespace DirectX;
			XMVECTOR q1 = *this;
			XMVECTOR q2 = q;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), XMQuaternionMultiply(q1, q2));
			return *this;
		}

		Quaternion& Quaternion::operator*= (float S)
		{
			using namespace DirectX;
			XMVECTOR q = *this;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), XMVectorScale(q, S));
			return *this;
		}

		Quaternion& Quaternion::operator/= (const Quaternion& q)
		{
			using namespace DirectX;
			XMVECTOR q1 = *this;
			XMVECTOR q2 = q;
			q2 = XMQuaternionInverse(q2);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), XMQuaternionMultiply(q1, q2));
			return *this;
		}

		//------------------------------------------------------------------------------
		// Urnary operators
		//------------------------------------------------------------------------------

		Quaternion Quaternion::operator- () const
		{
			using namespace DirectX;
			XMVECTOR q = *this;

			Quaternion R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R), XMVectorNegate(q));
			return R;
		}

		//------------------------------------------------------------------------------
		// Binary operators
		//------------------------------------------------------------------------------

		Quaternion operator+ (const Quaternion& Q1, const Quaternion& Q2)
		{
			using namespace DirectX;
			XMVECTOR q1 = Q1;
			XMVECTOR q2 = Q2;

			Quaternion R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R), XMVectorAdd(q1, q2));
			return R;
		}

		Quaternion operator- (const Quaternion& Q1, const Quaternion& Q2)
		{
			using namespace DirectX;
			XMVECTOR q1 = Q1;
			XMVECTOR q2 = Q2;

			Quaternion R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R), XMVectorSubtract(q1, q2));
			return R;
		}

		Quaternion operator* (const Quaternion& Q1, const Quaternion& Q2)
		{
			using namespace DirectX;
			XMVECTOR q1 = Q1;
			XMVECTOR q2 = Q2;

			Quaternion R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R), XMQuaternionMultiply(q1, q2));
			return R;
		}

		Quaternion operator* (const Quaternion& Q, float S)
		{
			using namespace DirectX;
			XMVECTOR q = Q;

			Quaternion R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R), XMVectorScale(q, S));
			return R;
		}

		Quaternion operator/ (const Quaternion& Q, float S)
		{
			using namespace DirectX;
			XMVECTOR v1 = Q;
			XMVECTOR X = XMVectorScale(v1, 1.f / S);

			Quaternion R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R), X);
			return R;
		}

		Quaternion operator/ (const Quaternion& Q1, const Quaternion& Q2)
		{
			using namespace DirectX;
			XMVECTOR q1 = Q1;
			XMVECTOR q2 = Q2;
			q2 = XMQuaternionInverse(q2);

			Quaternion R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R), XMQuaternionMultiply(q1, q2));
			return R;
		}

		Quaternion operator* (float S, const Quaternion& Q)
		{
			using namespace DirectX;
			XMVECTOR q1 = Q;

			Quaternion R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R), XMVectorScale(q1, S));
			return R;
		}

		//------------------------------------------------------------------------------
		// Quaternion operations
		//------------------------------------------------------------------------------

		float Quaternion::Length() const
		{
			using namespace DirectX;
			XMVECTOR q = *this;
			return XMVectorGetX(XMQuaternionLength(q));
		}

		float Quaternion::LengthSquared() const
		{
			using namespace DirectX;
			XMVECTOR q = *this;
			return XMVectorGetX(XMQuaternionLengthSq(q));
		}

		void Quaternion::Normalize()
		{
			using namespace DirectX;
			XMVECTOR q = *this;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), XMQuaternionNormalize(q));
		}

		void Quaternion::Normalize(Quaternion& result) const
		{
			using namespace DirectX;
			XMVECTOR q = *this;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), XMQuaternionNormalize(q));
		}

		void Quaternion::Conjugate()
		{
			using namespace DirectX;
			XMVECTOR q = *this;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), XMQuaternionConjugate(q));
		}

		void Quaternion::Conjugate(Quaternion& result) const
		{
			using namespace DirectX;
			XMVECTOR q = *this;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), XMQuaternionConjugate(q));
		}

		void Quaternion::Inverse(Quaternion& result) const
		{
			using namespace DirectX;
			XMVECTOR q = *this;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), XMQuaternionInverse(q));
		}

		Quaternion Quaternion::Inverse() const
		{
			using namespace DirectX;
			XMVECTOR q = *this;

			Quaternion R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R), XMQuaternionInverse(q));
			return R;
		}

		float Quaternion::Dot(const Quaternion& q) const
		{
			using namespace DirectX;
			XMVECTOR q1 = *this;
			XMVECTOR q2 = q;
			return XMVectorGetX(XMQuaternionDot(q1, q2));
		}

		float3 Quaternion::ToEularRadians() const
		{
			const Matrix rotation = Matrix::CreateFromQuaternion(*this);

			float3 YawPitchRoll;

			const float forwardY = rotation._32;
			if (forwardY <= -1.f)
			{
				YawPitchRoll.x = -PI2;
			}
			else if (forwardY >= 1.f)
			{
				YawPitchRoll.x = PI2;
			}
			else
			{
				YawPitchRoll.x = asin(forwardY);
			}

			if (forwardY > 0.9999f)
			{
				YawPitchRoll.y = 0.f;
				YawPitchRoll.z = atan2f(rotation._13, rotation._11);
			}
			else
			{
				YawPitchRoll.y = atan2f(rotation._31, rotation._33);
				YawPitchRoll.z = atan2f(rotation._12, rotation._22);
			}
			return YawPitchRoll;
		}

		float3 Quaternion::ToEularDegrees() const
		{
			float3 eular = ToEularRadians();
			eular.x = ToDegrees(eular.x);
			eular.y = ToDegrees(eular.y);
			eular.z = ToDegrees(eular.z);
			return eular;
		}

		//------------------------------------------------------------------------------
		// Static functions
		//------------------------------------------------------------------------------

		Quaternion Quaternion::CreateFromAxisAngle(const float3& axis, float angle)
		{
			using namespace DirectX;
			XMVECTOR a = axis;

			Quaternion R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R), XMQuaternionRotationAxis(a, angle));
			return R;
		}

		Quaternion Quaternion::CreateFromYawPitchRoll(float yaw, float pitch, float roll)
		{
			using namespace DirectX;
			Quaternion R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R), XMQuaternionRotationRollPitchYaw(pitch, yaw, roll));
			return R;
		}

		Quaternion Quaternion::CreateFromRotationMatrix(const Matrix& M)
		{
			using namespace DirectX;
			XMMATRIX M0 = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&M));

			Quaternion R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R), XMQuaternionRotationMatrix(M0));
			return R;
		}

		void Quaternion::Lerp(const Quaternion& q1, const Quaternion& q2, float t, Quaternion& result)
		{
			using namespace DirectX;
			XMVECTOR Q0 = q1;
			XMVECTOR Q1 = q2;

			XMVECTOR dot = XMVector4Dot(Q0, Q1);

			XMVECTOR R;
			if (XMVector4GreaterOrEqual(dot, XMVectorZero()))
			{
				R = XMVectorLerp(Q0, Q1, t);
			}
			else
			{
				XMVECTOR tv = XMVectorReplicate(t);
				XMVECTOR t1v = XMVectorReplicate(1.f - t);
				XMVECTOR X0 = XMVectorMultiply(Q0, t1v);
				XMVECTOR X1 = XMVectorMultiply(Q1, tv);
				R = XMVectorSubtract(X0, X1);
			}

			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), XMQuaternionNormalize(R));
		}

		Quaternion Quaternion::Lerp(const Quaternion& q1, const Quaternion& q2, float t)
		{
			using namespace DirectX;
			XMVECTOR Q0 = q1;
			XMVECTOR Q1 = q2;

			XMVECTOR dot = XMVector4Dot(Q0, Q1);

			XMVECTOR R;
			if (XMVector4GreaterOrEqual(dot, XMVectorZero()))
			{
				R = XMVectorLerp(Q0, Q1, t);
			}
			else
			{
				XMVECTOR tv = XMVectorReplicate(t);
				XMVECTOR t1v = XMVectorReplicate(1.f - t);
				XMVECTOR X0 = XMVectorMultiply(Q0, t1v);
				XMVECTOR X1 = XMVectorMultiply(Q1, tv);
				R = XMVectorSubtract(X0, X1);
			}

			Quaternion result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), XMQuaternionNormalize(R));
			return result;
		}

		void Quaternion::Slerp(const Quaternion& q1, const Quaternion& q2, float t, Quaternion& result)
		{
			using namespace DirectX;
			XMVECTOR Q0 = q1;
			XMVECTOR Q1 = q2;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), XMQuaternionSlerp(Q0, Q1, t));
		}

		Quaternion Quaternion::Slerp(const Quaternion& q1, const Quaternion& q2, float t)
		{
			using namespace DirectX;
			XMVECTOR Q0 = q1;
			XMVECTOR Q1 = q2;

			Quaternion result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), XMQuaternionSlerp(Q0, Q1, t));
			return result;
		}

		void Quaternion::Concatenate(const Quaternion& q1, const Quaternion& q2, Quaternion& result)
		{
			using namespace DirectX;
			XMVECTOR Q0 = q1;
			XMVECTOR Q1 = q2;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), XMQuaternionMultiply(Q1, Q0));
		}

		Quaternion Quaternion::Concatenate(const Quaternion& q1, const Quaternion& q2)
		{
			using namespace DirectX;
			XMVECTOR Q0 = q1;
			XMVECTOR Q1 = q2;

			Quaternion result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), XMQuaternionMultiply(Q1, Q0));
			return result;
		}

		const Quaternion Quaternion::Identity = { 0.f, 0.f, 0.f, 1.f };

		Transform::Transform() : scale(float3::One) {}
		Transform::Transform(const math::float3& scale, const math::Quaternion& rotation, const math::float3& position) : scale(scale), rotation(rotation), position(position) {}
		Transform::Transform(const Matrix& matrix)
		{
			matrix.Decompose(scale, rotation, position);
		}

		Matrix Transform::Compose() const
		{
			return Matrix::Compose(scale, rotation, position);
		}

		void Transform::Lerp(const Transform& v1, const Transform& v2, float lerpPerccent, Transform& result)
		{
			math::float3::Lerp(v1.scale, v2.scale, lerpPerccent, result.scale);
			math::Quaternion::Lerp(v1.rotation, v2.rotation, lerpPerccent, result.rotation);
			math::float3::Lerp(v1.position, v2.position, lerpPerccent, result.position);
		}

		RGBA::RGBA() : b(0), g(0), r(0), a(255) {}
		RGBA::RGBA(uint32_t Color) : c(Color) {}
		RGBA::RGBA(float r, float g, float b, float a) { using namespace DirectX; XMStoreColor(reinterpret_cast<PackedVector::XMCOLOR*>(this), XMVectorSet(r, g, b, a)); }
		RGBA::RGBA(_In_reads_(4) const float *pArray) { using namespace DirectX; XMStoreColor(reinterpret_cast<PackedVector::XMCOLOR*>(this), XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(pArray))); }

		/****************************************************************************
		*
		* Color
		*
		****************************************************************************/

		Color::Color() : r(0.f), g(0.f), b(0.f), a(1.f) {}
		Color::Color(float r, float g, float b) : r(r), g(g), b(b), a(1.f) {}
		Color::Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
		Color::Color(const float3& clr) : r(clr.x), g(clr.y), b(clr.z), a(1.f) {}
		Color::Color(const float4& clr) : r(clr.x), g(clr.y), b(clr.z), a(clr.w) {}
		Color::Color(_In_reads_(4) const float *pArray) : r(pArray[0]), g(pArray[1]), b(pArray[2]), a(pArray[3]) {}
		Color::Color(const __m128& V) { using namespace DirectX; XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), V); }

		Color::Color(const RGBA& Packed)
		{
			using namespace DirectX;

			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), PackedVector::XMLoadUByteN4(reinterpret_cast<const PackedVector::XMUBYTEN4*>(&Packed)));
		}

		Color::operator __m128() const
		{
			using namespace DirectX;

			return XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(this));
		}

		//------------------------------------------------------------------------------
		// Comparision operators
		//------------------------------------------------------------------------------
		bool Color::operator == (const Color& c) const
		{
			using namespace DirectX;
			XMVECTOR c1 = *this;
			XMVECTOR c2 = c;
			return XMColorEqual(c1, c2);
		}

		bool Color::operator != (const Color& c) const
		{
			using namespace DirectX;
			XMVECTOR c1 = *this;
			XMVECTOR c2 = c;
			return XMColorNotEqual(c1, c2);
		}

		//------------------------------------------------------------------------------
		// Assignment operators
		//------------------------------------------------------------------------------

		Color& Color::operator= (const RGBA& Packed)
		{
			using namespace DirectX;

			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), XMLoadUByteN4(reinterpret_cast<const PackedVector::XMUBYTEN4*>(&Packed)));
			return *this;
		}

		Color& Color::operator+= (const Color& c)
		{
			using namespace DirectX;
			XMVECTOR c1 = *this;
			XMVECTOR c2 = c;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), XMVectorAdd(c1, c2));
			return *this;
		}

		Color& Color::operator-= (const Color& c)
		{
			using namespace DirectX;
			XMVECTOR c1 = *this;
			XMVECTOR c2 = c;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), XMVectorSubtract(c1, c2));
			return *this;
		}

		Color& Color::operator*= (const Color& c)
		{
			using namespace DirectX;
			XMVECTOR c1 = *this;
			XMVECTOR c2 = c;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), XMVectorMultiply(c1, c2));
			return *this;
		}

		Color& Color::operator*= (float S)
		{
			using namespace DirectX;
			XMVECTOR c = *this;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), XMVectorScale(c, S));
			return *this;
		}

		Color& Color::operator/= (const Color& c)
		{
			using namespace DirectX;
			XMVECTOR c1 = *this;
			XMVECTOR c2 = c;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), XMVectorDivide(c1, c2));
			return *this;
		}

		//------------------------------------------------------------------------------
		// Urnary operators
		//------------------------------------------------------------------------------

		Color Color::operator- () const
		{
			using namespace DirectX;
			XMVECTOR c = *this;
			Color R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R), XMVectorNegate(c));
			return R;
		}

		//------------------------------------------------------------------------------
		// Binary operators
		//------------------------------------------------------------------------------

		Color operator+ (const Color& C1, const Color& C2)
		{
			using namespace DirectX;
			XMVECTOR c1 = C1;
			XMVECTOR c2 = C2;
			Color R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R), XMVectorAdd(c1, c2));
			return R;
		}

		Color operator- (const Color& C1, const Color& C2)
		{
			using namespace DirectX;
			XMVECTOR c1 = C1;
			XMVECTOR c2 = C2;
			Color R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R), XMVectorSubtract(c1, c2));
			return R;
		}

		Color operator* (const Color& C1, const Color& C2)
		{
			using namespace DirectX;
			XMVECTOR c1 = C1;
			XMVECTOR c2 = C2;
			Color R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R), XMVectorMultiply(c1, c2));
			return R;
		}

		Color operator* (const Color& C, float S)
		{
			using namespace DirectX;
			XMVECTOR c = C;
			Color R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R), XMVectorScale(c, S));
			return R;
		}

		Color operator/ (const Color& C1, const Color& C2)
		{
			using namespace DirectX;
			XMVECTOR c1 = C1;
			XMVECTOR c2 = C2;
			Color R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R), XMVectorDivide(c1, c2));
			return R;
		}

		Color operator* (float S, const Color& C)
		{
			using namespace DirectX;
			XMVECTOR c1 = C;
			Color R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R), XMVectorScale(c1, S));
			return R;
		}

		//------------------------------------------------------------------------------
		// Color operations
		//------------------------------------------------------------------------------

		RGBA Color::GetRGBA() const
		{
			using namespace DirectX;

			XMVECTOR clr = *this;
			RGBA Packed;
			PackedVector::XMStoreUByteN4(reinterpret_cast<PackedVector::XMUBYTEN4*>(&Packed), clr);
			return Packed;
		}

		void Color::Negate()
		{
			using namespace DirectX;
			XMVECTOR c = *this;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), XMColorNegative(c));
		}

		void Color::Negate(Color& result) const
		{
			using namespace DirectX;
			XMVECTOR c = *this;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), XMColorNegative(c));
		}

		void Color::Saturate()
		{
			using namespace DirectX;
			XMVECTOR c = *this;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), XMVectorSaturate(c));
		}

		void Color::Saturate(Color& result) const
		{
			using namespace DirectX;
			XMVECTOR c = *this;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), XMVectorSaturate(c));
		}

		void Color::Premultiply()
		{
			using namespace DirectX;
			XMVECTOR c = *this;
			XMVECTOR v = XMVectorSplatW(c);
			v = XMVectorSelect(g_XMIdentityR3, v, g_XMSelect1110);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), XMVectorMultiply(c, v));
		}

		void Color::Premultiply(Color& result) const
		{
			using namespace DirectX;
			XMVECTOR c = *this;
			XMVECTOR v = XMVectorSplatW(c);
			v = XMVectorSelect(g_XMIdentityR3, v, g_XMSelect1110);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), XMVectorMultiply(c, v));
		}

		void Color::AdjustSaturation(float sat)
		{
			using namespace DirectX;
			XMVECTOR c = *this;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), XMColorAdjustSaturation(c, sat));
		}

		void Color::AdjustSaturation(float sat, Color& result) const
		{
			using namespace DirectX;
			XMVECTOR c = *this;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), XMColorAdjustSaturation(c, sat));
		}

		void Color::AdjustContrast(float contrast)
		{
			using namespace DirectX;
			XMVECTOR c = *this;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), XMColorAdjustContrast(c, contrast));
		}

		void Color::AdjustContrast(float contrast, Color& result) const
		{
			using namespace DirectX;
			XMVECTOR c = *this;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), XMColorAdjustContrast(c, contrast));
		}

		//------------------------------------------------------------------------------
		// Static functions
		//------------------------------------------------------------------------------

		void Color::Modulate(const Color& c1, const Color& c2, Color& result)
		{
			using namespace DirectX;
			XMVECTOR C0 = c1;
			XMVECTOR C1 = c2;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), XMColorModulate(C0, C1));
		}

		Color Color::Modulate(const Color& c1, const Color& c2)
		{
			using namespace DirectX;
			XMVECTOR C0 = c1;
			XMVECTOR C1 = c2;

			Color result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), XMColorModulate(C0, C1));
			return result;
		}

		void Color::Lerp(const Color& c1, const Color& c2, float t, Color& result)
		{
			using namespace DirectX;
			XMVECTOR C0 = c1;
			XMVECTOR C1 = c2;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), XMVectorLerp(C0, C1, t));
		}

		Color Color::Lerp(const Color& c1, const Color& c2, float t)
		{
			using namespace DirectX;
			XMVECTOR C0 = c1;
			XMVECTOR C1 = c2;

			Color result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), XMVectorLerp(C0, C1, t));
			return result;
		}

		const Color Color::AliceBlue = { 0.941176534f, 0.972549081f, 1.000000000f, 1.000000000f };
		const Color Color::AntiqueWhite = { 0.980392218f, 0.921568692f, 0.843137324f, 1.000000000f };
		const Color Color::Aqua = { 0.000000000f, 1.000000000f, 1.000000000f, 1.000000000f };
		const Color Color::Aquamarine = { 0.498039246f, 1.000000000f, 0.831372619f, 1.000000000f };
		const Color Color::Azure = { 0.941176534f, 1.000000000f, 1.000000000f, 1.000000000f };
		const Color Color::Beige = { 0.960784376f, 0.960784376f, 0.862745166f, 1.000000000f };
		const Color Color::Bisque = { 1.000000000f, 0.894117713f, 0.768627524f, 1.000000000f };
		const Color Color::Black = { 0.000000000f, 0.000000000f, 0.000000000f, 1.000000000f };
		const Color Color::BlanchedAlmond = { 1.000000000f, 0.921568692f, 0.803921640f, 1.000000000f };
		const Color Color::Blue = { 0.000000000f, 0.000000000f, 1.000000000f, 1.000000000f };
		const Color Color::BlueViolet = { 0.541176498f, 0.168627456f, 0.886274576f, 1.000000000f };
		const Color Color::Brown = { 0.647058845f, 0.164705887f, 0.164705887f, 1.000000000f };
		const Color Color::BurlyWood = { 0.870588303f, 0.721568644f, 0.529411793f, 1.000000000f };
		const Color Color::CadetBlue = { 0.372549027f, 0.619607866f, 0.627451003f, 1.000000000f };
		const Color Color::Chartreuse = { 0.498039246f, 1.000000000f, 0.000000000f, 1.000000000f };
		const Color Color::Chocolate = { 0.823529482f, 0.411764741f, 0.117647067f, 1.000000000f };
		const Color Color::Coral = { 1.000000000f, 0.498039246f, 0.313725501f, 1.000000000f };
		const Color Color::CornflowerBlue = { 0.392156899f, 0.584313750f, 0.929411829f, 1.000000000f };
		const Color Color::Cornsilk = { 1.000000000f, 0.972549081f, 0.862745166f, 1.000000000f };
		const Color Color::Crimson = { 0.862745166f, 0.078431375f, 0.235294133f, 1.000000000f };
		const Color Color::Cyan = { 0.000000000f, 1.000000000f, 1.000000000f, 1.000000000f };
		const Color Color::DarkBlue = { 0.000000000f, 0.000000000f, 0.545098066f, 1.000000000f };
		const Color Color::DarkCyan = { 0.000000000f, 0.545098066f, 0.545098066f, 1.000000000f };
		const Color Color::DarkGoldenrod = { 0.721568644f, 0.525490224f, 0.043137256f, 1.000000000f };
		const Color Color::DarkGray = { 0.662745118f, 0.662745118f, 0.662745118f, 1.000000000f };
		const Color Color::DarkGreen = { 0.000000000f, 0.392156899f, 0.000000000f, 1.000000000f };
		const Color Color::DarkKhaki = { 0.741176486f, 0.717647076f, 0.419607878f, 1.000000000f };
		const Color Color::DarkMagenta = { 0.545098066f, 0.000000000f, 0.545098066f, 1.000000000f };
		const Color Color::DarkOliveGreen = { 0.333333343f, 0.419607878f, 0.184313729f, 1.000000000f };
		const Color Color::DarkOrange = { 1.000000000f, 0.549019635f, 0.000000000f, 1.000000000f };
		const Color Color::DarkOrchid = { 0.600000024f, 0.196078449f, 0.800000072f, 1.000000000f };
		const Color Color::DarkRed = { 0.545098066f, 0.000000000f, 0.000000000f, 1.000000000f };
		const Color Color::DarkSalmon = { 0.913725555f, 0.588235319f, 0.478431404f, 1.000000000f };
		const Color Color::DarkSeaGreen = { 0.560784340f, 0.737254918f, 0.545098066f, 1.000000000f };
		const Color Color::DarkSlateBlue = { 0.282352954f, 0.239215702f, 0.545098066f, 1.000000000f };
		const Color Color::DarkSlateGray = { 0.184313729f, 0.309803933f, 0.309803933f, 1.000000000f };
		const Color Color::DarkTurquoise = { 0.000000000f, 0.807843208f, 0.819607913f, 1.000000000f };
		const Color Color::DarkViolet = { 0.580392182f, 0.000000000f, 0.827451050f, 1.000000000f };
		const Color Color::DeepPink = { 1.000000000f, 0.078431375f, 0.576470613f, 1.000000000f };
		const Color Color::DeepSkyBlue = { 0.000000000f, 0.749019623f, 1.000000000f, 1.000000000f };
		const Color Color::DimGray = { 0.411764741f, 0.411764741f, 0.411764741f, 1.000000000f };
		const Color Color::DodgerBlue = { 0.117647067f, 0.564705908f, 1.000000000f, 1.000000000f };
		const Color Color::Firebrick = { 0.698039234f, 0.133333340f, 0.133333340f, 1.000000000f };
		const Color Color::FloralWhite = { 1.000000000f, 0.980392218f, 0.941176534f, 1.000000000f };
		const Color Color::ForestGreen = { 0.133333340f, 0.545098066f, 0.133333340f, 1.000000000f };
		const Color Color::Fuchsia = { 1.000000000f, 0.000000000f, 1.000000000f, 1.000000000f };
		const Color Color::Gainsboro = { 0.862745166f, 0.862745166f, 0.862745166f, 1.000000000f };
		const Color Color::GhostWhite = { 0.972549081f, 0.972549081f, 1.000000000f, 1.000000000f };
		const Color Color::Gold = { 1.000000000f, 0.843137324f, 0.000000000f, 1.000000000f };
		const Color Color::Goldenrod = { 0.854902029f, 0.647058845f, 0.125490203f, 1.000000000f };
		const Color Color::Gray = { 0.501960814f, 0.501960814f, 0.501960814f, 1.000000000f };
		const Color Color::Green = { 0.000000000f, 0.501960814f, 0.000000000f, 1.000000000f };
		const Color Color::GreenYellow = { 0.678431392f, 1.000000000f, 0.184313729f, 1.000000000f };
		const Color Color::Honeydew = { 0.941176534f, 1.000000000f, 0.941176534f, 1.000000000f };
		const Color Color::HotPink = { 1.000000000f, 0.411764741f, 0.705882370f, 1.000000000f };
		const Color Color::IndianRed = { 0.803921640f, 0.360784322f, 0.360784322f, 1.000000000f };
		const Color Color::Indigo = { 0.294117659f, 0.000000000f, 0.509803951f, 1.000000000f };
		const Color Color::Ivory = { 1.000000000f, 1.000000000f, 0.941176534f, 1.000000000f };
		const Color Color::Khaki = { 0.941176534f, 0.901960850f, 0.549019635f, 1.000000000f };
		const Color Color::Lavender = { 0.901960850f, 0.901960850f, 0.980392218f, 1.000000000f };
		const Color Color::LavenderBlush = { 1.000000000f, 0.941176534f, 0.960784376f, 1.000000000f };
		const Color Color::LawnGreen = { 0.486274540f, 0.988235354f, 0.000000000f, 1.000000000f };
		const Color Color::LemonChiffon = { 1.000000000f, 0.980392218f, 0.803921640f, 1.000000000f };
		const Color Color::LightBlue = { 0.678431392f, 0.847058892f, 0.901960850f, 1.000000000f };
		const Color Color::LightCoral = { 0.941176534f, 0.501960814f, 0.501960814f, 1.000000000f };
		const Color Color::LightCyan = { 0.878431439f, 1.000000000f, 1.000000000f, 1.000000000f };
		const Color Color::LightGoldenrodYellow = { 0.980392218f, 0.980392218f, 0.823529482f, 1.000000000f };
		const Color Color::LightGreen = { 0.564705908f, 0.933333397f, 0.564705908f, 1.000000000f };
		const Color Color::LightGray = { 0.827451050f, 0.827451050f, 0.827451050f, 1.000000000f };
		const Color Color::LightPink = { 1.000000000f, 0.713725507f, 0.756862819f, 1.000000000f };
		const Color Color::LightSalmon = { 1.000000000f, 0.627451003f, 0.478431404f, 1.000000000f };
		const Color Color::LightSeaGreen = { 0.125490203f, 0.698039234f, 0.666666687f, 1.000000000f };
		const Color Color::LightSkyBlue = { 0.529411793f, 0.807843208f, 0.980392218f, 1.000000000f };
		const Color Color::LightSlateGray = { 0.466666698f, 0.533333361f, 0.600000024f, 1.000000000f };
		const Color Color::LightSteelBlue = { 0.690196097f, 0.768627524f, 0.870588303f, 1.000000000f };
		const Color Color::LightYellow = { 1.000000000f, 1.000000000f, 0.878431439f, 1.000000000f };
		const Color Color::Lime = { 0.000000000f, 1.000000000f, 0.000000000f, 1.000000000f };
		const Color Color::LimeGreen = { 0.196078449f, 0.803921640f, 0.196078449f, 1.000000000f };
		const Color Color::Linen = { 0.980392218f, 0.941176534f, 0.901960850f, 1.000000000f };
		const Color Color::Magenta = { 1.000000000f, 0.000000000f, 1.000000000f, 1.000000000f };
		const Color Color::Maroon = { 0.501960814f, 0.000000000f, 0.000000000f, 1.000000000f };
		const Color Color::MediumAquamarine = { 0.400000036f, 0.803921640f, 0.666666687f, 1.000000000f };
		const Color Color::MediumBlue = { 0.000000000f, 0.000000000f, 0.803921640f, 1.000000000f };
		const Color Color::MediumOrchid = { 0.729411781f, 0.333333343f, 0.827451050f, 1.000000000f };
		const Color Color::MediumPurple = { 0.576470613f, 0.439215720f, 0.858823597f, 1.000000000f };
		const Color Color::MediumSeaGreen = { 0.235294133f, 0.701960802f, 0.443137288f, 1.000000000f };
		const Color Color::MediumSlateBlue = { 0.482352972f, 0.407843173f, 0.933333397f, 1.000000000f };
		const Color Color::MediumSpringGreen = { 0.000000000f, 0.980392218f, 0.603921592f, 1.000000000f };
		const Color Color::MediumTurquoise = { 0.282352954f, 0.819607913f, 0.800000072f, 1.000000000f };
		const Color Color::MediumVioletRed = { 0.780392230f, 0.082352944f, 0.521568656f, 1.000000000f };
		const Color Color::MidnightBlue = { 0.098039225f, 0.098039225f, 0.439215720f, 1.000000000f };
		const Color Color::MintCream = { 0.960784376f, 1.000000000f, 0.980392218f, 1.000000000f };
		const Color Color::MistyRose = { 1.000000000f, 0.894117713f, 0.882353008f, 1.000000000f };
		const Color Color::Moccasin = { 1.000000000f, 0.894117713f, 0.709803939f, 1.000000000f };
		const Color Color::NavajoWhite = { 1.000000000f, 0.870588303f, 0.678431392f, 1.000000000f };
		const Color Color::Navy = { 0.000000000f, 0.000000000f, 0.501960814f, 1.000000000f };
		const Color Color::OldLace = { 0.992156923f, 0.960784376f, 0.901960850f, 1.000000000f };
		const Color Color::Olive = { 0.501960814f, 0.501960814f, 0.000000000f, 1.000000000f };
		const Color Color::OliveDrab = { 0.419607878f, 0.556862772f, 0.137254909f, 1.000000000f };
		const Color Color::Orange = { 1.000000000f, 0.647058845f, 0.000000000f, 1.000000000f };
		const Color Color::OrangeRed = { 1.000000000f, 0.270588249f, 0.000000000f, 1.000000000f };
		const Color Color::Orchid = { 0.854902029f, 0.439215720f, 0.839215755f, 1.000000000f };
		const Color Color::PaleGoldenrod = { 0.933333397f, 0.909803987f, 0.666666687f, 1.000000000f };
		const Color Color::PaleGreen = { 0.596078455f, 0.984313786f, 0.596078455f, 1.000000000f };
		const Color Color::PaleTurquoise = { 0.686274529f, 0.933333397f, 0.933333397f, 1.000000000f };
		const Color Color::PaleVioletRed = { 0.858823597f, 0.439215720f, 0.576470613f, 1.000000000f };
		const Color Color::PapayaWhip = { 1.000000000f, 0.937254965f, 0.835294187f, 1.000000000f };
		const Color Color::PeachPuff = { 1.000000000f, 0.854902029f, 0.725490212f, 1.000000000f };
		const Color Color::Peru = { 0.803921640f, 0.521568656f, 0.247058839f, 1.000000000f };
		const Color Color::Pink = { 1.000000000f, 0.752941251f, 0.796078503f, 1.000000000f };
		const Color Color::Plum = { 0.866666734f, 0.627451003f, 0.866666734f, 1.000000000f };
		const Color Color::PowderBlue = { 0.690196097f, 0.878431439f, 0.901960850f, 1.000000000f };
		const Color Color::Purple = { 0.501960814f, 0.000000000f, 0.501960814f, 1.000000000f };
		const Color Color::Red = { 1.000000000f, 0.000000000f, 0.000000000f, 1.000000000f };
		const Color Color::RosyBrown = { 0.737254918f, 0.560784340f, 0.560784340f, 1.000000000f };
		const Color Color::RoyalBlue = { 0.254901975f, 0.411764741f, 0.882353008f, 1.000000000f };
		const Color Color::SaddleBrown = { 0.545098066f, 0.270588249f, 0.074509807f, 1.000000000f };
		const Color Color::Salmon = { 0.980392218f, 0.501960814f, 0.447058856f, 1.000000000f };
		const Color Color::SandyBrown = { 0.956862807f, 0.643137276f, 0.376470625f, 1.000000000f };
		const Color Color::SeaGreen = { 0.180392161f, 0.545098066f, 0.341176480f, 1.000000000f };
		const Color Color::SeaShell = { 1.000000000f, 0.960784376f, 0.933333397f, 1.000000000f };
		const Color Color::Sienna = { 0.627451003f, 0.321568638f, 0.176470593f, 1.000000000f };
		const Color Color::Silver = { 0.752941251f, 0.752941251f, 0.752941251f, 1.000000000f };
		const Color Color::SkyBlue = { 0.529411793f, 0.807843208f, 0.921568692f, 1.000000000f };
		const Color Color::SlateBlue = { 0.415686309f, 0.352941185f, 0.803921640f, 1.000000000f };
		const Color Color::SlateGray = { 0.439215720f, 0.501960814f, 0.564705908f, 1.000000000f };
		const Color Color::Snow = { 1.000000000f, 0.980392218f, 0.980392218f, 1.000000000f };
		const Color Color::SpringGreen = { 0.000000000f, 1.000000000f, 0.498039246f, 1.000000000f };
		const Color Color::SteelBlue = { 0.274509817f, 0.509803951f, 0.705882370f, 1.000000000f };
		const Color Color::Tan = { 0.823529482f, 0.705882370f, 0.549019635f, 1.000000000f };
		const Color Color::Teal = { 0.000000000f, 0.501960814f, 0.501960814f, 1.000000000f };
		const Color Color::Thistle = { 0.847058892f, 0.749019623f, 0.847058892f, 1.000000000f };
		const Color Color::Tomato = { 1.000000000f, 0.388235331f, 0.278431386f, 1.000000000f };
		const Color Color::Transparent = { 0.000000000f, 0.000000000f, 0.000000000f, 0.000000000f };
		const Color Color::Turquoise = { 0.250980407f, 0.878431439f, 0.815686345f, 1.000000000f };
		const Color Color::Violet = { 0.933333397f, 0.509803951f, 0.933333397f, 1.000000000f };
		const Color Color::Wheat = { 0.960784376f, 0.870588303f, 0.701960802f, 1.000000000f };
		const Color Color::White = { 1.000000000f, 1.000000000f, 1.000000000f, 1.000000000f };
		const Color Color::WhiteSmoke = { 0.960784376f, 0.960784376f, 0.960784376f, 1.000000000f };
		const Color Color::Yellow = { 1.000000000f, 1.000000000f, 0.000000000f, 1.000000000f };
		const Color Color::YellowGreen = { 0.603921592f, 0.803921640f, 0.196078449f, 1.000000000f };

		Rect::Rect()
		{
			left = 0;
			top = 0;
			right = 0;
			bottom = 0;
		}
		Rect::Rect(long _left, long _top, long _right, long _bottom)
		{
			left = _left;
			top = _top;
			right = _right;
			bottom = _bottom;
		}
		Rect::Rect(const RECT& o)
			: RECT(o)
		{
		}
		Rect::Rect(long width, long height)
		{
			left = 0;
			top = 0;
			right = width;
			bottom = height;
		}

		/****************************************************************************
		*
		* Viewport
		*
		****************************************************************************/

		Viewport::Viewport() :
			x(0.f), y(0.f), width(0.f), height(0.f), minDepth(0.f), maxDepth(1.f) {}
		Viewport::Viewport(float ix, float iy, float iw, float ih, float iminz, float imaxz) :
			x(ix), y(iy), width(iw), height(ih), minDepth(iminz), maxDepth(imaxz) {}
		Viewport::Viewport(const RECT& rct) :
			x(float(rct.left)), y(float(rct.top)),
			width(float(rct.right - rct.left)),
			height(float(rct.bottom - rct.top)),
			minDepth(0.f), maxDepth(1.f) {}

		//------------------------------------------------------------------------------
		// Viewport operations
		//------------------------------------------------------------------------------

		float3 Viewport::Project(const float3& p, const Matrix& proj, const Matrix& view, const Matrix& world) const
		{
			float3 result;
			Project(p, proj, view, world, result);
			return result;
		}

		void Viewport::Project(const float3& p, const Matrix& proj, const Matrix& view, const Matrix& world, float3& result) const
		{
			using namespace DirectX;
			XMVECTOR v = p;
			XMMATRIX worldMatrix = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&world));
			XMMATRIX viewMatrix = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&view));
			XMMATRIX projMatrix = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&proj));
			v = XMVector3Project(v, x, y, width, height, minDepth, maxDepth, projMatrix, viewMatrix, worldMatrix);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), v);
		}

		float3 Viewport::Unproject(const float3& p, const Matrix& proj, const Matrix& view, const Matrix& world) const
		{
			float3 result;
			Unproject(p, proj, view, world, result);
			return result;
		}

		void Viewport::Unproject(const float3& p, const Matrix& proj, const Matrix& view, const Matrix& world, float3& result) const
		{
			using namespace DirectX;
			XMVECTOR v = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&p));
			XMMATRIX worldMatrix = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&world));
			XMMATRIX viewMatrix = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&view));
			XMMATRIX projMatrix = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&proj));
			v = XMVector3Unproject(v, x, y, width, height, minDepth, maxDepth, projMatrix, viewMatrix, worldMatrix);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), v);
		}

		namespace bezier
		{
			void CreatePatchVertices(_In_reads_(16) float3 patch[16], size_t tessellation, bool isMirrored, std::function<void(const float3&, const float3&, const float3&)> outputVertex)
			{
				using namespace DirectX;

				for (size_t i = 0; i <= tessellation; i++)
				{
					float u = (float)i / tessellation;

					for (size_t j = 0; j <= tessellation; j++)
					{
						float v = (float)j / tessellation;

						// Perform four horizontal bezier interpolations
						// between the control points of this patch.
						XMVECTOR p1 = CubicInterpolate(patch[0], patch[1], patch[2], patch[3], u);
						XMVECTOR p2 = CubicInterpolate(patch[4], patch[5], patch[6], patch[7], u);
						XMVECTOR p3 = CubicInterpolate(patch[8], patch[9], patch[10], patch[11], u);
						XMVECTOR p4 = CubicInterpolate(patch[12], patch[13], patch[14], patch[15], u);

						// Perform a vertical interpolation between the results of the
						// previous horizontal interpolations, to compute the position.
						XMVECTOR position = CubicInterpolate(p1, p2, p3, p4, v);

						// Perform another four bezier interpolations between the control
						// points, but this time vertically rather than horizontally.
						XMVECTOR q1 = CubicInterpolate(patch[0], patch[4], patch[8], patch[12], v);
						XMVECTOR q2 = CubicInterpolate(patch[1], patch[5], patch[9], patch[13], v);
						XMVECTOR q3 = CubicInterpolate(patch[2], patch[6], patch[10], patch[14], v);
						XMVECTOR q4 = CubicInterpolate(patch[3], patch[7], patch[11], patch[15], v);

						// Compute vertical and horizontal tangent vectors.
						XMVECTOR tangent1 = CubicTangent(p1, p2, p3, p4, v);
						XMVECTOR tangent2 = CubicTangent(q1, q2, q3, q4, u);

						// Cross the two tangent vectors to compute the normal.
						XMVECTOR normal = XMVector3Cross(tangent1, tangent2);

						if (!XMVector3NearEqual(normal, XMVectorZero(), g_XMEpsilon))
						{
							normal = XMVector3Normalize(normal);

							// If this patch is mirrored, we must invert the normal.
							if (isMirrored)
							{
								normal = XMVectorNegate(normal);
							}
						}
						else
						{
							// In a tidy and well constructed bezier patch, the preceding
							// normal computation will always work. But the classic teapot
							// model is not tidy or well constructed! At the top and bottom
							// of the teapot, it contains degenerate geometry where a patch
							// has several control points in the same place, which causes
							// the tangent computation to fail and produce a zero normal.
							// We 'fix' these cases by just hard-coding a normal that points
							// either straight up or straight down, depending on whether we
							// are on the top or bottom of the teapot. This is not a robust
							// solution for all possible degenerate bezier patches, but hey,
							// it's good enough to make the teapot work correctly!

							normal = XMVectorSelect(g_XMIdentityR1, g_XMNegIdentityR1, XMVectorLess(position, XMVectorZero()));
						}

						// Compute the texture coordinate.
						float mirroredU = isMirrored ? 1 - u : u;

						XMVECTOR textureCoordinate = XMVectorSet(mirroredU, v, 0, 0);

						// Output this vertex.
						outputVertex(position, normal, textureCoordinate);
					}
				}
			}
		}
	}
}