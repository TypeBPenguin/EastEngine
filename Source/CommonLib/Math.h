#pragma once

#include <xmmintrin.h>
#include <random>

struct D3D11_VIEWPORT;

namespace eastengine
{
	namespace math
	{
		const float PI = 3.1415926535897932384626433832795f;
		const float PI2 = 6.283185307179586476925286766559f;
		const float DIVPI = 0.31830988618379067153776752674503f;
		const float DIVPI2 = 0.15915494309189533576888376337251f;
		const float PIDIV2 = 1.5707963267948966192313216916398f;
		const float PIDIV4 = 0.78539816339744830961566084581988f;

		std::mt19937& mt19937();

		float ToRadians(float fDegrees);
		float ToDegrees(float fRadians);

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

		template <typename T>
		inline bool IsReallyZero(T value)
		{
			if (std::abs(value) <= std::numeric_limits<T>::epsilon())
				return true;

			return false;
		}

		template <typename T>
		inline bool IsReallySame(T value1, T value2)
		{
			return IsReallyZero(value1 - value2);
		}

		template <typename T>
		inline bool IsZero(T value)
		{
			if (value == 0)
				return true;

			return false;
		}

		template <>
		inline bool IsZero(float value)
		{
			if (::std::abs(value) <= 1e-05f)
				return true;

			return false;
		}

		template <>
		inline bool IsZero(double value)
		{
			if (::std::abs(value) <= 1e-10)
				return true;

			return false;
		}

		template <typename T>
		inline bool IsEqual(T value1, T value2)
		{
			return IsZero(value1 - value2);
		}

		template <typename T>
		__forceinline T DivideByMultiple(T value, size_t alignment)
		{
			return (T)((value + alignment - 1) / alignment);
		}

		template <typename T>
		T Random(T min = -RAND_MAX, T max = RAND_MAX);

		struct Vector2;
		struct Vector3;
		struct Vector4;
		struct Matrix;
		struct Quaternion;
		struct Plane;

		Vector2 CompressNormal(const Vector3& f3Normal);
		Vector3 DeCompressNormal(const Vector2& f2Normal);
		Vector3 CalcTangent(const Vector3& f3Normal);
		Vector3 CalcBinormal(const Vector3& f3Normal, const Vector3& f3Tangent);

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
				uint32_t v;
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

		struct Int2
		{
			int32_t x;
			int32_t y;

			Int2();
			explicit Int2(int x);
			Int2(int x, int y);
			Int2(const Int2& V);

			// Comparison operators
			bool operator == (const Int2& V) const;
			bool operator != (const Int2& V) const;

			// Assignment operators
			Int2& operator= (const Int2& V) { x = V.x; y = V.y; return *this; }
			Int2& operator+= (const Int2& V);
			Int2& operator-= (const Int2& V);
			Int2& operator*= (const Int2& V);
			Int2& operator*= (float S);
			Int2& operator/= (float S);

			// Unary operators
			Int2 operator+ () const { return *this; }
			Int2 operator- () const { return Int2(-x, -y); }

			static const Int2 Zero;
			static const Int2 One;
			static const Int2 UnitX;
			static const Int2 UnitY;
		};

		struct Int3
		{
			int32_t x;
			int32_t y;
			int32_t z;

			Int3();
			explicit Int3(int x);
			Int3(int x, int y, int z);
			Int3(const Int3& V);

			// Comparison operators
			bool operator == (const Int3& V) const;
			bool operator != (const Int3& V) const;

			// Assignment operators
			Int3& operator= (const Int3& V) { x = V.x; y = V.y; z = V.z; return *this; }
			Int3& operator+= (const Int3& V);
			Int3& operator-= (const Int3& V);
			Int3& operator*= (const Int3& V);
			Int3& operator*= (float S);
			Int3& operator/= (float S);

			// Unary operators
			Int3 operator+ () const { return *this; }
			Int3 operator- () const { return Int3(-x, -y, -z); }

			static const Int3 Zero;
			static const Int3 One;
			static const Int3 UnitX;
			static const Int3 UnitY;
			static const Int3 UnitZ;
		};

		struct Int4
		{
			int32_t x;
			int32_t y;
			int32_t z;
			int32_t w;

			Int4();
			explicit Int4(int x);
			Int4(int x, int y, int z, int w);
			Int4(const Int4& V);

