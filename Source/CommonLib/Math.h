#pragma once

#include <xmmintrin.h>
#include <random>
#include <cmath>

struct D3D11_VIEWPORT;

namespace est
{
	namespace math
	{
		constexpr float PI = 3.1415926535897932384626433832795f;
		constexpr float PI2 = 6.283185307179586476925286766559f;
		constexpr float DIVPI = 0.31830988618379067153776752674503f;
		constexpr float DIVPI2 = 0.15915494309189533576888376337251f;
		constexpr float PIDIV2 = 1.5707963267948966192313216916398f;
		constexpr float PIDIV4 = 0.78539816339744830961566084581988f;

		extern std::random_device random_device;
		extern std::mt19937_64 mt19937_64;

		constexpr float ToRadians(float degrees) { return degrees * (PI / 180.f); }
		constexpr float ToDegrees(float radians) { return radians * (180.f / PI); }

		bool NearEqual(float S1, float S2, float Epsilon);
		float ModAngle(float Value);
		
		float Sin(float Value);
		float SinEst(float Value);
		
		float Cos(float Value);
		float CosEst(float Value);
		
		void SinCos(_Out_ float* pSin, _Out_ float* pCos, float Value);
		void SinCosEst(_Out_ float* pSin, _Out_ float* pCos, float Value);
		
		float ASin(float Value);
		float ASinEst(float Value);
		
		float ACos(float Value);
		float ACosEst(float Value);

		template <typename T1, typename T2>
		inline constexpr T1 Lerp(T1 x, T1 y, T2 s)
		{
			return x + (T1)(s * (T2)(y - x));
		}

		inline float Smoothstep(float min, float max, float x)
		{
			// Scale, bias and saturate x to 0..1 range
			x = std::clamp((x - min) / (max - min), 0.f, 1.f);
			// Evaluate polynomial
			return x * x * (3.f - 2.f * x);
		}

		inline double Smoothstep(double min, double max, double x)
		{
			// Scale, bias and saturate x to 0..1 range
			x = std::clamp((x - min) / (max - min), 0.0, 1.0);
			// Evaluate polynomial
			return x * x * (3.0 - 2.0 * x);
		}

		template <typename T>
		inline constexpr bool IsZero(T value)
		{
			if (std::abs(value) <= std::numeric_limits<T>::epsilon())
				return true;

			return false;
		}

		template <typename T>
		inline constexpr bool IsEqual(T value1, T value2)
		{
			return IsZero(value1 - value2);
		}

		template <typename T>
		__forceinline T DivideByMultiple(T value, size_t alignment)
		{
			return (T)((value + alignment - 1) / alignment);
		}

		template <typename T>
		T Random(T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max())
		{
			if (min > max)
			{
				std::swap(min, max);
			}

			const std::uniform_int_distribution<T> distribution(min, max);
			return distribution(mt19937_64);
		}

		template <typename T>
		T RandomReal(T min = std::numeric_limits<T>::min(), T max = std::numeric_limits<T>::max())
		{
			if (min > max)
			{
				std::swap(min, max);
			}

			const std::uniform_real_distribution<T> distribution(min, max);
			return distribution(mt19937_64);
		}

		struct float2;
		struct float3;
		struct float4;
		struct Matrix;
		struct Quaternion;
		struct Plane;

		float2 CompressNormal(const float3& f3Normal);
		float3 DeCompressNormal(const float2& f2Normal);
		float3 CalcTangent(const float3& f3Normal);
		float3 CalcBinormal(const float3& f3Normal, const float3& f3Tangent);

		struct UByte4
		{
			union
			{
				struct
				{
					uint8_t x;
					uint8_t y;
					uint8_t z;
					uint8_t w;
				};
				uint32_t v{ 0 };
			};

			UByte4() = default;
			constexpr UByte4(uint8_t _x, uint8_t _y, uint8_t _z, uint8_t _w) : x(_x), y(_y), z(_z), w(_w) {}
			explicit constexpr UByte4(uint32_t Packed) : v(Packed) {}
			explicit UByte4(_In_reads_(4) const uint8_t *pArray) : x(pArray[0]), y(pArray[1]), z(pArray[2]), w(pArray[3]) {}
			UByte4(float _x, float _y, float _z, float _w);
			explicit UByte4(_In_reads_(4) const float *pArray);

			UByte4& operator= (const UByte4& UByte4) { x = UByte4.x; y = UByte4.y; z = UByte4.z; w = UByte4.w; return *this; }
			UByte4& operator= (uint32_t Packed) { v = Packed; return *this; }
		};

		struct int2
		{
			int32_t x{ 0 };
			int32_t y{ 0 };

			int2();
			explicit int2(int x);
			int2(int x, int y);
			int2(const int2& V);

			// Comparison operators
			bool operator == (const int2& V) const;
			bool operator != (const int2& V) const;

			// Assignment operators
			int2& operator= (const int2& V) { x = V.x; y = V.y; return *this; }
			int2& operator+= (const int2& V);
			int2& operator-= (const int2& V);
			int2& operator*= (const int2& V);
			int2& operator*= (float S);
			int2& operator/= (float S);

			// Unary operators
			int2 operator+ () const { return *this; }
			int2 operator- () const { return int2(-x, -y); }

			static const int2 Zero;
			static const int2 One;
			static const int2 UnitX;
			static const int2 UnitY;
		};

		struct int3
		{
			int32_t x{ 0 };
			int32_t y{ 0 };
			int32_t z{ 0 };

			int3();
			explicit int3(int x);
			int3(int x, int y, int z);
			int3(const int3& V);

			// Comparison operators
			bool operator == (const int3& V) const;
			bool operator != (const int3& V) const;

			// Assignment operators
			int3& operator= (const int3& V) { x = V.x; y = V.y; z = V.z; return *this; }
			int3& operator+= (const int3& V);
			int3& operator-= (const int3& V);
			int3& operator*= (const int3& V);
			int3& operator*= (float S);
			int3& operator/= (float S);

			// Unary operators
			int3 operator+ () const { return *this; }
			int3 operator- () const { return int3(-x, -y, -z); }

			static const int3 Zero;
			static const int3 One;
			static const int3 UnitX;
			static const int3 UnitY;
			static const int3 UnitZ;
		};

		struct int4
		{
			int32_t x{ 0 };
			int32_t y{ 0 };
			int32_t z{ 0 };
			int32_t w{ 0 };

			int4();
			explicit int4(int x);
			int4(int x, int y, int z, int w);
			int4(const int4& V);

