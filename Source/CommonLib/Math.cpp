#include "stdafx.h"
#include "Math.h"

#include <DirectXMath.h>
#include <DirectXPackedVector.h>

#define STEREOSCALE 1.77777f
#define INVSTEREOSCALE (1.f / STEREOSCALE)

namespace EastEngine
{
	namespace Math
	{
		static std::random_device s_randDevice;
		static std::mt19937 s_rand(s_randDevice());

		std::mt19937& mt19937() { return s_rand; }

		float ToRadians(float fDegrees)
		{
			return DirectX::XMConvertToRadians(fDegrees);
		}

		float ToDegrees(float fRadians)
		{
			return DirectX::XMConvertToDegrees(fRadians);
		}

		bool NearEqual(float S1, float S2, float Epsilon)
		{
			float Delta = S1 - S2;
			return (fabsf(Delta) <= Epsilon);
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

		template <>
		int Random(int min, int max)
		{
			if (min > max)
			{
				int temp = max;
				max = min;
				min = temp;
			}

			std::uniform_int_distribution<int> distribution(min, max);

			return distribution(s_rand);
		}

		template <>
		uint32_t Random(uint32_t min, uint32_t max)
		{
			if (min > max)
			{
				uint32_t temp = max;
				max = min;
				min = temp;
			}

			std::uniform_int_distribution<uint32_t> distribution(min, max);

			return distribution(s_rand);
		}

		template <>
		int64_t Random(int64_t min, int64_t max)
		{
			if (min > max)
			{
				int64_t temp = max;
				max = min;
				min = temp;
			}

			std::uniform_int_distribution<int64_t> distribution(min, max);

			return distribution(s_rand);
		}

		template <>
		uint64_t Random(uint64_t min, uint64_t max)
		{
			if (min > max)
			{
				uint64_t temp = max;
				max = min;
				min = temp;
			}

			std::uniform_int_distribution<uint64_t> distribution(min, max);

			return distribution(s_rand);
		}

		template <>
		float Random(float min, float max)
		{
			if (min > max)
			{
				float temp = max;
				max = min;
				min = temp;
			}

			std::uniform_real_distribution<float> distribution(min, max);

			return distribution(s_rand);
		}

		template <>
		double Random(double min, double max)
		{
			if (min > max)
			{
				double temp = max;
				max = min;
				min = temp;
			}

			std::uniform_real_distribution<double> distribution(min, max);

			return distribution(s_rand);
		}

		Vector2 CompressNormal(const Vector3& f3Normal)
		{
			Vector2 ret = Vector2(f3Normal.x, f3Normal.y) / Vector2(f3Normal.z + 1.f);
			ret *= INVSTEREOSCALE;

			return ret;
		}

		Vector3 DeCompressNormal(const Vector2& f2Normal)
		{
			Vector3 nn;
			nn.x = f2Normal.x * STEREOSCALE;
			nn.y = f2Normal.y * STEREOSCALE;
			nn.z = 1.f;

			float g = 2.f / nn.Dot(nn);

			Vector3 n;
			n.x = g * nn.x;
			n.y = g * nn.y;
			n.z = g - 1.f;

			n.Normalize();

			return n;
		}

		Vector3 CalcTangent(const Vector3& f3Normal)
		{
			Vector3 c1 = f3Normal.Cross(Vector3(0.f, 0.f, 1.f));
			Vector3 c2 = f3Normal.Cross(Vector3(0.f, 1.f, 0.f));

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

		Vector3 CalcBinormal(const Vector3& f3Normal, const Vector3& f3Tangent)
		{
			Vector3 binormal = f3Normal.Cross(f3Tangent);
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

		Int2::Int2() : x(0), y(0) {}
		Int2::Int2(int x) : x(x), y(x) {}
		Int2::Int2(int x, int y) : x(x), y(y) {}
		Int2::Int2(const Int2& V) { this->x = V.x; this->y = V.y; }

		const Int2 Int2::Zero = { 0, 0 };
		const Int2 Int2::One = { 1, 1 };
		const Int2 Int2::UnitX = { 1, 0 };
		const Int2 Int2::UnitY = { 1, 0 };

		Int3::Int3() : x(0), y(0), z(0) {}
		Int3::Int3(int x) : x(x), y(x), z(x) {}
		Int3::Int3(int x, int y, int z) : x(x), y(y), z(z) {}
		Int3::Int3(const Int3& V) { this->x = V.x; this->y = V.y; this->z = V.z; }

		const Int3 Int3::Zero = { 0, 0, 0 };
		const Int3 Int3::One = { 1, 1, 1 };
		const Int3 Int3::UnitX = { 1, 0, 0 };
		const Int3 Int3::UnitY = { 0, 1, 0 };
		const Int3 Int3::UnitZ = { 0, 0, 1 };

		Int4::Int4() : x(0), y(0), z(0), w(0) {}
		Int4::Int4(int x) : x(x), y(x), z(x), w(x) {}
		Int4::Int4(int x, int y, int z, int w) : x(x), y(y), z(z), w(w) {}
		Int4::Int4(const Int4& V) { this->x = V.x; this->y = V.y; this->z = V.z; this->w = V.w; }

		const Int4 Int4::Zero = { 0, 0, 0, 0 };
		const Int4 Int4::One = { 1, 1, 1, 1 };
		const Int4 Int4::UnitX = { 1, 0, 0, 0 };
		const Int4 Int4::UnitY = { 0, 1, 0, 0 };
		const Int4 Int4::UnitZ = { 0, 0, 1, 0 };
		const Int4 Int4::UnitW = { 0, 0, 0, 1 };

		UInt2::UInt2() : x(0), y(0) {}
		UInt2::UInt2(uint32_t x) : x(x), y(x) {}
		UInt2::UInt2(uint32_t x, uint32_t y) : x(x), y(y) {}
		UInt2::UInt2(const UInt2& V) { this->x = V.x; this->y = V.y; }

		const UInt2 UInt2::Zero = { 0, 0 };
		const UInt2 UInt2::One = { 1, 1 };
		const UInt2 UInt2::UnitX = { 1, 0 };
		const UInt2 UInt2::UnitY = { 1, 0 };

		UInt3::UInt3() : x(0), y(0), z(0) {}
		UInt3::UInt3(uint32_t x) : x(x), y(x), z(x) {}
		UInt3::UInt3(uint32_t x, uint32_t y, uint32_t z) : x(x), y(y), z(z) {}
		UInt3::UInt3(const UInt3& V) { this->x = V.x; this->y = V.y; this->z = V.z; }

		const UInt3 UInt3::Zero = { 0, 0, 0 };
		const UInt3 UInt3::One = { 1, 1, 1 };
		const UInt3 UInt3::UnitX = { 1, 0, 0 };
		const UInt3 UInt3::UnitY = { 0, 1, 0 };
		const UInt3 UInt3::UnitZ = { 0, 0, 1 };

		UInt4::UInt4() : x(0), y(0), z(0), w(0) {}
		UInt4::UInt4(uint32_t x) : x(x), y(x), z(x), w(x) {}
		UInt4::UInt4(uint32_t x, uint32_t y, uint32_t z, uint32_t w) : x(x), y(y), z(z), w(w) {}
		UInt4::UInt4(const UInt4& V) { this->x = V.x; this->y = V.y; this->z = V.z; this->w = V.w; }

		const UInt4 UInt4::Zero = { 0, 0, 0, 0 };
		const UInt4 UInt4::One = { 1, 1, 1, 1 };
		const UInt4 UInt4::UnitX = { 1, 0, 0, 0 };
		const UInt4 UInt4::UnitY = { 0, 1, 0, 0 };
		const UInt4 UInt4::UnitZ = { 0, 0, 1, 0 };
		const UInt4 UInt4::UnitW = { 0, 0, 0, 1 };

		/****************************************************************************
		*
		* Vector2
		*
		****************************************************************************/

		Vector2::Vector2() : x(0.f), y(0.f) {}
		Vector2::Vector2(float x) : x(x), y(x) {}
		Vector2::Vector2(float x, float y) : x(x), y(y) {}
		Vector2::Vector2(_In_reads_(2) const float *pArray) : x(pArray[0]), y(pArray[1]) {}
		Vector2::Vector2(const __m128& V) { using namespace DirectX; XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(this), V); }
		Vector2::Vector2(const Vector2& V) { this->x = V.x; this->y = V.y; }

		Vector2::operator __m128() const
		{
			using namespace DirectX;

			return XMLoadFloat2(reinterpret_cast<const XMFLOAT2*>(this));
		}

		//------------------------------------------------------------------------------
		// Comparision operators
		//------------------------------------------------------------------------------

		bool Vector2::operator == (const Vector2& V) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			return XMVector2Equal(v1, v2);
		}

		bool Vector2::operator != (const Vector2& V) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			return XMVector2NotEqual(v1, v2);
		}