			// Comparison operators
			bool operator == (const Int4& V) const;
			bool operator != (const Int4& V) const;

			// Assignment operators
			Int4& operator= (const Int4& V) { x = V.x; y = V.y; z = V.z; w = V.w; return *this; }
			Int4& operator+= (const Int4& V);
			Int4& operator-= (const Int4& V);
			Int4& operator*= (const Int4& V);
			Int4& operator*= (float S);
			Int4& operator/= (float S);

			// Unary operators
			Int4 operator+ () const { return *this; }
			Int4 operator- () const { return Int4(-x, -y, -z, -w); }

			static const Int4 Zero;
			static const Int4 One;
			static const Int4 UnitX;
			static const Int4 UnitY;
			static const Int4 UnitZ;
			static const Int4 UnitW;
		};

		struct UInt2
		{
			uint32_t x;
			uint32_t y;

			UInt2();
			explicit UInt2(uint32_t x);
			UInt2(uint32_t x, uint32_t y);
			UInt2(const UInt2& V);

			// Comparison operators
			bool operator == (const UInt2& V) const;
			bool operator != (const UInt2& V) const;

			// Assignment operators
			UInt2& operator= (const UInt2& V) { x = V.x; y = V.y; return *this; }
			UInt2& operator+= (const UInt2& V);
			UInt2& operator-= (const UInt2& V);
			UInt2& operator*= (const UInt2& V);
			UInt2& operator*= (float S);
			UInt2& operator/= (float S);

			// Unary operators
			UInt2 operator+ () const { return *this; }

			static const UInt2 Zero;
			static const UInt2 One;
			static const UInt2 UnitX;
			static const UInt2 UnitY;
		};

		struct UInt3
		{
			uint32_t x;
			uint32_t y;
			uint32_t z;

			UInt3();
			explicit UInt3(uint32_t x);
			UInt3(uint32_t x, uint32_t y, uint32_t z);
			UInt3(const UInt3& V);

			// Comparison operators
			bool operator == (const UInt3& V) const;
			bool operator != (const UInt3& V) const;

			// Assignment operators
			UInt3& operator= (const UInt3& V) { x = V.x; y = V.y; z = V.z; return *this; }
			UInt3& operator+= (const UInt3& V);
			UInt3& operator-= (const UInt3& V);
			UInt3& operator*= (const UInt3& V);
			UInt3& operator*= (float S);
			UInt3& operator/= (float S);

			// Unary operators
			UInt3 operator+ () const { return *this; }

			static const UInt3 Zero;
			static const UInt3 One;
			static const UInt3 UnitX;
			static const UInt3 UnitY;
			static const UInt3 UnitZ;
		};

		struct UInt4
		{
			uint32_t x;
			uint32_t y;
			uint32_t z;
			uint32_t w;

			UInt4();
			explicit UInt4(uint32_t x);
			UInt4(uint32_t x, uint32_t y, uint32_t z, uint32_t w);
			UInt4(const UInt4& V);

			// Comparison operators
			bool operator == (const UInt4& V) const;
			bool operator != (const UInt4& V) const;

			// Assignment operators
			UInt4& operator= (const UInt4& V) { x = V.x; y = V.y; z = V.z; w = V.w; return *this; }
			UInt4& operator+= (const UInt4& V);
			UInt4& operator-= (const UInt4& V);
			UInt4& operator*= (const UInt4& V);
			UInt4& operator*= (float S);
			UInt4& operator/= (float S);

			// Unary operators
			UInt4 operator+ () const { return *this; }

			static const UInt4 Zero;
			static const UInt4 One;
			static const UInt4 UnitX;
			static const UInt4 UnitY;
			static const UInt4 UnitZ;
			static const UInt4 UnitW;
		};

		//------------------------------------------------------------------------------
		// 2D vector
		struct Vector2
		{
			float x;
			float y;

			Vector2();
			explicit Vector2(float x);
			Vector2(float x, float y);
			explicit Vector2(_In_reads_(2) const float *pArray);
			Vector2(const __m128& V);
			Vector2(const Vector2& V);

			operator __m128() const;

			float operator [] (int nIndex) const { return reinterpret_cast<const float*>(this)[nIndex]; }

			// Comparison operators
			bool operator == (const Vector2& V) const;
			bool operator != (const Vector2& V) const;