			// Comparison operators
			bool operator == (const int4& V) const;
			bool operator != (const int4& V) const;

			// Assignment operators
			int4& operator= (const int4& V) { x = V.x; y = V.y; z = V.z; w = V.w; return *this; }
			int4& operator+= (const int4& V);
			int4& operator-= (const int4& V);
			int4& operator*= (const int4& V);
			int4& operator*= (float S);
			int4& operator/= (float S);

			// Unary operators
			int4 operator+ () const { return *this; }
			int4 operator- () const { return int4(-x, -y, -z, -w); }

			static const int4 Zero;
			static const int4 One;
			static const int4 UnitX;
			static const int4 UnitY;
			static const int4 UnitZ;
			static const int4 UnitW;
		};

		struct uint2
		{
			uint32_t x{ 0 };
			uint32_t y{ 0 };

			uint2();
			explicit uint2(uint32_t x);
			uint2(uint32_t x, uint32_t y);
			uint2(const uint2& V);

			// Comparison operators
			bool operator == (const uint2& V) const;
			bool operator != (const uint2& V) const;

			// Assignment operators
			uint2& operator= (const uint2& V) { x = V.x; y = V.y; return *this; }
			uint2& operator+= (const uint2& V);
			uint2& operator-= (const uint2& V);
			uint2& operator*= (const uint2& V);
			uint2& operator*= (float S);
			uint2& operator/= (float S);

			// Unary operators
			uint2 operator+ () const { return *this; }

			static const uint2 Zero;
			static const uint2 One;
			static const uint2 UnitX;
			static const uint2 UnitY;
		};

		struct uint3
		{
			uint32_t x{ 0 };
			uint32_t y{ 0 };
			uint32_t z{ 0 };

			uint3();
			explicit uint3(uint32_t x);
			uint3(uint32_t x, uint32_t y, uint32_t z);
			uint3(const uint3& V);

			// Comparison operators
			bool operator == (const uint3& V) const;
			bool operator != (const uint3& V) const;

			// Assignment operators
			uint3& operator= (const uint3& V) { x = V.x; y = V.y; z = V.z; return *this; }
			uint3& operator+= (const uint3& V);
			uint3& operator-= (const uint3& V);
			uint3& operator*= (const uint3& V);
			uint3& operator*= (float S);
			uint3& operator/= (float S);

			// Unary operators
			uint3 operator+ () const { return *this; }

			static const uint3 Zero;
			static const uint3 One;
			static const uint3 UnitX;
			static const uint3 UnitY;
			static const uint3 UnitZ;
		};

		struct uint4
		{
			uint32_t x{ 0 };
			uint32_t y{ 0 };
			uint32_t z{ 0 };
			uint32_t w{ 0 };

			uint4();
			explicit uint4(uint32_t x);
			uint4(uint32_t x, uint32_t y, uint32_t z, uint32_t w);
			uint4(const uint4& V);

			// Comparison operators
			bool operator == (const uint4& V) const;
			bool operator != (const uint4& V) const;

			// Assignment operators
			uint4& operator= (const uint4& V) { x = V.x; y = V.y; z = V.z; w = V.w; return *this; }
			uint4& operator+= (const uint4& V);
			uint4& operator-= (const uint4& V);
			uint4& operator*= (const uint4& V);
			uint4& operator*= (float S);
			uint4& operator/= (float S);

			// Unary operators
			uint4 operator+ () const { return *this; }

			static const uint4 Zero;
			static const uint4 One;
			static const uint4 UnitX;
			static const uint4 UnitY;
			static const uint4 UnitZ;
			static const uint4 UnitW;
		};

		//------------------------------------------------------------------------------
		// 2D vector
		struct float2
		{
			float x{ 0.f };
			float y{ 0.f };

			float2();
			explicit float2(float x);
			float2(float x, float y);
			explicit float2(_In_reads_(2) const float *pArray);
			float2(const __m128& V);
			float2(const float2& V);

			operator __m128() const;

			float operator [] (int index) const { return reinterpret_cast<const float*>(this)[index]; }

			// Comparison operators
			bool operator == (const float2& V) const;
			bool operator != (const float2& V) const;

			// Assignment operators
			float2& operator= (const float2& V) { x = V.x; y = V.y; return *this; }
			float2& operator+= (const float2& V);
			float2& operator-= (const float2& V);
			float2& operator*= (const float2& V);
			float2& operator*= (float S);
			float2& operator/= (float S);

			// Unary operators
			float2 operator+ () const { return *this; }
			float2 operator- () const { return float2(-x, -y); }

			// Vector operations
			bool InBounds(const float2& Bounds) const;

			float Length() const;
			float LengthSquared() const;

			float Dot(const float2& V) const;
			void Cross(const float2& V, float2& result) const;
			float2 Cross(const float2& V) const;

			void Normalize();
			void Normalize(float2& result) const;

			void Clamp(const float2& vmin, const float2& vmax);
			void Clamp(const float2& vmin, const float2& vmax, float2& result) const;

			// Static functions
			static float Distance(const float2& v1, const float2& v2);
			static float DistanceSquared(const float2& v1, const float2& v2);

			static void Min(const float2& v1, const float2& v2, float2& result);
			static float2 Min(const float2& v1, const float2& v2);

			static void Max(const float2& v1, const float2& v2, float2& result);
			static float2 Max(const float2& v1, const float2& v2);

			static void Lerp(const float2& v1, const float2& v2, float t, float2& result);
			static float2 Lerp(const float2& v1, const float2& v2, float t);

			static void SmoothStep(const float2& v1, const float2& v2, float t, float2& result);
			static float2 SmoothStep(const float2& v1, const float2& v2, float t);

			static void Barycentric(const float2& v1, const float2& v2, const float2& v3, float f, float g, float2& result);
			static float2 Barycentric(const float2& v1, const float2& v2, const float2& v3, float f, float g);

			static void CatmullRom(const float2& v1, const float2& v2, const float2& v3, const float2& v4, float t, float2& result);
			static float2 CatmullRom(const float2& v1, const float2& v2, const float2& v3, const float2& v4, float t);

			static void Hermite(const float2& v1, const float2& t1, const float2& v2, const float2& t2, float t, float2& result);
			static float2 Hermite(const float2& v1, const float2& t1, const float2& v2, const float2& t2, float t);

			static void Reflect(const float2& ivec, const float2& nvec, float2& result);
			static float2 Reflect(const float2& ivec, const float2& nvec);