		//------------------------------------------------------------------------------
		// Assignment operators
		//------------------------------------------------------------------------------

		Vector2& Vector2::operator+= (const Vector2& V)
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			XMVECTOR X = XMVectorAdd(v1, v2);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(this), X);
			return *this;
		}

		Vector2& Vector2::operator-= (const Vector2& V)
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			XMVECTOR X = XMVectorSubtract(v1, v2);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(this), X);
			return *this;
		}

		Vector2& Vector2::operator*= (const Vector2& V)
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			XMVECTOR X = XMVectorMultiply(v1, v2);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(this), X);
			return *this;
		}

		Vector2& Vector2::operator*= (float S)
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVectorScale(v1, S);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(this), X);
			return *this;
		}

		Vector2& Vector2::operator/= (float S)
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

		Vector2 operator+ (const Vector2& V1, const Vector2& V2)
		{
			using namespace DirectX;
			XMVECTOR v1 = V1;
			XMVECTOR v2 = V2;
			XMVECTOR X = XMVectorAdd(v1, v2);
			Vector2 R;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&R), X);
			return R;
		}

		Vector2 operator- (const Vector2& V1, const Vector2& V2)
		{
			using namespace DirectX;
			XMVECTOR v1 = V1;
			XMVECTOR v2 = V2;
			XMVECTOR X = XMVectorSubtract(v1, v2);
			Vector2 R;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&R), X);
			return R;
		}

		Vector2 operator* (const Vector2& V1, const Vector2& V2)
		{
			using namespace DirectX;
			XMVECTOR v1 = V1;
			XMVECTOR v2 = V2;
			XMVECTOR X = XMVectorMultiply(v1, v2);
			Vector2 R;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&R), X);
			return R;
		}

		Vector2 operator* (const Vector2& V, float S)
		{
			using namespace DirectX;
			XMVECTOR v1 = V;
			XMVECTOR X = XMVectorScale(v1, S);
			Vector2 R;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&R), X);
			return R;
		}

		Vector2 operator/ (const Vector2& V1, const Vector2& V2)
		{
			using namespace DirectX;
			XMVECTOR v1 = V1;
			XMVECTOR v2 = V2;
			XMVECTOR X = XMVectorDivide(v1, v2);
			Vector2 R;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&R), X);
			return R;
		}

		Vector2 operator/ (const Vector2& V1, float S)
		{
			using namespace DirectX;
			XMVECTOR v1 = V1;
			XMVECTOR X = XMVectorScale(v1, 1.f / S);
			Vector2 R;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&R), X);
			return R;
		}

		Vector2 operator* (float S, const Vector2& V)
		{
			using namespace DirectX;
			XMVECTOR v1 = V;
			XMVECTOR X = XMVectorScale(v1, S);
			Vector2 R;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&R), X);
			return R;
		}

		//------------------------------------------------------------------------------
		// Vector operations
		//------------------------------------------------------------------------------

		bool Vector2::InBounds(const Vector2& Bounds) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = Bounds;
			return XMVector2InBounds(v1, v2);
		}

		float Vector2::Length() const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVector2Length(v1);
			return XMVectorGetX(X);
		}

		float Vector2::LengthSquared() const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVector2LengthSq(v1);
			return XMVectorGetX(X);
		}

		float Vector2::Dot(const Vector2& V) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			XMVECTOR X = XMVector2Dot(v1, v2);
			return XMVectorGetX(X);
		}

		void Vector2::Cross(const Vector2& V, Vector2& result) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			XMVECTOR R = XMVector2Cross(v1, v2);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), R);
		}

		Vector2 Vector2::Cross(const Vector2& V) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			XMVECTOR R = XMVector2Cross(v1, v2);

			Vector2 result;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), R);
			return result;
		}

		void Vector2::Normalize()
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVector2Normalize(v1);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(this), X);
		}

		void Vector2::Normalize(Vector2& result) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVector2Normalize(v1);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
		}

		void Vector2::Clamp(const Vector2& vmin, const Vector2& vmax)
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = vmin;
			XMVECTOR v3 = vmax;
			XMVECTOR X = XMVectorClamp(v1, v2, v3);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(this), X);
		}

		void Vector2::Clamp(const Vector2& vmin, const Vector2& vmax, Vector2& result) const
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

		float Vector2::Distance(const Vector2& v1, const Vector2& v2)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR V = XMVectorSubtract(x2, x1);
			XMVECTOR X = XMVector2Length(V);
			return XMVectorGetX(X);
		}

		float Vector2::DistanceSquared(const Vector2& v1, const Vector2& v2)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR V = XMVectorSubtract(x2, x1);
			XMVECTOR X = XMVector2LengthSq(V);
			return XMVectorGetX(X);
		}

		void Vector2::Min(const Vector2& v1, const Vector2& v2, Vector2& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorMin(x1, x2);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
		}

		Vector2 Vector2::Min(const Vector2& v1, const Vector2& v2)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorMin(x1, x2);

			Vector2 result;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
			return result;
		}

		void Vector2::Max(const Vector2& v1, const Vector2& v2, Vector2& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorMax(x1, x2);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
		}

		Vector2 Vector2::Max(const Vector2& v1, const Vector2& v2)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorMax(x1, x2);

			Vector2 result;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
			return result;
		}

		void Vector2::Lerp(const Vector2& v1, const Vector2& v2, float t, Vector2& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorLerp(x1, x2, t);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
		}

		Vector2 Vector2::Lerp(const Vector2& v1, const Vector2& v2, float t)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorLerp(x1, x2, t);

			Vector2 result;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
			return result;
		}

		void Vector2::SmoothStep(const Vector2& v1, const Vector2& v2, float t, Vector2& result)
		{
			using namespace DirectX;
			t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t);  // Clamp value to 0 to 1
			t = t*t*(3.f - 2.f*t);
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorLerp(x1, x2, t);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
		}

		Vector2 Vector2::SmoothStep(const Vector2& v1, const Vector2& v2, float t)
		{
			using namespace DirectX;
			t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t);  // Clamp value to 0 to 1
			t = t*t*(3.f - 2.f*t);
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorLerp(x1, x2, t);

			Vector2 result;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
			return result;
		}

		void Vector2::Barycentric(const Vector2& v1, const Vector2& v2, const Vector2& v3, float f, float g, Vector2& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR x3 = v3;
			XMVECTOR X = XMVectorBaryCentric(x1, x2, x3, f, g);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
		}

		Vector2 Vector2::Barycentric(const Vector2& v1, const Vector2& v2, const Vector2& v3, float f, float g)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR x3 = v3;
			XMVECTOR X = XMVectorBaryCentric(x1, x2, x3, f, g);

			Vector2 result;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
			return result;
		}

		void Vector2::CatmullRom(const Vector2& v1, const Vector2& v2, const Vector2& v3, const Vector2& v4, float t, Vector2& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR x3 = v3;
			XMVECTOR x4 = v4;
			XMVECTOR X = XMVectorCatmullRom(x1, x2, x3, x4, t);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
		}

		Vector2 Vector2::CatmullRom(const Vector2& v1, const Vector2& v2, const Vector2& v3, const Vector2& v4, float t)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR x3 = v3;
			XMVECTOR x4 = v4;
			XMVECTOR X = XMVectorCatmullRom(x1, x2, x3, x4, t);

			Vector2 result;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
			return result;
		}

		void Vector2::Hermite(const Vector2& v1, const Vector2& t1, const Vector2& v2, const Vector2& t2, float t, Vector2& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = t1;
			XMVECTOR x3 = v2;
			XMVECTOR x4 = t2;
			XMVECTOR X = XMVectorHermite(x1, x2, x3, x4, t);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
		}

		Vector2 Vector2::Hermite(const Vector2& v1, const Vector2& t1, const Vector2& v2, const Vector2& t2, float t)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = t1;
			XMVECTOR x3 = v2;
			XMVECTOR x4 = t2;
			XMVECTOR X = XMVectorHermite(x1, x2, x3, x4, t);

			Vector2 result;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
			return result;
		}

		void Vector2::Reflect(const Vector2& ivec, const Vector2& nvec, Vector2& result)
		{
			using namespace DirectX;
			XMVECTOR i = ivec;
			XMVECTOR n = nvec;
			XMVECTOR X = XMVector2Reflect(i, n);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
		}

		Vector2 Vector2::Reflect(const Vector2& ivec, const Vector2& nvec)
		{
			using namespace DirectX;
			XMVECTOR i = ivec;
			XMVECTOR n = nvec;
			XMVECTOR X = XMVector2Reflect(i, n);

			Vector2 result;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
			return result;
		}

		void Vector2::Refract(const Vector2& ivec, const Vector2& nvec, float refractionIndex, Vector2& result)
		{
			using namespace DirectX;
			XMVECTOR i = ivec;
			XMVECTOR n = nvec;
			XMVECTOR X = XMVector2Refract(i, n, refractionIndex);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
		}

		Vector2 Vector2::Refract(const Vector2& ivec, const Vector2& nvec, float refractionIndex)
		{
			using namespace DirectX;
			XMVECTOR i = ivec;
			XMVECTOR n = nvec;
			XMVECTOR X = XMVector2Refract(i, n, refractionIndex);

			Vector2 result;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
			return result;
		}

		void Vector2::Transform(const Vector2& v, const Quaternion& quat, Vector2& result)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMVECTOR q = quat;
			XMVECTOR X = XMVector3Rotate(v1, q);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
		}

		Vector2 Vector2::Transform(const Vector2& v, const Quaternion& quat)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMVECTOR q = quat;
			XMVECTOR X = XMVector3Rotate(v1, q);

			Vector2 result;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
			return result;
		}

		void Vector2::Transform(const Vector2& v, const Matrix& m, Vector2& result)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVECTOR X = XMVector2TransformCoord(v1, M);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
		}

		Vector2 Vector2::Transform(const Vector2& v, const Matrix& m)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVECTOR X = XMVector2TransformCoord(v1, M);

			Vector2 result;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
			return result;
		}

		_Use_decl_annotations_
			void Vector2::Transform(const Vector2* varray, size_t count, const Matrix& m, Vector2* resultArray)
		{
			using namespace DirectX;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVector2TransformCoordStream(reinterpret_cast<XMFLOAT2*>(resultArray), sizeof(XMFLOAT2), reinterpret_cast<const XMFLOAT2*>(varray), sizeof(XMFLOAT2), count, M);
		}

		void Vector2::Transform(const Vector2& v, const Matrix& m, Vector4& result)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVECTOR X = XMVector2Transform(v1, M);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
		}

		_Use_decl_annotations_
			void Vector2::Transform(const Vector2* varray, size_t count, const Matrix& m, Vector4* resultArray)
		{
			using namespace DirectX;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVector2TransformStream(reinterpret_cast<XMFLOAT4*>(resultArray), sizeof(XMFLOAT4), reinterpret_cast<const XMFLOAT2*>(varray), sizeof(XMFLOAT2), count, M);
		}

		void Vector2::TransformNormal(const Vector2& v, const Matrix& m, Vector2& result)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVECTOR X = XMVector2TransformNormal(v1, M);
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
		}

		Vector2 Vector2::TransformNormal(const Vector2& v, const Matrix& m)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVECTOR X = XMVector2TransformNormal(v1, M);

			Vector2 result;
			XMStoreFloat2(reinterpret_cast<XMFLOAT2*>(&result), X);
			return result;
		}

		_Use_decl_annotations_
			void Vector2::TransformNormal(const Vector2* varray, size_t count, const Matrix& m, Vector2* resultArray)
		{
			using namespace DirectX;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVector2TransformNormalStream(reinterpret_cast<XMFLOAT2*>(resultArray), sizeof(XMFLOAT2), reinterpret_cast<const XMFLOAT2*>(varray), sizeof(XMFLOAT2), count, M);
		}

		const Vector2 Vector2::Zero = { 0.f, 0.f };
		const Vector2 Vector2::One = { 1.f, 1.f };
		const Vector2 Vector2::UnitX = { 1.f, 0.f };
		const Vector2 Vector2::UnitY = { 0.f, 1.f };

		/****************************************************************************
		*
		* Vector3
		*
		****************************************************************************/

		Vector3::Vector3() : x(0.f), y(0.f), z(0.f) {}
		Vector3::Vector3(float x) : x(x), y(x), z(x) {}
		Vector3::Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
		Vector3::Vector3(_In_reads_(3) const float *pArray) : x(pArray[0]), y(pArray[1]), z(pArray[2]) {}
		Vector3::Vector3(const __m128& V) { using namespace DirectX; XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(this), V); }
		Vector3::Vector3(const Vector3& V) { this->x = V.x; this->y = V.y; this->z = V.z; }

		Vector3::operator __m128() const
		{
			using namespace DirectX;

			return XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(this));
		}

		//------------------------------------------------------------------------------
		// Comparision operators
		//------------------------------------------------------------------------------

		bool Vector3::operator == (const Vector3& V) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			return XMVector3Equal(v1, v2);
		}

		bool Vector3::operator != (const Vector3& V) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			return XMVector3NotEqual(v1, v2);
		}

		//------------------------------------------------------------------------------
		// Assignment operators
		//------------------------------------------------------------------------------

		Vector3& Vector3::operator+= (const Vector3& V)
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			XMVECTOR X = XMVectorAdd(v1, v2);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(this), X);
			return *this;
		}

		Vector3& Vector3::operator-= (const Vector3& V)
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			XMVECTOR X = XMVectorSubtract(v1, v2);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(this), X);
			return *this;
		}

		Vector3& Vector3::operator*= (const Vector3& V)
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			XMVECTOR X = XMVectorMultiply(v1, v2);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(this), X);
			return *this;
		}

		Vector3& Vector3::operator/= (const Vector3& V)
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			XMVECTOR X = XMVectorDivide(v1, v2);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(this), X);
			return *this;
		}

		Vector3& Vector3::operator*= (float S)
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVectorScale(v1, S);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(this), X);
			return *this;
		}

		Vector3& Vector3::operator/= (float S)
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

		Vector3 Vector3::operator- () const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVectorNegate(v1);
			Vector3 R;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&R), X);
			return R;
		}

		//------------------------------------------------------------------------------
		// Binary operators
		//------------------------------------------------------------------------------

		Vector3 operator+ (const Vector3& V1, const Vector3& V2)
		{
			using namespace DirectX;
			XMVECTOR v1 = V1;
			XMVECTOR v2 = V2;
			XMVECTOR X = XMVectorAdd(v1, v2);
			Vector3 R;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&R), X);
			return R;
		}

		Vector3 operator- (const Vector3& V1, const Vector3& V2)
		{
			using namespace DirectX;
			XMVECTOR v1 = V1;
			XMVECTOR v2 = V2;
			XMVECTOR X = XMVectorSubtract(v1, v2);
			Vector3 R;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&R), X);
			return R;
		}

		Vector3 operator* (const Vector3& V1, const Vector3& V2)
		{
			using namespace DirectX;
			XMVECTOR v1 = V1;
			XMVECTOR v2 = V2;
			XMVECTOR X = XMVectorMultiply(v1, v2);
			Vector3 R;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&R), X);
			return R;
		}

		Vector3 operator* (const Vector3& V, float S)
		{
			using namespace DirectX;
			XMVECTOR v1 = V;
			XMVECTOR X = XMVectorScale(v1, S);
			Vector3 R;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&R), X);
			return R;
		}

		Vector3 operator/ (const Vector3& V1, const Vector3& V2)
		{
			using namespace DirectX;
			XMVECTOR v1 = V1;
			XMVECTOR v2 = V2;
			XMVECTOR X = XMVectorDivide(v1, v2);
			Vector3 R;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&R), X);
			return R;
		}

		Vector3 operator/ (const Vector3& V, float S)
		{
			using namespace DirectX;
			XMVECTOR v1 = V;
			XMVECTOR X = XMVectorScale(v1, 1.f / S);
			Vector3 R;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&R), X);
			return R;
		}

		Vector3 operator* (float S, const Vector3& V)
		{
			using namespace DirectX;
			XMVECTOR v1 = V;
			XMVECTOR X = XMVectorScale(v1, S);
			Vector3 R;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&R), X);
			return R;
		}

		//------------------------------------------------------------------------------
		// Vector operations
		//------------------------------------------------------------------------------

		bool Vector3::InBounds(const Vector3& Bounds) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = Bounds;
			return XMVector3InBounds(v1, v2);
		}

		float Vector3::Length() const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVector3Length(v1);
			return XMVectorGetX(X);
		}

		float Vector3::LengthSquared() const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVector3LengthSq(v1);
			return XMVectorGetX(X);
		}

		float Vector3::Dot(const Vector3& V) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			XMVECTOR X = XMVector3Dot(v1, v2);
			return XMVectorGetX(X);
		}

		void Vector3::Cross(const Vector3& V, Vector3& result) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			XMVECTOR R = XMVector3Cross(v1, v2);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), R);
		}

		Vector3 Vector3::Cross(const Vector3& V) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			XMVECTOR R = XMVector3Cross(v1, v2);

			Vector3 result;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), R);
			return result;
		}

		void Vector3::Normalize()
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVector3Normalize(v1);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(this), X);
		}

		void Vector3::Normalize(Vector3& result) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVector3Normalize(v1);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
		}

		void Vector3::Clamp(const Vector3& vmin, const Vector3& vmax)
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = vmin;
			XMVECTOR v3 = vmax;
			XMVECTOR X = XMVectorClamp(v1, v2, v3);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(this), X);
		}

		void Vector3::Clamp(const Vector3& vmin, const Vector3& vmax, Vector3& result) const
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

		float Vector3::Distance(const Vector3& v1, const Vector3& v2)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR V = XMVectorSubtract(x2, x1);
			XMVECTOR X = XMVector3Length(V);
			return XMVectorGetX(X);
		}

		float Vector3::DistanceSquared(const Vector3& v1, const Vector3& v2)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR V = XMVectorSubtract(x2, x1);
			XMVECTOR X = XMVector3LengthSq(V);
			return XMVectorGetX(X);
		}

		void Vector3::Min(const Vector3& v1, const Vector3& v2, Vector3& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorMin(x1, x2);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
		}

		Vector3 Vector3::Min(const Vector3& v1, const Vector3& v2)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorMin(x1, x2);

			Vector3 result;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
			return result;
		}

		void Vector3::Max(const Vector3& v1, const Vector3& v2, Vector3& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorMax(x1, x2);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
		}

		Vector3 Vector3::Max(const Vector3& v1, const Vector3& v2)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorMax(x1, x2);

			Vector3 result;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
			return result;
		}

		void Vector3::Lerp(const Vector3& v1, const Vector3& v2, float t, Vector3& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorLerp(x1, x2, t);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
		}

		Vector3 Vector3::Lerp(const Vector3& v1, const Vector3& v2, float t)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorLerp(x1, x2, t);

			Vector3 result;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
			return result;
		}

		void Vector3::SmoothStep(const Vector3& v1, const Vector3& v2, float t, Vector3& result)
		{
			using namespace DirectX;
			t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t);  // Clamp value to 0 to 1
			t = t*t*(3.f - 2.f*t);
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorLerp(x1, x2, t);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
		}

		Vector3 Vector3::SmoothStep(const Vector3& v1, const Vector3& v2, float t)
		{
			using namespace DirectX;
			t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t);  // Clamp value to 0 to 1
			t = t*t*(3.f - 2.f*t);
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorLerp(x1, x2, t);

			Vector3 result;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
			return result;
		}

		void Vector3::Barycentric(const Vector3& v1, const Vector3& v2, const Vector3& v3, float f, float g, Vector3& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR x3 = v3;;
			XMVECTOR X = XMVectorBaryCentric(x1, x2, x3, f, g);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
		}

		Vector3 Vector3::Barycentric(const Vector3& v1, const Vector3& v2, const Vector3& v3, float f, float g)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR x3 = v3;;
			XMVECTOR X = XMVectorBaryCentric(x1, x2, x3, f, g);

			Vector3 result;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
			return result;
		}

		void Vector3::CatmullRom(const Vector3& v1, const Vector3& v2, const Vector3& v3, const Vector3& v4, float t, Vector3& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR x3 = v3;;
			XMVECTOR x4 = v4;;
			XMVECTOR X = XMVectorCatmullRom(x1, x2, x3, x4, t);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
		}

		Vector3 Vector3::CatmullRom(const Vector3& v1, const Vector3& v2, const Vector3& v3, const Vector3& v4, float t)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR x3 = v3;;
			XMVECTOR x4 = v4;;
			XMVECTOR X = XMVectorCatmullRom(x1, x2, x3, x4, t);

			Vector3 result;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
			return result;
		}

		void Vector3::Hermite(const Vector3& v1, const Vector3& t1, const Vector3& v2, const Vector3& t2, float t, Vector3& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = t1;
			XMVECTOR x3 = v2;
			XMVECTOR x4 = t2;
			XMVECTOR X = XMVectorHermite(x1, x2, x3, x4, t);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
		}

		Vector3 Vector3::Hermite(const Vector3& v1, const Vector3& t1, const Vector3& v2, const Vector3& t2, float t)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = t1;
			XMVECTOR x3 = v2;
			XMVECTOR x4 = t2;
			XMVECTOR X = XMVectorHermite(x1, x2, x3, x4, t);

			Vector3 result;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
			return result;
		}

		void Vector3::Reflect(const Vector3& ivec, const Vector3& nvec, Vector3& result)
		{
			using namespace DirectX;
			XMVECTOR i = ivec;
			XMVECTOR n = nvec;
			XMVECTOR X = XMVector3Reflect(i, n);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
		}

		Vector3 Vector3::Reflect(const Vector3& ivec, const Vector3& nvec)
		{
			using namespace DirectX;
			XMVECTOR i = ivec;
			XMVECTOR n = nvec;
			XMVECTOR X = XMVector3Reflect(i, n);

			Vector3 result;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
			return result;
		}

		void Vector3::Refract(const Vector3& ivec, const Vector3& nvec, float refractionIndex, Vector3& result)
		{
			using namespace DirectX;
			XMVECTOR i = ivec;
			XMVECTOR n = nvec;
			XMVECTOR X = XMVector3Refract(i, n, refractionIndex);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
		}

		Vector3 Vector3::Refract(const Vector3& ivec, const Vector3& nvec, float refractionIndex)
		{
			using namespace DirectX;
			XMVECTOR i = ivec;
			XMVECTOR n = nvec;
			XMVECTOR X = XMVector3Refract(i, n, refractionIndex);

			Vector3 result;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
			return result;
		}

		void Vector3::Transform(const Vector3& v, const Quaternion& quat, Vector3& result)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMVECTOR q = quat;
			XMVECTOR X = XMVector3Rotate(v1, q);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
		}

		Vector3 Vector3::Transform(const Vector3& v, const Quaternion& quat)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMVECTOR q = quat;
			XMVECTOR X = XMVector3Rotate(v1, q);

			Vector3 result;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
			return result;
		}

		void Vector3::Transform(const Vector3& v, const Matrix& m, Vector3& result)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVECTOR X = XMVector3TransformCoord(v1, M);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
		}

		Vector3 Vector3::Transform(const Vector3& v, const Matrix& m)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVECTOR X = XMVector3TransformCoord(v1, M);

			Vector3 result;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
			return result;
		}

		_Use_decl_annotations_
			void Vector3::Transform(const Vector3* varray, size_t count, const Matrix& m, Vector3* resultArray)
		{
			using namespace DirectX;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVector3TransformCoordStream(reinterpret_cast<XMFLOAT3*>(resultArray), sizeof(XMFLOAT3), reinterpret_cast<const XMFLOAT3*>(varray), sizeof(XMFLOAT3), count, M);
		}

		void Vector3::Transform(const Vector3& v, const Matrix& m, Vector4& result)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVECTOR X = XMVector3Transform(v1, M);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
		}

		_Use_decl_annotations_
			void Vector3::Transform(const Vector3* varray, size_t count, const Matrix& m, Vector4* resultArray)
		{
			using namespace DirectX;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVector3TransformStream(reinterpret_cast<XMFLOAT4*>(resultArray), sizeof(XMFLOAT4), reinterpret_cast<const XMFLOAT3*>(varray), sizeof(XMFLOAT3), count, M);
		}

		void Vector3::TransformNormal(const Vector3& v, const Matrix& m, Vector3& result)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVECTOR X = XMVector3TransformNormal(v1, M);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
		}

		Vector3 Vector3::TransformNormal(const Vector3& v, const Matrix& m)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVECTOR X = XMVector3TransformNormal(v1, M);

			Vector3 result;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), X);
			return result;
		}

		_Use_decl_annotations_
			void Vector3::TransformNormal(const Vector3* varray, size_t count, const Matrix& m, Vector3* resultArray)
		{
			using namespace DirectX;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVector3TransformNormalStream(reinterpret_cast<XMFLOAT3*>(resultArray), sizeof(XMFLOAT3), reinterpret_cast<const XMFLOAT3*>(varray), sizeof(XMFLOAT3), count, M);
		}

		Vector3 FresnelTerm(const Vector3& v1, const Vector3& v2)
		{
			using namespace DirectX;
			XMVECTOR V1 = v1;
			XMVECTOR V2 = v2;

			Vector3 result;
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), XMFresnelTerm(V1, V2));

			return result;
		}

		const Vector3 Vector3::Zero = { 0.f, 0.f, 0.f };
		const Vector3 Vector3::One = { 1.f, 1.f, 1.f };
		const Vector3 Vector3::UnitX = { 1.f, 0.f, 0.f };
		const Vector3 Vector3::UnitY = { 0.f, 1.f, 0.f };
		const Vector3 Vector3::UnitZ = { 0.f, 0.f, 1.f };
		const Vector3 Vector3::Up = { 0.f, 1.f, 0.f };
		const Vector3 Vector3::Down = { 0.f, -1.f, 0.f };
		const Vector3 Vector3::Right = { 1.f, 0.f, 0.f };
		const Vector3 Vector3::Left = { -1.f, 0.f, 0.f };
		const Vector3 Vector3::Forward = { 0.f, 0.f, 1.f };
		const Vector3 Vector3::Backward = { 0.f, 0.f, -1.f };

		/****************************************************************************
		*
		* Vector4
		*
		****************************************************************************/

		Vector4::Vector4() : x(0.f), y(0.f), z(0.f), w(0.f) {}
		Vector4::Vector4(float x) : x(x), y(x), z(x), w(x) {}
		Vector4::Vector4(float x, float y, float z, float w) : x(x), y(y), z(z), w(w) {}
		Vector4::Vector4(_In_reads_(4) const float *pArray) : x(pArray[0]), y(pArray[1]), z(pArray[2]), w(pArray[3]) {}
		Vector4::Vector4(const __m128& V) { using namespace DirectX; XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), V); }
		Vector4::Vector4(const Vector4& V) { this->x = V.x; this->y = V.y; this->z = V.z; this->w = V.w; }

		Vector4::operator __m128() const
		{
			using namespace DirectX;

			return XMLoadFloat4(reinterpret_cast<const XMFLOAT4*>(this));
		}

		//------------------------------------------------------------------------------
		// Comparision operators
		//------------------------------------------------------------------------------

		bool Vector4::operator == (const Vector4& V) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			return XMVector4Equal(v1, v2);
		}

		bool Vector4::operator != (const Vector4& V) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			return XMVector4NotEqual(v1, v2);
		}

		//------------------------------------------------------------------------------
		// Assignment operators
		//------------------------------------------------------------------------------

		Vector4& Vector4::operator+= (const Vector4& V)
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			XMVECTOR X = XMVectorAdd(v1, v2);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), X);
			return *this;
		}

		Vector4& Vector4::operator-= (const Vector4& V)
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			XMVECTOR X = XMVectorSubtract(v1, v2);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), X);
			return *this;
		}

		Vector4& Vector4::operator*= (const Vector4& V)
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			XMVECTOR X = XMVectorMultiply(v1, v2);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), X);
			return *this;
		}

		Vector4& Vector4::operator*= (float S)
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVectorScale(v1, S);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), X);
			return *this;
		}

		Vector4& Vector4::operator/= (float S)
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

		Vector4 Vector4::operator- () const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVectorNegate(v1);
			Vector4 R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R), X);
			return R;
		}

		//------------------------------------------------------------------------------
		// Binary operators
		//------------------------------------------------------------------------------

		Vector4 operator+ (const Vector4& V1, const Vector4& V2)
		{
			using namespace DirectX;
			XMVECTOR v1 = V1;
			XMVECTOR v2 = V2;
			XMVECTOR X = XMVectorAdd(v1, v2);
			Vector4 R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R), X);
			return R;
		}

		Vector4 operator- (const Vector4& V1, const Vector4& V2)
		{
			using namespace DirectX;
			XMVECTOR v1 = V1;
			XMVECTOR v2 = V2;
			XMVECTOR X = XMVectorSubtract(v1, v2);
			Vector4 R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R), X);
			return R;
		}

		Vector4 operator* (const Vector4& V1, const Vector4& V2)
		{
			using namespace DirectX;
			XMVECTOR v1 = V1;
			XMVECTOR v2 = V2;
			XMVECTOR X = XMVectorMultiply(v1, v2);
			Vector4 R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R), X);
			return R;
		}

		Vector4 operator* (const Vector4& V, float S)
		{
			using namespace DirectX;
			XMVECTOR v1 = V;
			XMVECTOR X = XMVectorScale(v1, S);
			Vector4 R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R), X);
			return R;
		}

		Vector4 operator/ (const Vector4& V1, const Vector4& V2)
		{
			using namespace DirectX;
			XMVECTOR v1 = V1;
			XMVECTOR v2 = V2;
			XMVECTOR X = XMVectorDivide(v1, v2);
			Vector4 R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R), X);
			return R;
		}

		Vector4 operator* (float S, const Vector4& V)
		{
			using namespace DirectX;
			XMVECTOR v1 = V;
			XMVECTOR X = XMVectorScale(v1, S);
			Vector4 R;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&R), X);
			return R;
		}

		//------------------------------------------------------------------------------
		// Vector operations
		//------------------------------------------------------------------------------

		bool Vector4::InBounds(const Vector4& Bounds) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = Bounds;
			return XMVector4InBounds(v1, v2);
		}

		float Vector4::Length() const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVector4Length(v1);
			return XMVectorGetX(X);
		}

		float Vector4::LengthSquared() const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVector4LengthSq(v1);
			return XMVectorGetX(X);
		}

		float Vector4::Dot(const Vector4& V) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = V;
			XMVECTOR X = XMVector4Dot(v1, v2);
			return XMVectorGetX(X);
		}

		void Vector4::Cross(const Vector4& v1, const Vector4& v2, Vector4& result) const
		{
			using namespace DirectX;
			XMVECTOR x1 = *this;
			XMVECTOR x2 = v1;
			XMVECTOR x3 = v2;
			XMVECTOR R = XMVector4Cross(x1, x2, x3);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), R);
		}

		Vector4 Vector4::Cross(const Vector4& v1, const Vector4& v2) const
		{
			using namespace DirectX;
			XMVECTOR x1 = *this;
			XMVECTOR x2 = v1;
			XMVECTOR x3 = v2;
			XMVECTOR R = XMVector4Cross(x1, x2, x3);

			Vector4 result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), R);
			return result;
		}

		void Vector4::Normalize()
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVector4Normalize(v1);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), X);
		}

		void Vector4::Normalize(Vector4& result) const
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR X = XMVector4Normalize(v1);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
		}

		void Vector4::Clamp(const Vector4& vmin, const Vector4& vmax)
		{
			using namespace DirectX;
			XMVECTOR v1 = *this;
			XMVECTOR v2 = vmin;
			XMVECTOR v3 = vmax;
			XMVECTOR X = XMVectorClamp(v1, v2, v3);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), X);
		}

		void Vector4::Clamp(const Vector4& vmin, const Vector4& vmax, Vector4& result) const
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

		float Vector4::Distance(const Vector4& v1, const Vector4& v2)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR V = XMVectorSubtract(x2, x1);
			XMVECTOR X = XMVector4Length(V);
			return XMVectorGetX(X);
		}

		float Vector4::DistanceSquared(const Vector4& v1, const Vector4& v2)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR V = XMVectorSubtract(x2, x1);
			XMVECTOR X = XMVector4LengthSq(V);
			return XMVectorGetX(X);
		}

		void Vector4::Min(const Vector4& v1, const Vector4& v2, Vector4& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorMin(x1, x2);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
		}

		Vector4 Vector4::Min(const Vector4& v1, const Vector4& v2)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorMin(x1, x2);

			Vector4 result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
			return result;
		}

		void Vector4::Max(const Vector4& v1, const Vector4& v2, Vector4& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorMax(x1, x2);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
		}

		Vector4 Vector4::Max(const Vector4& v1, const Vector4& v2)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorMax(x1, x2);

			Vector4 result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
			return result;
		}

		void Vector4::Lerp(const Vector4& v1, const Vector4& v2, float t, Vector4& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorLerp(x1, x2, t);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
		}

		Vector4 Vector4::Lerp(const Vector4& v1, const Vector4& v2, float t)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorLerp(x1, x2, t);

			Vector4 result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
			return result;
		}

		void Vector4::SmoothStep(const Vector4& v1, const Vector4& v2, float t, Vector4& result)
		{
			using namespace DirectX;
			t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t);  // Clamp value to 0 to 1
			t = t*t*(3.f - 2.f*t);
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorLerp(x1, x2, t);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
		}

		Vector4 Vector4::SmoothStep(const Vector4& v1, const Vector4& v2, float t)
		{
			using namespace DirectX;
			t = (t > 1.0f) ? 1.0f : ((t < 0.0f) ? 0.0f : t);  // Clamp value to 0 to 1
			t = t*t*(3.f - 2.f*t);
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR X = XMVectorLerp(x1, x2, t);

			Vector4 result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
			return result;
		}

		void Vector4::Barycentric(const Vector4& v1, const Vector4& v2, const Vector4& v3, float f, float g, Vector4& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR x3 = v3;
			XMVECTOR X = XMVectorBaryCentric(x1, x2, x3, f, g);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
		}

		Vector4 Vector4::Barycentric(const Vector4& v1, const Vector4& v2, const Vector4& v3, float f, float g)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR x3 = v3;
			XMVECTOR X = XMVectorBaryCentric(x1, x2, x3, f, g);

			Vector4 result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
			return result;
		}

		void Vector4::CatmullRom(const Vector4& v1, const Vector4& v2, const Vector4& v3, const Vector4& v4, float t, Vector4& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR x3 = v3;
			XMVECTOR x4 = v4;
			XMVECTOR X = XMVectorCatmullRom(x1, x2, x3, x4, t);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
		}

		Vector4 Vector4::CatmullRom(const Vector4& v1, const Vector4& v2, const Vector4& v3, const Vector4& v4, float t)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = v2;
			XMVECTOR x3 = v3;
			XMVECTOR x4 = v4;
			XMVECTOR X = XMVectorCatmullRom(x1, x2, x3, x4, t);

			Vector4 result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
			return result;
		}

		void Vector4::Hermite(const Vector4& v1, const Vector4& t1, const Vector4& v2, const Vector4& t2, float t, Vector4& result)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = t1;
			XMVECTOR x3 = v2;
			XMVECTOR x4 = t2;
			XMVECTOR X = XMVectorHermite(x1, x2, x3, x4, t);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
		}

		Vector4 Vector4::Hermite(const Vector4& v1, const Vector4& t1, const Vector4& v2, const Vector4& t2, float t)
		{
			using namespace DirectX;
			XMVECTOR x1 = v1;
			XMVECTOR x2 = t1;
			XMVECTOR x3 = v2;
			XMVECTOR x4 = t2;
			XMVECTOR X = XMVectorHermite(x1, x2, x3, x4, t);

			Vector4 result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
			return result;
		}

		void Vector4::Reflect(const Vector4& ivec, const Vector4& nvec, Vector4& result)
		{
			using namespace DirectX;
			XMVECTOR i = ivec;
			XMVECTOR n = nvec;
			XMVECTOR X = XMVector4Reflect(i, n);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
		}

		Vector4 Vector4::Reflect(const Vector4& ivec, const Vector4& nvec)
		{
			using namespace DirectX;
			XMVECTOR i = ivec;
			XMVECTOR n = nvec;
			XMVECTOR X = XMVector4Reflect(i, n);

			Vector4 result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
			return result;
		}

		void Vector4::Refract(const Vector4& ivec, const Vector4& nvec, float refractionIndex, Vector4& result)
		{
			using namespace DirectX;
			XMVECTOR i = ivec;
			XMVECTOR n = nvec;
			XMVECTOR X = XMVector4Refract(i, n, refractionIndex);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
		}

		Vector4 Vector4::Refract(const Vector4& ivec, const Vector4& nvec, float refractionIndex)
		{
			using namespace DirectX;
			XMVECTOR i = ivec;
			XMVECTOR n = nvec;
			XMVECTOR X = XMVector4Refract(i, n, refractionIndex);

			Vector4 result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
			return result;
		}

		void Vector4::Transform(const Vector2& v, const Quaternion& quat, Vector4& result)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMVECTOR q = quat;
			XMVECTOR X = XMVector3Rotate(v1, q);
			X = XMVectorSelect(g_XMIdentityR3, X, g_XMSelect1110); // result.w = 1.f
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
		}

		Vector4 Vector4::Transform(const Vector2& v, const Quaternion& quat)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMVECTOR q = quat;
			XMVECTOR X = XMVector3Rotate(v1, q);
			X = XMVectorSelect(g_XMIdentityR3, X, g_XMSelect1110); // result.w = 1.f

			Vector4 result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
			return result;
		}

		void Vector4::Transform(const Vector3& v, const Quaternion& quat, Vector4& result)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMVECTOR q = quat;
			XMVECTOR X = XMVector3Rotate(v1, q);
			X = XMVectorSelect(g_XMIdentityR3, X, g_XMSelect1110); // result.w = 1.f
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
		}

		Vector4 Vector4::Transform(const Vector3& v, const Quaternion& quat)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMVECTOR q = quat;
			XMVECTOR X = XMVector3Rotate(v1, q);
			X = XMVectorSelect(g_XMIdentityR3, X, g_XMSelect1110); // result.w = 1.f

			Vector4 result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
			return result;
		}

		void Vector4::Transform(const Vector4& v, const Quaternion& quat, Vector4& result)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMVECTOR q = quat;
			XMVECTOR X = XMVector3Rotate(v1, q);
			X = XMVectorSelect(v1, X, g_XMSelect1110); // result.w = v.w
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
		}

		Vector4 Vector4::Transform(const Vector4& v, const Quaternion& quat)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMVECTOR q = quat;
			XMVECTOR X = XMVector3Rotate(v1, q);
			X = XMVectorSelect(v1, X, g_XMSelect1110); // result.w = v.w

			Vector4 result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
			return result;
		}

		void Vector4::Transform(const Vector4& v, const Matrix& m, Vector4& result)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVECTOR X = XMVector4Transform(v1, M);
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
		}

		Vector4 Vector4::Transform(const Vector4& v, const Matrix& m)
		{
			using namespace DirectX;
			XMVECTOR v1 = v;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVECTOR X = XMVector4Transform(v1, M);

			Vector4 result;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(&result), X);
			return result;
		}

		_Use_decl_annotations_
			void Vector4::Transform(const Vector4* varray, size_t count, const Matrix& m, Vector4* resultArray)
		{
			using namespace DirectX;
			XMMATRIX M = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&m));
			XMVector4TransformStream(reinterpret_cast<XMFLOAT4*>(resultArray), sizeof(XMFLOAT4), reinterpret_cast<const XMFLOAT4*>(varray), sizeof(XMFLOAT4), count, M);
		}

		const Vector4 Vector4::Zero = { 0.f, 0.f, 0.f, 0.f };
		const Vector4 Vector4::One = { 1.f, 1.f, 1.f, 1.f };
		const Vector4 Vector4::UnitX = { 1.f, 0.f, 0.f, 0.f };
		const Vector4 Vector4::UnitY = { 0.f, 1.f, 0.f, 0.f };
		const Vector4 Vector4::UnitZ = { 0.f, 0.f, 1.f, 0.f };
		const Vector4 Vector4::UnitW = { 0.f, 0.f, 0.f, 1.f };

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
		Matrix::Matrix(const Vector3& r0, const Vector3& r1, const Vector3& r2)
			: _11(r0.x), _12(r0.y), _13(r0.z), _14(0.f)
			, _21(r1.x), _22(r1.y), _23(r1.z), _24(0.f)
			, _31(r2.x), _32(r2.y), _33(r2.z), _34(0.f)
			, _41(0.f), _42(0.f), _43(0.f), _44(0.f)
		{}
		Matrix::Matrix(const Vector4& r0, const Vector4& r1, const Vector4& r2, const Vector4& r3)
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

		bool Matrix::Decompose(Vector3& scale, Quaternion& rotation, Vector3& translation) const
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

		Matrix Matrix::Compose(const Vector3& scale, const Quaternion& rotation, const Vector3& translation)
		{
			Matrix result;
			Compose(scale, rotation, translation, result);
			return result;
		}

		void Matrix::Compose(const Vector3& scale, const Quaternion& rotation, const Vector3& translation, Matrix& result)
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
			Matrix Matrix::CreateBillboard(const Vector3& object, const Vector3& cameraPosition, const Vector3& cameraUp, const Vector3* cameraForward)
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
			Matrix Matrix::CreateConstrainedBillboard(const Vector3& object, const Vector3& cameraPosition, const Vector3& rotateAxis,
				const Vector3* cameraForward, const Vector3* objectForward)
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

		Matrix Matrix::CreateTranslation(const Vector3& position)
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

		Matrix Matrix::CreateScale(const Vector3& scales)
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

		Matrix Matrix::CreateFromAxisAngle(const Vector3& axis, float angle)
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

		Matrix Matrix::CreateLookAt(const Vector3& eye, const Vector3& target, const Vector3& up)
		{
			using namespace DirectX;
			Matrix R;
			XMVECTOR eyev = eye;
			XMVECTOR targetv = target;
			XMVECTOR upv = up;
			XMStoreFloat4x4(reinterpret_cast<XMFLOAT4X4*>(&R), XMMatrixLookAtLH(eyev, targetv, upv));
			return R;
		}

		Matrix Matrix::CreateWorld(const Vector3& position, const Vector3& forward, const Vector3& up)
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

		Matrix Matrix::CreateShadow(const Vector3& lightDir, const Plane& plane)
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
		Plane::Plane(const Vector3& normal, float d) : x(normal.x), y(normal.y), z(normal.z), w(d) {}
		Plane::Plane(const Vector3& point1, const Vector3& point2, const Vector3& point3)
		{
			using namespace DirectX;
			XMVECTOR P0 = point1;
			XMVECTOR P1 = point2;
			XMVECTOR P2 = point3;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), XMPlaneFromPoints(P0, P1, P2));
		}
		Plane::Plane(const Vector3& point, const Vector3& normal)
		{
			using namespace DirectX;
			XMVECTOR P = point;
			XMVECTOR N = normal;
			XMStoreFloat4(reinterpret_cast<XMFLOAT4*>(this), XMPlaneFromPointNormal(P, N));
		}
		Plane::Plane(const Vector4& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
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

		float Plane::Dot(const Vector4& v) const
		{
			using namespace DirectX;
			XMVECTOR p = *this;
			XMVECTOR v0 = v;
			return XMVectorGetX(XMPlaneDot(p, v0));
		}

		float Plane::DotCoordinate(const Vector3& position) const
		{
			using namespace DirectX;
			XMVECTOR p = *this;
			XMVECTOR v0 = position;
			return XMVectorGetX(XMPlaneDotCoord(p, v0));
		}

		float Plane::DotNormal(const Vector3& normal) const
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
		Quaternion::Quaternion(const Vector3& v, float scalar) : x(v.x), y(v.y), z(v.z), w(scalar) {}
		Quaternion::Quaternion(const Vector4& v) : x(v.x), y(v.y), z(v.z), w(v.w) {}
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

		Vector3 Quaternion::ToEularRadians() const
		{
			/*float m00 = 1.f - (2.f * ((y * y) + z * z));
			float m01 = 2.f * (x * y + w * z);

			Math::Vector3 result
			(
				std::atan2(2.f * (y * z + w * x), 1.f - (2.f * ((x * x) + (y * y)))),
				std::atan2(-2.f * (x * z - w * y), std::sqrt((m00 * m00) + (m01 * m01))),
				std::atan2(m01, m00)
			);

			return result;*/

			double check = x * y + z * w;
			if (check > 0.499)
			{
				return Math::Vector3(2 * std::atan2(x, w), Math::PI / 2, 0);
			}
			else if (check < -0.499)
			{
				return Math::Vector3(-2 * std::atan2(x, w), -Math::PI / 2, 0);
			}
			else
			{
				return Math::Vector3
				(
					std::atan2(2 * x * w - 2 * y * z, 1 - 2 * x * x - 2 * z * z),
					std::asin(2 * check),
					std::atan2(2 * y * w - 2 * x * z, 1 - 2 * y * y - 2 * z * z)
				);
			}

			/*Math::Vector3 v;

			v.z = (float)std::atan2
			(
				2 * y * w - 2 * x * z,
				1 - 2 * std::pow(y, 2) - 2 * std::pow(z, 2)
			);

			v.y = (float)std::asin
			(
				2 * x*y + 2 * z*w
			);

			v.x = (float)std::atan2
			(
				2 * x*w - 2 * y*z,
				1 - 2 * std::pow(x, 2) - 2 * std::pow(z, 2)
			);

			if (x*y + z*w == 0.5)
			{
				v.z = (float)(2 * std::atan2(x, w));
				v.x = 0;
			}
			else if (x*y + z*w == -0.5)
			{
				v.z = (float)(-2 * std::atan2(x, w));
				v.x = 0;
			}

			return v;*/
		}

		Vector3 Quaternion::ToEularDegrees() const
		{
			Vector3 eular = ToEularRadians();
			eular.x = ToDegrees(eular.x);
			eular.y = ToDegrees(eular.y);
			eular.z = ToDegrees(eular.z);
			return eular;
		}

		//------------------------------------------------------------------------------
		// Static functions
		//------------------------------------------------------------------------------

		Quaternion Quaternion::CreateFromAxisAngle(const Vector3& axis, float angle)
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

		Transform::Transform() : scale(Vector3::One) {}
		Transform::Transform(const Math::Vector3& scale, const Math::Quaternion& rotation, const Math::Vector3& position) : scale(scale), rotation(rotation), position(position) {}
		Transform::Transform(const Matrix& matrix)
		{
			matrix.Decompose(scale, rotation, position);
		}

		Matrix Transform::Compose()
		{
			return Matrix::Compose(scale, rotation, position);
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

		Color::Color() : r(0.f), g(0.f), b(0.f), a(0.f) {}
		Color::Color(float r, float g, float b) : r(r), g(g), b(b), a(1.f) {}
		Color::Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
		Color::Color(const Vector3& clr) : r(clr.x), g(clr.y), b(clr.z), a(1.f) {}
		Color::Color(const Vector4& clr) : r(clr.x), g(clr.y), b(clr.z), a(clr.w) {}
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

		const D3D11_VIEWPORT* Viewport::Get11() const { return reinterpret_cast<const D3D11_VIEWPORT*>(this); }

		//------------------------------------------------------------------------------
		// Viewport operations
		//------------------------------------------------------------------------------

		Vector3 Viewport::Project(const Vector3& p, const Matrix& proj, const Matrix& view, const Matrix& world) const
		{
			Vector3 result;
			Project(p, proj, view, world, result);
			return result;
		}

		void Viewport::Project(const Vector3& p, const Matrix& proj, const Matrix& view, const Matrix& world, Vector3& result) const
		{
			using namespace DirectX;
			XMVECTOR v = p;
			XMMATRIX worldMatrix = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&world));
			XMMATRIX viewMatrix = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&view));
			XMMATRIX projMatrix = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&proj));
			v = XMVector3Project(v, x, y, width, height, minDepth, maxDepth, projMatrix, viewMatrix, worldMatrix);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), v);
		}

		Vector3 Viewport::Unproject(const Vector3& p, const Matrix& proj, const Matrix& view, const Matrix& world) const
		{
			Vector3 result;
			Unproject(p, proj, view, world, result);
			return result;
		}

		void Viewport::Unproject(const Vector3& p, const Matrix& proj, const Matrix& view, const Matrix& world, Vector3& result) const
		{
			using namespace DirectX;
			XMVECTOR v = XMLoadFloat3(reinterpret_cast<const XMFLOAT3*>(&p));
			XMMATRIX worldMatrix = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&world));
			XMMATRIX viewMatrix = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&view));
			XMMATRIX projMatrix = XMLoadFloat4x4(reinterpret_cast<const XMFLOAT4X4*>(&proj));
			v = XMVector3Unproject(v, x, y, width, height, minDepth, maxDepth, projMatrix, viewMatrix, worldMatrix);
			XMStoreFloat3(reinterpret_cast<XMFLOAT3*>(&result), v);
		}
	}
}