			// Assignment operators
			Vector2& operator= (const Vector2& V) { x = V.x; y = V.y; return *this; }
			Vector2& operator+= (const Vector2& V);
			Vector2& operator-= (const Vector2& V);
			Vector2& operator*= (const Vector2& V);
			Vector2& operator*= (float S);
			Vector2& operator/= (float S);

			// Unary operators
			Vector2 operator+ () const { return *this; }
			Vector2 operator- () const { return Vector2(-x, -y); }

			// Vector operations
			bool InBounds(const Vector2& Bounds) const;

			float Length() const;
			float LengthSquared() const;

			float Dot(const Vector2& V) const;
			void Cross(const Vector2& V, Vector2& result) const;
			Vector2 Cross(const Vector2& V) const;

			void Normalize();
			void Normalize(Vector2& result) const;

			void Clamp(const Vector2& vmin, const Vector2& vmax);
			void Clamp(const Vector2& vmin, const Vector2& vmax, Vector2& result) const;

			// Static functions
			static float Distance(const Vector2& v1, const Vector2& v2);
			static float DistanceSquared(const Vector2& v1, const Vector2& v2);

			static void Min(const Vector2& v1, const Vector2& v2, Vector2& result);
			static Vector2 Min(const Vector2& v1, const Vector2& v2);

			static void Max(const Vector2& v1, const Vector2& v2, Vector2& result);
			static Vector2 Max(const Vector2& v1, const Vector2& v2);

			static void Lerp(const Vector2& v1, const Vector2& v2, float t, Vector2& result);
			static Vector2 Lerp(const Vector2& v1, const Vector2& v2, float t);

			static void SmoothStep(const Vector2& v1, const Vector2& v2, float t, Vector2& result);
			static Vector2 SmoothStep(const Vector2& v1, const Vector2& v2, float t);

			static void Barycentric(const Vector2& v1, const Vector2& v2, const Vector2& v3, float f, float g, Vector2& result);
			static Vector2 Barycentric(const Vector2& v1, const Vector2& v2, const Vector2& v3, float f, float g);

			static void CatmullRom(const Vector2& v1, const Vector2& v2, const Vector2& v3, const Vector2& v4, float t, Vector2& result);
			static Vector2 CatmullRom(const Vector2& v1, const Vector2& v2, const Vector2& v3, const Vector2& v4, float t);

			static void Hermite(const Vector2& v1, const Vector2& t1, const Vector2& v2, const Vector2& t2, float t, Vector2& result);
			static Vector2 Hermite(const Vector2& v1, const Vector2& t1, const Vector2& v2, const Vector2& t2, float t);

			static void Reflect(const Vector2& ivec, const Vector2& nvec, Vector2& result);
			static Vector2 Reflect(const Vector2& ivec, const Vector2& nvec);

			static void Refract(const Vector2& ivec, const Vector2& nvec, float refractionIndex, Vector2& result);
			static Vector2 Refract(const Vector2& ivec, const Vector2& nvec, float refractionIndex);

			static void Transform(const Vector2& v, const Quaternion& quat, Vector2& result);
			static Vector2 Transform(const Vector2& v, const Quaternion& quat);

			static void Transform(const Vector2& v, const Matrix& m, Vector2& result);
			static Vector2 Transform(const Vector2& v, const Matrix& m);
			static void Transform(_In_reads_(count) const Vector2* varray, size_t count, const Matrix& m, _Out_writes_(count) Vector2* resultArray);

			static void Transform(const Vector2& v, const Matrix& m, Vector4& result);
			static void Transform(_In_reads_(count) const Vector2* varray, size_t count, const Matrix& m, _Out_writes_(count) Vector4* resultArray);

			static void TransformNormal(const Vector2& v, const Matrix& m, Vector2& result);
			static Vector2 TransformNormal(const Vector2& v, const Matrix& m);
			static void TransformNormal(_In_reads_(count) const Vector2* varray, size_t count, const Matrix& m, _Out_writes_(count) Vector2* resultArray);

			// Constants
			static const Vector2 Zero;
			static const Vector2 One;
			static const Vector2 UnitX;
			static const Vector2 UnitY;
		};