			static void Refract(const float2& ivec, const float2& nvec, float refractionIndex, float2& result);
			static float2 Refract(const float2& ivec, const float2& nvec, float refractionIndex);

			static void Transform(const float2& v, const Quaternion& quat, float2& result);
			static float2 Transform(const float2& v, const Quaternion& quat);

			static void Transform(const float2& v, const Matrix& m, float2& result);
			static float2 Transform(const float2& v, const Matrix& m);
			static void Transform(_In_reads_(count) const float2* varray, size_t count, const Matrix& m, _Out_writes_(count) float2* resultArray);

			static void Transform(const float2& v, const Matrix& m, float4& result);
			static void Transform(_In_reads_(count) const float2* varray, size_t count, const Matrix& m, _Out_writes_(count) float4* resultArray);

			static void TransformNormal(const float2& v, const Matrix& m, float2& result);
			static float2 TransformNormal(const float2& v, const Matrix& m);
			static void TransformNormal(_In_reads_(count) const float2* varray, size_t count, const Matrix& m, _Out_writes_(count) float2* resultArray);

			// Constants
			static const float2 Zero;
			static const float2 One;
			static const float2 UnitX;
			static const float2 UnitY;
		};

		// Binary operators
		float2 operator+ (const float2& V1, const float2& V2);
		float2 operator- (const float2& V1, const float2& V2);
		float2 operator* (const float2& V1, const float2& V2);
		float2 operator* (const float2& V, float S);
		float2 operator/ (const float2& V1, const float2& V2);
		float2 operator/ (const float2& V1, float S);
		float2 operator* (float S, const float2& V);

		//------------------------------------------------------------------------------
		// 3D vector
		struct float3
		{
			float x{ 0.f };
			float y{ 0.f };
			float z{ 0.f };

			float3();
			explicit float3(float x);
			float3(float x, float y, float z);
			explicit float3(_In_reads_(3) const float *pArray);
			float3(const __m128& V);
			float3(const float3& V);

			operator __m128() const;

			float operator [] (int index) const { return reinterpret_cast<const float*>(this)[index]; }

			// Comparison operators
			bool operator == (const float3& V) const;
			bool operator != (const float3& V) const;

			// Assignment operators
			float3& operator= (const float3& V) { x = V.x; y = V.y; z = V.z; return *this; }
			float3& operator+= (const float3& V);
			float3& operator-= (const float3& V);
			float3& operator*= (const float3& V);
			float3& operator/= (const float3& V);
			float3& operator*= (float S);
			float3& operator/= (float S);

			// Unary operators
			float3 operator+ () const { return *this; }
			float3 operator- () const;

			// Vector operations
			bool InBounds(const float3& Bounds) const;

			float Length() const;
			float LengthSquared() const;

			float Dot(const float3& V) const;
			void Cross(const float3& V, float3& result) const;
			float3 Cross(const float3& V) const;

			void Normalize();
			void Normalize(float3& result) const;

			void Clamp(const float3& vmin, const float3& vmax);
			void Clamp(const float3& vmin, const float3& vmax, float3& result) const;

			// Static functions
			static float Distance(const float3& v1, const float3& v2);
			static float DistanceSquared(const float3& v1, const float3& v2);

			static void Min(const float3& v1, const float3& v2, float3& result);
			static float3 Min(const float3& v1, const float3& v2);

			static void Max(const float3& v1, const float3& v2, float3& result);
			static float3 Max(const float3& v1, const float3& v2);

			static void Lerp(const float3& v1, const float3& v2, float t, float3& result);
			static float3 Lerp(const float3& v1, const float3& v2, float t);

			static void SmoothStep(const float3& v1, const float3& v2, float t, float3& result);
			static float3 SmoothStep(const float3& v1, const float3& v2, float t);

			static void Barycentric(const float3& v1, const float3& v2, const float3& v3, float f, float g, float3& result);
			static float3 Barycentric(const float3& v1, const float3& v2, const float3& v3, float f, float g);

			static void CatmullRom(const float3& v1, const float3& v2, const float3& v3, const float3& v4, float t, float3& result);
			static float3 CatmullRom(const float3& v1, const float3& v2, const float3& v3, const float3& v4, float t);

			static void Hermite(const float3& v1, const float3& t1, const float3& v2, const float3& t2, float t, float3& result);
			static float3 Hermite(const float3& v1, const float3& t1, const float3& v2, const float3& t2, float t);

			static void Reflect(const float3& ivec, const float3& nvec, float3& result);
			static float3 Reflect(const float3& ivec, const float3& nvec);

			static void Refract(const float3& ivec, const float3& nvec, float refractionIndex, float3& result);
			static float3 Refract(const float3& ivec, const float3& nvec, float refractionIndex);

			static void Transform(const float3& v, const Quaternion& quat, float3& result);
			static float3 Transform(const float3& v, const Quaternion& quat);

			static void Transform(const float3& v, const Matrix& m, float3& result);
			static float3 Transform(const float3& v, const Matrix& m);
			static void Transform(_In_reads_(count) const float3* varray, size_t count, const Matrix& m, _Out_writes_(count) float3* resultArray);

			static void Transform(const float3& v, const Matrix& m, float4& result);
			static void Transform(_In_reads_(count) const float3* varray, size_t count, const Matrix& m, _Out_writes_(count) float4* resultArray);

			static void TransformNormal(const float3& v, const Matrix& m, float3& result);
			static float3 TransformNormal(const float3& v, const Matrix& m);
			static void TransformNormal(_In_reads_(count) const float3* varray, size_t count, const Matrix& m, _Out_writes_(count) float3* resultArray);

			static float3 FresnelTerm(const float3& v1, const float3& v2);

			// Constants
			static const float3 Zero;
			static const float3 One;
			static const float3 UnitX;
			static const float3 UnitY;
			static const float3 UnitZ;
			static const float3 Up;
			static const float3 Down;
			static const float3 Right;
			static const float3 Left;
			static const float3 Forward;
			static const float3 Backward;
		};

		// Binary operators
		float3 operator+ (const float3& V1, const float3& V2);
		float3 operator- (const float3& V1, const float3& V2);
		float3 operator* (const float3& V1, const float3& V2);
		float3 operator* (const float3& V, float S);
		float3 operator/ (const float3& V1, const float3& V2);
		float3 operator/ (const float3& V, float S);
		float3 operator* (float S, const float3& V);

		//------------------------------------------------------------------------------
		// 4D vector
		struct float4
		{
			float x{ 0.f };
			float y{ 0.f };
			float z{ 0.f };
			float w{ 0.f };