		// Binary operators
		Vector2 operator+ (const Vector2& V1, const Vector2& V2);
		Vector2 operator- (const Vector2& V1, const Vector2& V2);
		Vector2 operator* (const Vector2& V1, const Vector2& V2);
		Vector2 operator* (const Vector2& V, float S);
		Vector2 operator/ (const Vector2& V1, const Vector2& V2);
		Vector2 operator/ (const Vector2& V1, float S);
		Vector2 operator* (float S, const Vector2& V);

		//------------------------------------------------------------------------------
		// 3D vector
		struct Vector3
		{
			float x;
			float y;
			float z;

			Vector3();
			explicit Vector3(float x);
			Vector3(float x, float y, float z);
			explicit Vector3(_In_reads_(3) const float *pArray);
			Vector3(const __m128& V);
			Vector3(const Vector3& V);

			operator __m128() const;

			float operator [] (int nIndex) const { return reinterpret_cast<const float*>(this)[nIndex]; }

			// Comparison operators
			bool operator == (const Vector3& V) const;
			bool operator != (const Vector3& V) const;

			// Assignment operators
			Vector3& operator= (const Vector3& V) { x = V.x; y = V.y; z = V.z; return *this; }
			Vector3& operator+= (const Vector3& V);
			Vector3& operator-= (const Vector3& V);
			Vector3& operator*= (const Vector3& V);
			Vector3& operator/= (const Vector3& V);
			Vector3& operator*= (float S);
			Vector3& operator/= (float S);

			// Unary operators
			Vector3 operator+ () const { return *this; }
			Vector3 operator- () const;

			// Vector operations
			bool InBounds(const Vector3& Bounds) const;

			float Length() const;
			float LengthSquared() const;

			float Dot(const Vector3& V) const;
			void Cross(const Vector3& V, Vector3& result) const;
			Vector3 Cross(const Vector3& V) const;

			void Normalize();
			void Normalize(Vector3& result) const;

			void Clamp(const Vector3& vmin, const Vector3& vmax);
			void Clamp(const Vector3& vmin, const Vector3& vmax, Vector3& result) const;

			// Static functions
			static float Distance(const Vector3& v1, const Vector3& v2);
			static float DistanceSquared(const Vector3& v1, const Vector3& v2);

			static void Min(const Vector3& v1, const Vector3& v2, Vector3& result);
			static Vector3 Min(const Vector3& v1, const Vector3& v2);

			static void Max(const Vector3& v1, const Vector3& v2, Vector3& result);
			static Vector3 Max(const Vector3& v1, const Vector3& v2);

			static void Lerp(const Vector3& v1, const Vector3& v2, float t, Vector3& result);
			static Vector3 Lerp(const Vector3& v1, const Vector3& v2, float t);

			static void SmoothStep(const Vector3& v1, const Vector3& v2, float t, Vector3& result);
			static Vector3 SmoothStep(const Vector3& v1, const Vector3& v2, float t);

			static void Barycentric(const Vector3& v1, const Vector3& v2, const Vector3& v3, float f, float g, Vector3& result);
			static Vector3 Barycentric(const Vector3& v1, const Vector3& v2, const Vector3& v3, float f, float g);

			static void CatmullRom(const Vector3& v1, const Vector3& v2, const Vector3& v3, const Vector3& v4, float t, Vector3& result);
			static Vector3 CatmullRom(const Vector3& v1, const Vector3& v2, const Vector3& v3, const Vector3& v4, float t);

			static void Hermite(const Vector3& v1, const Vector3& t1, const Vector3& v2, const Vector3& t2, float t, Vector3& result);
			static Vector3 Hermite(const Vector3& v1, const Vector3& t1, const Vector3& v2, const Vector3& t2, float t);

			static void Reflect(const Vector3& ivec, const Vector3& nvec, Vector3& result);
			static Vector3 Reflect(const Vector3& ivec, const Vector3& nvec);

			static void Refract(const Vector3& ivec, const Vector3& nvec, float refractionIndex, Vector3& result);
			static Vector3 Refract(const Vector3& ivec, const Vector3& nvec, float refractionIndex);

			static void Transform(const Vector3& v, const Quaternion& quat, Vector3& result);
			static Vector3 Transform(const Vector3& v, const Quaternion& quat);

			static void Transform(const Vector3& v, const Matrix& m, Vector3& result);
			static Vector3 Transform(const Vector3& v, const Matrix& m);
			static void Transform(_In_reads_(count) const Vector3* varray, size_t count, const Matrix& m, _Out_writes_(count) Vector3* resultArray);

			static void Transform(const Vector3& v, const Matrix& m, Vector4& result);
			static void Transform(_In_reads_(count) const Vector3* varray, size_t count, const Matrix& m, _Out_writes_(count) Vector4* resultArray);

			static void TransformNormal(const Vector3& v, const Matrix& m, Vector3& result);
			static Vector3 TransformNormal(const Vector3& v, const Matrix& m);
			static void TransformNormal(_In_reads_(count) const Vector3* varray, size_t count, const Matrix& m, _Out_writes_(count) Vector3* resultArray);

			static Vector3 FresnelTerm(const Vector3& v1, const Vector3& v2);

			// Constants
			static const Vector3 Zero;
			static const Vector3 One;
			static const Vector3 UnitX;
			static const Vector3 UnitY;
			static const Vector3 UnitZ;
			static const Vector3 Up;
			static const Vector3 Down;
			static const Vector3 Right;
			static const Vector3 Left;
			static const Vector3 Forward;
			static const Vector3 Backward;
		};

		// Binary operators
		Vector3 operator+ (const Vector3& V1, const Vector3& V2);
		Vector3 operator- (const Vector3& V1, const Vector3& V2);
		Vector3 operator* (const Vector3& V1, const Vector3& V2);
		Vector3 operator* (const Vector3& V, float S);
		Vector3 operator/ (const Vector3& V1, const Vector3& V2);
		Vector3 operator/ (const Vector3& V, float S);
		Vector3 operator* (float S, const Vector3& V);

		//------------------------------------------------------------------------------
		// 4D vector
		struct Vector4
		{
			float x;
			float y;
			float z;
			float w;

			Vector4();
			explicit Vector4(float x);
			Vector4(float x, float y, float z, float w);
			explicit Vector4(_In_reads_(4) const float *pArray);
			Vector4(const __m128& V);
			Vector4(const Vector4& V);

			operator __m128() const;

			float operator [] (int nIndex) const { return reinterpret_cast<const float*>(this)[nIndex]; }

			// Comparison operators
			bool operator == (const Vector4& V) const;
			bool operator != (const Vector4& V) const;

			// Assignment operators
			Vector4& operator= (const Vector4& V) { x = V.x; y = V.y; z = V.z; w = V.w; return *this; }
			Vector4& operator+= (const Vector4& V);
			Vector4& operator-= (const Vector4& V);
			Vector4& operator*= (const Vector4& V);
			Vector4& operator*= (float S);
			Vector4& operator/= (float S);

			// Unary operators
			Vector4 operator+ () const { return *this; }
			Vector4 operator- () const;

			// Vector operations
			bool InBounds(const Vector4& Bounds) const;

			float Length() const;
			float LengthSquared() const;

			float Dot(const Vector4& V) const;
			void Cross(const Vector4& v1, const Vector4& v2, Vector4& result) const;
			Vector4 Cross(const Vector4& v1, const Vector4& v2) const;

			void Normalize();
			void Normalize(Vector4& result) const;

			void Clamp(const Vector4& vmin, const Vector4& vmax);
			void Clamp(const Vector4& vmin, const Vector4& vmax, Vector4& result) const;

			// Static functions
			static float Distance(const Vector4& v1, const Vector4& v2);
			static float DistanceSquared(const Vector4& v1, const Vector4& v2);

			static void Min(const Vector4& v1, const Vector4& v2, Vector4& result);
			static Vector4 Min(const Vector4& v1, const Vector4& v2);

			static void Max(const Vector4& v1, const Vector4& v2, Vector4& result);
			static Vector4 Max(const Vector4& v1, const Vector4& v2);

			static void Lerp(const Vector4& v1, const Vector4& v2, float t, Vector4& result);
			static Vector4 Lerp(const Vector4& v1, const Vector4& v2, float t);

			static void SmoothStep(const Vector4& v1, const Vector4& v2, float t, Vector4& result);
			static Vector4 SmoothStep(const Vector4& v1, const Vector4& v2, float t);

			static void Barycentric(const Vector4& v1, const Vector4& v2, const Vector4& v3, float f, float g, Vector4& result);
			static Vector4 Barycentric(const Vector4& v1, const Vector4& v2, const Vector4& v3, float f, float g);