			float4();
			explicit float4(float x);
			float4(float x, float y, float z, float w);
			explicit float4(_In_reads_(4) const float *pArray);
			float4(const __m128& V);
			float4(const float4& V);

			operator __m128() const;

			float operator [] (int index) const { return reinterpret_cast<const float*>(this)[index]; }

			// Comparison operators
			bool operator == (const float4& V) const;
			bool operator != (const float4& V) const;

			// Assignment operators
			float4& operator= (const float4& V) { x = V.x; y = V.y; z = V.z; w = V.w; return *this; }
			float4& operator+= (const float4& V);
			float4& operator-= (const float4& V);
			float4& operator*= (const float4& V);
			float4& operator*= (float S);
			float4& operator/= (float S);

			// Unary operators
			float4 operator+ () const { return *this; }
			float4 operator- () const;

			// Vector operations
			bool InBounds(const float4& Bounds) const;

			float Length() const;
			float LengthSquared() const;

			float Dot(const float4& V) const;
			void Cross(const float4& v1, const float4& v2, float4& result) const;
			float4 Cross(const float4& v1, const float4& v2) const;

			void Normalize();
			void Normalize(float4& result) const;

			void Clamp(const float4& vmin, const float4& vmax);
			void Clamp(const float4& vmin, const float4& vmax, float4& result) const;

			// Static functions
			static float Distance(const float4& v1, const float4& v2);
			static float DistanceSquared(const float4& v1, const float4& v2);

			static void Min(const float4& v1, const float4& v2, float4& result);
			static float4 Min(const float4& v1, const float4& v2);

			static void Max(const float4& v1, const float4& v2, float4& result);
			static float4 Max(const float4& v1, const float4& v2);

			static void Lerp(const float4& v1, const float4& v2, float t, float4& result);
			static float4 Lerp(const float4& v1, const float4& v2, float t);

			static void SmoothStep(const float4& v1, const float4& v2, float t, float4& result);
			static float4 SmoothStep(const float4& v1, const float4& v2, float t);

			static void Barycentric(const float4& v1, const float4& v2, const float4& v3, float f, float g, float4& result);
			static float4 Barycentric(const float4& v1, const float4& v2, const float4& v3, float f, float g);

			static void CatmullRom(const float4& v1, const float4& v2, const float4& v3, const float4& v4, float t, float4& result);
			static float4 CatmullRom(const float4& v1, const float4& v2, const float4& v3, const float4& v4, float t);

			static void Hermite(const float4& v1, const float4& t1, const float4& v2, const float4& t2, float t, float4& result);
			static float4 Hermite(const float4& v1, const float4& t1, const float4& v2, const float4& t2, float t);

			static void Reflect(const float4& ivec, const float4& nvec, float4& result);
			static float4 Reflect(const float4& ivec, const float4& nvec);

			static void Refract(const float4& ivec, const float4& nvec, float refractionIndex, float4& result);
			static float4 Refract(const float4& ivec, const float4& nvec, float refractionIndex);

			static void Transform(const float2& v, const Quaternion& quat, float4& result);
			static float4 Transform(const float2& v, const Quaternion& quat);

			static void Transform(const float3& v, const Quaternion& quat, float4& result);
			static float4 Transform(const float3& v, const Quaternion& quat);

			static void Transform(const float4& v, const Quaternion& quat, float4& result);
			static float4 Transform(const float4& v, const Quaternion& quat);

			static void Transform(const float4& v, const Matrix& m, float4& result);
			static float4 Transform(const float4& v, const Matrix& m);
			static void Transform(_In_reads_(count) const float4* varray, size_t count, const Matrix& m, _Out_writes_(count) float4* resultArray);

			// Constants
			static const float4 Zero;
			static const float4 One;
			static const float4 UnitX;
			static const float4 UnitY;
			static const float4 UnitZ;
			static const float4 UnitW;
		};

		// Binary operators
		float4 operator+ (const float4& V1, const float4& V2);
		float4 operator- (const float4& V1, const float4& V2);
		float4 operator* (const float4& V1, const float4& V2);
		float4 operator* (const float4& V, float S);
		float4 operator/ (const float4& V1, const float4& V2);
		float4 operator/ (const float4& V, float S);
		float4 operator* (float S, const float4& V);

		//------------------------------------------------------------------------------
		struct Matrix
		{
			union
			{
				struct
				{
					float _11, _12, _13, _14;
					float _21, _22, _23, _24;
					float _31, _32, _33, _34;
					float _41, _42, _43, _44;
				};
				struct
				{
					float4 _r0;
					float4 _r1;
					float4 _r2;
					float4 _r3;
				};
				float m[4][4];
			};

			Matrix();
			Matrix(float m00, float m01, float m02, float m03,
				float m10, float m11, float m12, float m13,
				float m20, float m21, float m22, float m23,
				float m30, float m31, float m32, float m33);
			explicit Matrix(const float3& r0, const float3& r1, const float3& r2);
			explicit Matrix(const float4& r0, const float4& r1, const float4& r2, const float4& r3);
			Matrix(const Matrix& M);

			// Comparison operators
			bool operator == (const Matrix& M) const;
			bool operator != (const Matrix& M) const;

			// Assignment operators
			Matrix& operator= (const Matrix& M) { memcpy_s(this, sizeof(float) * 16, &M, sizeof(float) * 16); return *this; }
			Matrix& operator+= (const Matrix& M);
			Matrix& operator-= (const Matrix& M);
			Matrix& operator*= (const Matrix& M);
			Matrix& operator*= (float S);
			Matrix& operator/= (float S);

			Matrix& operator/= (const Matrix& M);
			// Element-wise divide

			// Unary operators
			Matrix operator+ () const { return *this; }
			Matrix operator- () const;

			// Properties
			float3 Up() const { return float3(_12, _22, _32); }
			void Up(const float3& v) { _12 = v.x; _22 = v.y; _32 = v.z; }

			float3 Down() const { return float3(-_12, -_22, -_32); }
			void Down(const float3& v) { _12 = -v.x; _22 = -v.y; _32 = -v.z; }

			float3 Right() const { return float3(_11, _21, _31); }
			void Right(const float3& v) { _11 = v.x; _21 = v.y; _31 = v.z; }

			float3 Left() const { return float3(-_11, -_21, -_31); }
			void Left(const float3& v) { _11 = -v.x; _21 = -v.y; _31 = -v.z; }

			float3 Forward() const { return float3(_13, _23, _33); }
			void Forward(const float3& v) { _13 = v.x; _23 = v.y; _33 = v.z; }

			float3 Backward() const { return float3(-_13, -_23, -_33); }
			void Backward(const float3& v) { _13 = -v.x; _23 = -v.y; _33 = -v.z; }

			float3 Translation() const { return float3(_41, _42, _43); }
			void Translation(const float3& v) { _41 = v.x; _42 = v.y; _43 = v.z; }

			// Matrix operations
			bool Decompose(float3& scale, Quaternion& rotation, float3& translation) const;

			Matrix Transpose() const;
			void Transpose(Matrix& result) const;

			Matrix Invert() const;
			void Invert(Matrix& result) const;

			float Determinant() const;

			// Static functions
			static Matrix Compose(const float3& scale, const Quaternion& rotation, const float3& translation);
			static void Compose(const float3& scale, const Quaternion& rotation, const float3& translation, Matrix& result);

			static Matrix CreateBillboard(const float3& object, const float3& cameraPosition, const float3& cameraUp, _In_opt_ const float3* cameraForward = nullptr);

			static Matrix CreateConstrainedBillboard(const float3& object, const float3& cameraPosition, const float3& rotateAxis,
				_In_opt_ const float3* cameraForward = nullptr, _In_opt_ const float3* objectForward = nullptr);

			static Matrix CreateTranslation(const float3& position);
			static Matrix CreateTranslation(float x, float y, float z);

			static Matrix CreateScale(const float3& scales);
			static Matrix CreateScale(float xs, float ys, float zs);
			static Matrix CreateScale(float scale);

			static Matrix CreateRotationX(float radians);
			static Matrix CreateRotationY(float radians);
			static Matrix CreateRotationZ(float radians);

			static Matrix CreateFromAxisAngle(const float3& axis, float angle);

			static Matrix CreatePerspectiveFieldOfView(float fov, float aspectRatio, float nearPlane, float farPlane);
			static Matrix CreatePerspective(float width, float height, float nearPlane, float farPlane);
			static Matrix CreatePerspectiveOffCenter(float left, float right, float bottom, float top, float nearPlane, float farPlane);
			static Matrix CreateOrthographic(float width, float height, float zNearPlane, float zFarPlane);
			static Matrix CreateOrthographicOffCenter(float left, float right, float bottom, float top, float zNearPlane, float zFarPlane);

			static Matrix CreateLookAt(const float3& position, const float3& target, const float3& up);
			static Matrix CreateWorld(const float3& position, const float3& forward, const float3& up);

			static Matrix CreateFromQuaternion(const Quaternion& quat);

			static Matrix CreateFromYawPitchRoll(float yaw, float pitch, float roll);

			static Matrix CreateShadow(const float3& lightDir, const Plane& plane);

			static Matrix CreateReflection(const Plane& plane);

			static void Lerp(const Matrix& M1, const Matrix& M2, float t, Matrix& result);
			static Matrix Lerp(const Matrix& M1, const Matrix& M2, float t);

			static void Transform(const Matrix& M, const Quaternion& rotation, Matrix& result);
			static Matrix Transform(const Matrix& M, const Quaternion& rotation);

			// Constants
			static const Matrix Identity;
			static const Matrix ZConversion;
		};

		// Binary operators
		Matrix operator+ (const Matrix& M1, const Matrix& M2);
		Matrix operator- (const Matrix& M1, const Matrix& M2);
		Matrix operator* (const Matrix& M1, const Matrix& M2);
		Matrix operator* (const Matrix& M, float S);
		Matrix operator/ (const Matrix& M, float S);
		Matrix operator/ (const Matrix& M1, const Matrix& M2);
		// Element-wise divide
		Matrix operator* (float S, const Matrix& M);


		//-----------------------------------------------------------------------------
		// Plane
		struct Plane
		{
			float x{ 0.f };
			float y{ 0.f };
			float z{ 0.f };
			float w{ 1.f };

			Plane();
			Plane(float x, float y, float z, float w);
			Plane(const float3& normal, float d);
			Plane(const float3& point1, const float3& point2, const float3& point3);
			Plane(const float3& point, const float3& normal);
			explicit Plane(const float4& v);
			explicit Plane(_In_reads_(4) const float *pArray);
			Plane(const __m128& V);

			operator __m128() const;

			// Comparison operators
			bool operator == (const Plane& p) const;
			bool operator != (const Plane& p) const;

			// Assignment operators
			Plane& operator= (const Plane& p) { x = p.x; y = p.y; z = p.z; w = p.w; return *this; }

			// Properties
			float3 Normal() const { return float3(x, y, z); }
			void Normal(const float3& normal) { x = normal.x; y = normal.y; z = normal.z; }

			float D() const { return w; }
			void D(float d) { w = d; }

			// Plane operations
			void Normalize();
			void Normalize(Plane& result) const;

			float Dot(const float4& v) const;
			float DotCoordinate(const float3& position) const;
			float DotNormal(const float3& normal) const;

			// Static functions
			static void Transform(const Plane& plane, const Matrix& M, Plane& result);
			static Plane Transform(const Plane& plane, const Matrix& M);

			static void Transform(const Plane& plane, const Quaternion& rotation, Plane& result);
			static Plane Transform(const Plane& plane, const Quaternion& rotation);
			// Input quaternion must be the inverse transpose of the transformation
		};

		//------------------------------------------------------------------------------
		// Quaternion
		struct Quaternion
		{
			float x{ 0.f };
			float y{ 0.f };
			float z{ 0.f };
			float w{ 1.f };

			Quaternion();
			Quaternion(float x, float y, float z, float w);
			Quaternion(const float3& v, float scalar);
			explicit Quaternion(const float4& v);
			explicit Quaternion(_In_reads_(4) const float *pArray);
			Quaternion(const __m128& V);

			operator __m128() const;

			// Comparison operators
			bool operator == (const Quaternion& q) const;
			bool operator != (const Quaternion& q) const;

			// Assignment operators
			Quaternion& operator= (const Quaternion& q) { x = q.x; y = q.y; z = q.z; w = q.w; return *this; }
			Quaternion& operator+= (const Quaternion& q);
			Quaternion& operator-= (const Quaternion& q);
			Quaternion& operator*= (const Quaternion& q);
			Quaternion& operator*= (float S);
			Quaternion& operator/= (const Quaternion& q);

			// Unary operators
			Quaternion operator+ () const { return *this; }
			Quaternion operator- () const;

			// Quaternion operations
			float Length() const;
			float LengthSquared() const;