			static void CatmullRom(const Vector4& v1, const Vector4& v2, const Vector4& v3, const Vector4& v4, float t, Vector4& result);
			static Vector4 CatmullRom(const Vector4& v1, const Vector4& v2, const Vector4& v3, const Vector4& v4, float t);

			static void Hermite(const Vector4& v1, const Vector4& t1, const Vector4& v2, const Vector4& t2, float t, Vector4& result);
			static Vector4 Hermite(const Vector4& v1, const Vector4& t1, const Vector4& v2, const Vector4& t2, float t);

			static void Reflect(const Vector4& ivec, const Vector4& nvec, Vector4& result);
			static Vector4 Reflect(const Vector4& ivec, const Vector4& nvec);

			static void Refract(const Vector4& ivec, const Vector4& nvec, float refractionIndex, Vector4& result);
			static Vector4 Refract(const Vector4& ivec, const Vector4& nvec, float refractionIndex);

			static void Transform(const Vector2& v, const Quaternion& quat, Vector4& result);
			static Vector4 Transform(const Vector2& v, const Quaternion& quat);

			static void Transform(const Vector3& v, const Quaternion& quat, Vector4& result);
			static Vector4 Transform(const Vector3& v, const Quaternion& quat);

			static void Transform(const Vector4& v, const Quaternion& quat, Vector4& result);
			static Vector4 Transform(const Vector4& v, const Quaternion& quat);

			static void Transform(const Vector4& v, const Matrix& m, Vector4& result);
			static Vector4 Transform(const Vector4& v, const Matrix& m);
			static void Transform(_In_reads_(count) const Vector4* varray, size_t count, const Matrix& m, _Out_writes_(count) Vector4* resultArray);

			// Constants
			static const Vector4 Zero;
			static const Vector4 One;
			static const Vector4 UnitX;
			static const Vector4 UnitY;
			static const Vector4 UnitZ;
			static const Vector4 UnitW;
		};

		// Binary operators
		Vector4 operator+ (const Vector4& V1, const Vector4& V2);
		Vector4 operator- (const Vector4& V1, const Vector4& V2);
		Vector4 operator* (const Vector4& V1, const Vector4& V2);
		Vector4 operator* (const Vector4& V, float S);
		Vector4 operator/ (const Vector4& V1, const Vector4& V2);
		Vector4 operator* (float S, const Vector4& V);

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
				float m[4][4];
			};

			Matrix();
			Matrix(float m00, float m01, float m02, float m03,
				float m10, float m11, float m12, float m13,
				float m20, float m21, float m22, float m23,
				float m30, float m31, float m32, float m33);
			explicit Matrix(const Vector3& r0, const Vector3& r1, const Vector3& r2);
			explicit Matrix(const Vector4& r0, const Vector4& r1, const Vector4& r2, const Vector4& r3);
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
			Vector3 Up() const { return Vector3(_12, _22, _32); }
			void Up(const Vector3& v) { _12 = v.x; _22 = v.y; _32 = v.z; }

			Vector3 Down() const { return Vector3(-_12, -_22, -_32); }
			void Down(const Vector3& v) { _12 = -v.x; _22 = -v.y; _32 = -v.z; }

			Vector3 Right() const { return Vector3(_11, _21, _31); }
			void Right(const Vector3& v) { _11 = v.x; _21 = v.y; _31 = v.z; }

			Vector3 Left() const { return Vector3(-_11, -_21, -_31); }
			void Left(const Vector3& v) { _11 = -v.x; _21 = -v.y; _31 = -v.z; }

			Vector3 Forward() const { return Vector3(_13, _23, _33); }
			void Forward(const Vector3& v) { _13 = v.x; _23 = v.y; _33 = v.z; }

			Vector3 Backward() const { return Vector3(-_13, -_23, -_33); }
			void Backward(const Vector3& v) { _13 = -v.x; _23 = -v.y; _33 = -v.z; }

			Vector3 Translation() const { return Vector3(_41, _42, _43); }
			void Translation(const Vector3& v) { _41 = v.x; _42 = v.y; _43 = v.z; }

			// Matrix operations
			bool Decompose(Vector3& scale, Quaternion& rotation, Vector3& translation) const;