			void Normalize();
			void Normalize(Quaternion& result) const;

			void Conjugate();
			void Conjugate(Quaternion& result) const;

			void Inverse(Quaternion& result) const;
			Quaternion Inverse() const;

			float Dot(const Quaternion& Q) const;

			float3 ToEularRadians() const;
			float3 ToEularDegrees() const;

			// Static functions
			static Quaternion CreateFromAxisAngle(const float3& axis, float angle);
			static Quaternion CreateFromYawPitchRoll(float yaw, float pitch, float roll);
			static Quaternion CreateFromRotationMatrix(const Matrix& M);

			static void Lerp(const Quaternion& q1, const Quaternion& q2, float t, Quaternion& result);
			static Quaternion Lerp(const Quaternion& q1, const Quaternion& q2, float t);

			static void Slerp(const Quaternion& q1, const Quaternion& q2, float t, Quaternion& result);
			static Quaternion Slerp(const Quaternion& q1, const Quaternion& q2, float t);

			static void Concatenate(const Quaternion& q1, const Quaternion& q2, Quaternion& result);
			static Quaternion Concatenate(const Quaternion& q1, const Quaternion& q2);

			// Constants
			static const Quaternion Identity;
		};

		// Binary operators
		Quaternion operator+ (const Quaternion& Q1, const Quaternion& Q2);
		Quaternion operator- (const Quaternion& Q1, const Quaternion& Q2);
		Quaternion operator* (const Quaternion& Q1, const Quaternion& Q2);
		Quaternion operator* (const Quaternion& Q, float S);
		Quaternion operator/ (const Quaternion& Q, float S);
		Quaternion operator/ (const Quaternion& Q1, const Quaternion& Q2);
		Quaternion operator* (float S, const Quaternion& Q);

		struct Transform
		{
			math::float3 scale;
			math::Quaternion rotation;
			math::float3 position;

			Transform();
			Transform(const float3& scale, const Quaternion& rotation, const float3& position);
			Transform(const Matrix& matrix);

			Matrix Compose() const;

			static void Lerp(const Transform& v1, const Transform& v2, float lerpPerccent, Transform& result);
		};

		//------------------------------------------------------------------------------
		// Color
		struct RGBA
		{
			union
			{
				struct
				{
					uint8_t r;  // Red:     0/255 to 255/255
					uint8_t g;  // Green:   0/255 to 255/255
					uint8_t b;  // Blue:    0/255 to 255/255
					uint8_t a;  // Alpha:   0/255 to 255/255
				};
				uint32_t c{ 0x0000ff };
			};

			RGBA();
			RGBA(uint32_t Color);
			RGBA(float _r, float _g, float _b, float _a);
			explicit RGBA(_In_reads_(4) const float *pArray);

			operator uint32_t () const { return c; }

			RGBA& operator= (const RGBA& Color) { c = Color.c; return *this; }
			RGBA& operator= (const uint32_t Color) { c = Color; return *this; }
		};

		struct Color
		{
			float r{ 0.f };
			float g{ 0.f };
			float b{ 0.f };
			float a{ 1.f };

			Color();
			Color(float r, float g, float b);
			Color(float r, float g, float b, float a);
			explicit Color(const float3& clr);
			explicit Color(const float4& clr);
			explicit Color(_In_reads_(4) const float *pArray);
			Color(const __m128& V);

			explicit Color(const RGBA& Packed);

			operator __m128() const;
			operator const float*() const { return reinterpret_cast<const float*>(this); }

			// Comparison operators
			bool operator == (const Color& c) const;
			bool operator != (const Color& c) const;

			// Assignment operators
			Color& operator= (const Color& c) { r = c.r; g = c.g; b = c.b; a = c.a; return *this; }
			Color& operator= (const float4& c) { r = c.x; g = c.y; b = c.z; a = c.w; return *this; }
			Color& operator= (const RGBA& Packed);
			Color& operator+= (const Color& c);
			Color& operator-= (const Color& c);
			Color& operator*= (const Color& c);
			Color& operator*= (float S);
			Color& operator/= (const Color& c);

			// Unary operators
			Color operator+ () const { return *this; }
			Color operator- () const;

			// Properties
			float R() const { return r; }
			void R(float _r) { r = _r; }

			float G() const { return g; }
			void G(float _g) { g = _g; }

			float B() const { return b; }
			void B(float _b) { b = _b; }

			float A() const { return a; }
			void A(float _a) { a = _a; }

			// Color operations
			RGBA GetRGBA() const;

			float3 ToVector3() const { return float3(r, g, b); }
			float4 Tofloat4() const { return float4(r, g, b, a); }

			void Negate();
			void Negate(Color& result) const;

			void Saturate();
			void Saturate(Color& result) const;

			void Premultiply();
			void Premultiply(Color& result) const;

			void AdjustSaturation(float sat);
			void AdjustSaturation(float sat, Color& result) const;

			void AdjustContrast(float contrast);
			void AdjustContrast(float contrast, Color& result) const;

			// Static functions
			static void Modulate(const Color& c1, const Color& c2, Color& result);
			static Color Modulate(const Color& c1, const Color& c2);

			static void Lerp(const Color& c1, const Color& c2, float t, Color& result);
			static Color Lerp(const Color& c1, const Color& c2, float t);