			Matrix Transpose() const;
			void Transpose(Matrix& result) const;

			Matrix Invert() const;
			void Invert(Matrix& result) const;

			float Determinant() const;

			// Static functions
			static Matrix Compose(const Vector3& scale, const Quaternion& rotation, const Vector3& translation);
			static void Compose(const Vector3& scale, const Quaternion& rotation, const Vector3& translation, Matrix& result);

			static Matrix CreateBillboard(const Vector3& object, const Vector3& cameraPosition, const Vector3& cameraUp, _In_opt_ const Vector3* cameraForward = nullptr);

			static Matrix CreateConstrainedBillboard(const Vector3& object, const Vector3& cameraPosition, const Vector3& rotateAxis,
				_In_opt_ const Vector3* cameraForward = nullptr, _In_opt_ const Vector3* objectForward = nullptr);

			static Matrix CreateTranslation(const Vector3& position);
			static Matrix CreateTranslation(float x, float y, float z);

			static Matrix CreateScale(const Vector3& scales);
			static Matrix CreateScale(float xs, float ys, float zs);
			static Matrix CreateScale(float scale);

			static Matrix CreateRotationX(float radians);
			static Matrix CreateRotationY(float radians);
			static Matrix CreateRotationZ(float radians);

			static Matrix CreateFromAxisAngle(const Vector3& axis, float angle);

			static Matrix CreatePerspectiveFieldOfView(float fov, float aspectRatio, float nearPlane, float farPlane);
			static Matrix CreatePerspective(float width, float height, float nearPlane, float farPlane);
			static Matrix CreatePerspectiveOffCenter(float left, float right, float bottom, float top, float nearPlane, float farPlane);
			static Matrix CreateOrthographic(float width, float height, float zNearPlane, float zFarPlane);
			static Matrix CreateOrthographicOffCenter(float left, float right, float bottom, float top, float zNearPlane, float zFarPlane);

			static Matrix CreateLookAt(const Vector3& position, const Vector3& target, const Vector3& up);
			static Matrix CreateWorld(const Vector3& position, const Vector3& forward, const Vector3& up);

			static Matrix CreateFromQuaternion(const Quaternion& quat);

			static Matrix CreateFromYawPitchRoll(float yaw, float pitch, float roll);

			static Matrix CreateShadow(const Vector3& lightDir, const Plane& plane);

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
			float x;
			float y;
			float z;
			float w;

			Plane();
			Plane(float x, float y, float z, float w);
			Plane(const Vector3& normal, float d);
			Plane(const Vector3& point1, const Vector3& point2, const Vector3& point3);
			Plane(const Vector3& point, const Vector3& normal);
			explicit Plane(const Vector4& v);
			explicit Plane(_In_reads_(4) const float *pArray);
			Plane(const __m128& V);

			operator __m128() const;

			// Comparison operators
			bool operator == (const Plane& p) const;
			bool operator != (const Plane& p) const;

			// Assignment operators
			Plane& operator= (const Plane& p) { x = p.x; y = p.y; z = p.z; w = p.w; return *this; }

			// Properties
			Vector3 Normal() const { return Vector3(x, y, z); }
			void Normal(const Vector3& normal) { x = normal.x; y = normal.y; z = normal.z; }

			float D() const { return w; }
			void D(float d) { w = d; }

			// Plane operations
			void Normalize();
			void Normalize(Plane& result) const;

			float Dot(const Vector4& v) const;
			float DotCoordinate(const Vector3& position) const;
			float DotNormal(const Vector3& normal) const;

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
			float x;
			float y;
			float z;
			float w;

			Quaternion();
			Quaternion(float x, float y, float z, float w);
			Quaternion(const Vector3& v, float scalar);
			explicit Quaternion(const Vector4& v);
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

			Vector3 ToEularRadians() const;
			Vector3 ToEularDegrees() const;

			// Static functions
			static Quaternion CreateFromAxisAngle(const Vector3& axis, float angle);
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
			math::Vector3 scale;
			math::Quaternion rotation;
			math::Vector3 position;

			Transform();
			Transform(const Vector3& scale, const Quaternion& rotation, const Vector3& position);
			Transform(const Matrix& matrix);

			Matrix Compose() const;
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
				uint32_t c;
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
			float r;
			float g;
			float b;
			float a;