			// Standard colors (Red/Green/Blue/Alpha)
			static const Color AliceBlue;
			static const Color AntiqueWhite;
			static const Color Aqua;
			static const Color Aquamarine;
			static const Color Azure;
			static const Color Beige;
			static const Color Bisque;
			static const Color Black;
			static const Color BlanchedAlmond;
			static const Color Blue;
			static const Color BlueViolet;
			static const Color Brown;
			static const Color BurlyWood;
			static const Color CadetBlue;
			static const Color Chartreuse;
			static const Color Chocolate;
			static const Color Coral;
			static const Color CornflowerBlue;
			static const Color Cornsilk;
			static const Color Crimson;
			static const Color Cyan;
			static const Color DarkBlue;
			static const Color DarkCyan;
			static const Color DarkGoldenrod;
			static const Color DarkGray;
			static const Color DarkGreen;
			static const Color DarkKhaki;
			static const Color DarkMagenta;
			static const Color DarkOliveGreen;
			static const Color DarkOrange;
			static const Color DarkOrchid;
			static const Color DarkRed;
			static const Color DarkSalmon;
			static const Color DarkSeaGreen;
			static const Color DarkSlateBlue;
			static const Color DarkSlateGray;
			static const Color DarkTurquoise;
			static const Color DarkViolet;
			static const Color DeepPink;
			static const Color DeepSkyBlue;
			static const Color DimGray;
			static const Color DodgerBlue;
			static const Color Firebrick;
			static const Color FloralWhite;
			static const Color ForestGreen;
			static const Color Fuchsia;
			static const Color Gainsboro;
			static const Color GhostWhite;
			static const Color Gold;
			static const Color Goldenrod;
			static const Color Gray;
			static const Color Green;
			static const Color GreenYellow;
			static const Color Honeydew;
			static const Color HotPink;
			static const Color IndianRed;
			static const Color Indigo;
			static const Color Ivory;
			static const Color Khaki;
			static const Color Lavender;
			static const Color LavenderBlush;
			static const Color LawnGreen;
			static const Color LemonChiffon;
			static const Color LightBlue;
			static const Color LightCoral;
			static const Color LightCyan;
			static const Color LightGoldenrodYellow;
			static const Color LightGreen;
			static const Color LightGray;
			static const Color LightPink;
			static const Color LightSalmon;
			static const Color LightSeaGreen;
			static const Color LightSkyBlue;
			static const Color LightSlateGray;
			static const Color LightSteelBlue;
			static const Color LightYellow;
			static const Color Lime;
			static const Color LimeGreen;
			static const Color Linen;
			static const Color Magenta;
			static const Color Maroon;
			static const Color MediumAquamarine;
			static const Color MediumBlue;
			static const Color MediumOrchid;
			static const Color MediumPurple;
			static const Color MediumSeaGreen;
			static const Color MediumSlateBlue;
			static const Color MediumSpringGreen;
			static const Color MediumTurquoise;
			static const Color MediumVioletRed;
			static const Color MidnightBlue;
			static const Color MintCream;
			static const Color MistyRose;
			static const Color Moccasin;
			static const Color NavajoWhite;
			static const Color Navy;
			static const Color OldLace;
			static const Color Olive;
			static const Color OliveDrab;
			static const Color Orange;
			static const Color OrangeRed;
			static const Color Orchid;
			static const Color PaleGoldenrod;
			static const Color PaleGreen;
			static const Color PaleTurquoise;
			static const Color PaleVioletRed;
			static const Color PapayaWhip;
			static const Color PeachPuff;
			static const Color Peru;
			static const Color Pink;
			static const Color Plum;
			static const Color PowderBlue;
			static const Color Purple;
			static const Color Red;
			static const Color RosyBrown;
			static const Color RoyalBlue;
			static const Color SaddleBrown;
			static const Color Salmon;
			static const Color SandyBrown;
			static const Color SeaGreen;
			static const Color SeaShell;
			static const Color Sienna;
			static const Color Silver;
			static const Color SkyBlue;
			static const Color SlateBlue;
			static const Color SlateGray;
			static const Color Snow;
			static const Color SpringGreen;
			static const Color SteelBlue;
			static const Color Tan;
			static const Color Teal;
			static const Color Thistle;
			static const Color Tomato;
			static const Color Transparent;
			static const Color Turquoise;
			static const Color Violet;
			static const Color Wheat;
			static const Color White;
			static const Color WhiteSmoke;
			static const Color Yellow;
			static const Color YellowGreen;
		};

		// Binary operators
		Color operator+ (const Color& C1, const Color& C2);
		Color operator- (const Color& C1, const Color& C2);
		Color operator* (const Color& C1, const Color& C2);
		Color operator* (const Color& C, float S);
		Color operator/ (const Color& C1, const Color& C2);
		Color operator* (float S, const Color& C);
		
		//------------------------------------------------------------------------------
		// Rect
		class Rect : public RECT
		{
		public:
			Rect();
			Rect(long _left, long _top, long _right, long _bottom);
			Rect(const RECT& o);
			Rect(long width, long height);

			bool operator == (const Rect& rect) { return EqualRect(this, &rect) == TRUE; }

			long GetWidth() const { return right - left; }
			long GetHeight() const { return bottom - top; }

			void Move(long dx, long dy) { OffsetRect(this, dx, dy); }
			void Set(long _left, long _top, long _right, long _bottom)
			{
				left = _left;
				top = _top;
				right = _right;
				bottom = _bottom;
			}

			bool IsSameSize(const Rect& rect)
			{
				return GetWidth() == rect.GetWidth() && GetHeight() == rect.GetHeight();
			}

			bool IsInside(long x, long y)
			{
				POINT pt;
				pt.x = x;
				pt.y = y;

				return PtInRect(this, pt) == TRUE;
			}

			bool IsFitsIn(const Rect& rect)
			{
				if (GetWidth() < rect.GetWidth() || GetHeight() < rect.GetHeight())
					return false;

				return true;
			}
		};

		//------------------------------------------------------------------------------
		// Viewport
		class Viewport
		{
		public:
			float x;
			float y;
			float width;
			float height;
			float minDepth;
			float maxDepth;

			Viewport();
			Viewport(float ix, float iy, float iw, float ih, float iminz = 0.f, float imaxz = 1.f);
			explicit Viewport(const RECT& rct);

			// Comparison operators
			bool operator == (const Viewport& vp) const;
			bool operator != (const Viewport& vp) const;

			const D3D11_VIEWPORT* Get11() const;

			// Assignment operators
			Viewport& operator= (const Viewport& vp);
			Viewport& operator= (const Rect& rct);

			// Viewport operations
			float AspectRatio() const;

			float3 Project(const float3& p, const Matrix& proj, const Matrix& view, const Matrix& world) const;
			void Project(const float3& p, const Matrix& proj, const Matrix& view, const Matrix& world, float3& result) const;

			float3 Unproject(const float3& p, const Matrix& proj, const Matrix& view, const Matrix& world) const;
			void Unproject(const float3& p, const Matrix& proj, const Matrix& view, const Matrix& world, float3& result) const;
		};

		namespace bezier
		{
			// Performs a cubic bezier interpolation between four control points,
			// returning the value at the specified time (t ranges 0 to 1).
			// This template implementation can be used to interpolate XMVECTOR,
			// float, or any other types that define suitable * and + operators.
			template<typename T>
			T CubicInterpolate(T const& p1, T const& p2, T const& p3, T const& p4, float t)
			{
				return p1 * (1 - t) * (1 - t) * (1 - t) +
					p2 * 3 * t * (1 - t) * (1 - t) +
					p3 * 3 * t * t * (1 - t) +
					p4 * t * t * t;
			}


			// Computes the tangent of a cubic bezier curve at the specified time.
			// Template supports XMVECTOR, float, or any other types with * and + operators.
			template<typename T>
			T CubicTangent(T const& p1, T const& p2, T const& p3, T const& p4, float t)
			{
				return p1 * (-1 + 2 * t - t * t) +
					p2 * (1 - 4 * t + 3 * t * t) +
					p3 * (2 * t - 3 * t * t) +
					p4 * (t * t);
			}


			// Creates vertices for a patch that is tessellated at the specified level.
			// Calls the specified outputVertex function for each generated vertex,
			// passing the position, normal, and texture coordinate as parameters.
			void CreatePatchVertices(_In_reads_(16) float3 patch[16], size_t tessellation, bool isMirrored, std::function<void(const float3&, const float3&, const float3&)> outputVertex);


			// Creates indices for a patch that is tessellated at the specified level.
			// Calls the specified outputIndex function for each generated index value.
			template<typename TOutputFunc>
			void CreatePatchIndices(size_t tessellation, bool isMirrored, TOutputFunc outputIndex)
			{
				size_t stride = tessellation + 1;

				for (size_t i = 0; i < tessellation; i++)
				{
					for (size_t j = 0; j < tessellation; j++)
					{
						// Make a list of six index values (two triangles).
						std::array<size_t, 6> indices =
						{
							i * stride + j,
							(i + 1) * stride + j,
							(i + 1) * stride + j + 1,

							i * stride + j,
							(i + 1) * stride + j + 1,
							i * stride + j + 1,
						};

						// If this patch is mirrored, reverse indices to fix the winding order.
						if (isMirrored)
						{
							std::reverse(indices.begin(), indices.end());
						}

						// Output these index values.
						std::for_each(indices.begin(), indices.end(), outputIndex);
					}
				}
			}
		}

#include "Math.inl"
	}
}

//------------------------------------------------------------------------------
// Support for SimpleMath and Standard C++ Library containers
namespace std
{
	template<> struct less<est::math::float2>
	{
		bool operator()(const est::math::float2& V1, const est::math::float2& V2) const
		{
			return ((V1.x < V2.x) || ((V1.x == V2.x) && (V1.y < V2.y)));
		}
	};

	template<> struct less<est::math::float3>
	{
		bool operator()(const est::math::float3& V1, const est::math::float3& V2) const
		{
			return ((V1.x < V2.x)
				|| ((V1.x == V2.x) && (V1.y < V2.y))
				|| ((V1.x == V2.x) && (V1.y == V2.y) && (V1.z < V2.z)));
		}
	};

	template<> struct less<est::math::float4>
	{
		bool operator()(const est::math::float4& V1, const est::math::float4& V2) const
		{
			return ((V1.x < V2.x)
				|| ((V1.x == V2.x) && (V1.y < V2.y))
				|| ((V1.x == V2.x) && (V1.y == V2.y) && (V1.z < V2.z))
				|| ((V1.x == V2.x) && (V1.y == V2.y) && (V1.z == V2.z) && (V1.w < V2.w)));
		}
	};

	template<> struct less<est::math::Matrix>
	{
		bool operator()(const est::math::Matrix& M1, const est::math::Matrix& M2) const
		{
			if (M1._11 != M2._11) return M1._11 < M2._11;
			if (M1._12 != M2._12) return M1._12 < M2._12;
			if (M1._13 != M2._13) return M1._13 < M2._13;
			if (M1._14 != M2._14) return M1._14 < M2._14;
			if (M1._21 != M2._21) return M1._21 < M2._21;
			if (M1._22 != M2._22) return M1._22 < M2._22;
			if (M1._23 != M2._23) return M1._23 < M2._23;
			if (M1._24 != M2._24) return M1._24 < M2._24;
			if (M1._31 != M2._31) return M1._31 < M2._31;
			if (M1._32 != M2._32) return M1._32 < M2._32;
			if (M1._33 != M2._33) return M1._33 < M2._33;
			if (M1._34 != M2._34) return M1._34 < M2._34;
			if (M1._41 != M2._41) return M1._41 < M2._41;
			if (M1._42 != M2._42) return M1._42 < M2._42;
			if (M1._43 != M2._43) return M1._43 < M2._43;
			if (M1._44 != M2._44) return M1._44 < M2._44;

			return false;
		}
	};

	template<> struct less<est::math::Plane>
	{
		bool operator()(const est::math::Plane& P1, const est::math::Plane& P2) const
		{
			return ((P1.x < P2.x)
				|| ((P1.x == P2.x) && (P1.y < P2.y))
				|| ((P1.x == P2.x) && (P1.y == P2.y) && (P1.z < P2.z))
				|| ((P1.x == P2.x) && (P1.y == P2.y) && (P1.z == P2.z) && (P1.w < P2.w)));
		}
	};

	template<> struct less<est::math::Quaternion>
	{
		bool operator()(const est::math::Quaternion& Q1, const est::math::Quaternion& Q2) const
		{
			return ((Q1.x < Q2.x)
				|| ((Q1.x == Q2.x) && (Q1.y < Q2.y))
				|| ((Q1.x == Q2.x) && (Q1.y == Q2.y) && (Q1.z < Q2.z))
				|| ((Q1.x == Q2.x) && (Q1.y == Q2.y) && (Q1.z == Q2.z) && (Q1.w < Q2.w)));
		}
	};

	template<> struct less<est::math::Color>
	{
		bool operator()(const est::math::Color& C1, const est::math::Color& C2) const
		{
			return ((C1.r < C2.r)
				|| ((C1.r == C2.r) && (C1.g < C2.g))
				|| ((C1.r == C2.r) && (C1.g == C2.g) && (C1.b < C2.b))
				|| ((C1.r == C2.r) && (C1.g == C2.g) && (C1.b == C2.b) && (C1.a < C2.a)));
		}
	};

	template<> struct less<est::math::Viewport>
	{
		bool operator()(const est::math::Viewport& vp1, const est::math::Viewport& vp2) const
		{
			if (vp1.x != vp2.x) return (vp1.x < vp2.x);
			if (vp1.y != vp2.y) return (vp1.y < vp2.y);

			if (vp1.width != vp2.width) return (vp1.width < vp2.width);
			if (vp1.height != vp2.height) return (vp1.height < vp2.height);

			if (vp1.minDepth != vp2.minDepth) return (vp1.minDepth < vp2.minDepth);
			if (vp1.maxDepth != vp2.maxDepth) return (vp1.maxDepth < vp2.maxDepth);

			return false;
		}
	};
}