			Color();
			Color(float r, float g, float b);
			Color(float r, float g, float b, float a);
			explicit Color(const Vector3& clr);
			explicit Color(const Vector4& clr);
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
			Color& operator= (const Vector4& c) { r = c.x; g = c.y; b = c.z; a = c.w; return *this; }
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

			Vector3 ToVector3() const { return Vector3(r, g, b); }
			Vector4 ToVector4() const { return Vector4(r, g, b, a); }

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

			Vector3 Project(const Vector3& p, const Matrix& proj, const Matrix& view, const Matrix& world) const;
			void Project(const Vector3& p, const Matrix& proj, const Matrix& view, const Matrix& world, Vector3& result) const;

			Vector3 Unproject(const Vector3& p, const Matrix& proj, const Matrix& view, const Matrix& world) const;
			void Unproject(const Vector3& p, const Matrix& proj, const Matrix& view, const Matrix& world, Vector3& result) const;
		};

#include "Math.inl"
	}
}

//------------------------------------------------------------------------------
// Support for SimpleMath and Standard C++ Library containers
namespace std
{
	template<> struct less<eastengine::math::Vector2>
	{
		bool operator()(const eastengine::math::Vector2& V1, const eastengine::math::Vector2& V2) const
		{
			return ((V1.x < V2.x) || ((V1.x == V2.x) && (V1.y < V2.y)));
		}
	};

	template<> struct less<eastengine::math::Vector3>
	{
		bool operator()(const eastengine::math::Vector3& V1, const eastengine::math::Vector3& V2) const
		{
			return ((V1.x < V2.x)
				|| ((V1.x == V2.x) && (V1.y < V2.y))
				|| ((V1.x == V2.x) && (V1.y == V2.y) && (V1.z < V2.z)));
		}
	};

	template<> struct less<eastengine::math::Vector4>
	{
		bool operator()(const eastengine::math::Vector4& V1, const eastengine::math::Vector4& V2) const
		{
			return ((V1.x < V2.x)
				|| ((V1.x == V2.x) && (V1.y < V2.y))
				|| ((V1.x == V2.x) && (V1.y == V2.y) && (V1.z < V2.z))
				|| ((V1.x == V2.x) && (V1.y == V2.y) && (V1.z == V2.z) && (V1.w < V2.w)));
		}
	};

	template<> struct less<eastengine::math::Matrix>
	{
		bool operator()(const eastengine::math::Matrix& M1, const eastengine::math::Matrix& M2) const
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

	template<> struct less<eastengine::math::Plane>
	{
		bool operator()(const eastengine::math::Plane& P1, const eastengine::math::Plane& P2) const
		{
			return ((P1.x < P2.x)
				|| ((P1.x == P2.x) && (P1.y < P2.y))
				|| ((P1.x == P2.x) && (P1.y == P2.y) && (P1.z < P2.z))
				|| ((P1.x == P2.x) && (P1.y == P2.y) && (P1.z == P2.z) && (P1.w < P2.w)));
		}
	};

	template<> struct less<eastengine::math::Quaternion>
	{
		bool operator()(const eastengine::math::Quaternion& Q1, const eastengine::math::Quaternion& Q2) const
		{
			return ((Q1.x < Q2.x)
				|| ((Q1.x == Q2.x) && (Q1.y < Q2.y))
				|| ((Q1.x == Q2.x) && (Q1.y == Q2.y) && (Q1.z < Q2.z))
				|| ((Q1.x == Q2.x) && (Q1.y == Q2.y) && (Q1.z == Q2.z) && (Q1.w < Q2.w)));
		}
	};

	template<> struct less<eastengine::math::Color>
	{
		bool operator()(const eastengine::math::Color& C1, const eastengine::math::Color& C2) const
		{
			return ((C1.r < C2.r)
				|| ((C1.r == C2.r) && (C1.g < C2.g))
				|| ((C1.r == C2.r) && (C1.g == C2.g) && (C1.b < C2.b))
				|| ((C1.r == C2.r) && (C1.g == C2.g) && (C1.b == C2.b) && (C1.a < C2.a)));
		}
	};

	template<> struct less<eastengine::math::Viewport>
	{
		bool operator()(const eastengine::math::Viewport& vp1, const eastengine::math::Viewport& vp2) const
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