#pragma once

/****************************************************************************
*
* int2
*
****************************************************************************/

//------------------------------------------------------------------------------
// Comparision operators
//------------------------------------------------------------------------------

inline bool int2::operator == (const int2& V) const
{
	return this->x == V.x && this->y == V.y;
}

inline bool int2::operator != (const int2& V) const
{
	return (this->x == V.x && this->y == V.y) == false;
}

//------------------------------------------------------------------------------
// Assignment operators
//------------------------------------------------------------------------------

inline int2& int2::operator+= (const int2& V)
{
	this->x += V.x;
	this->y += V.y;

	return *this;
}

inline int2& int2::operator-= (const int2& V)
{
	this->x -= V.x;
	this->y -= V.y;

	return *this;
}

inline int2& int2::operator*= (const int2& V)
{
	this->x *= V.x;
	this->y *= V.y;

	return *this;
}

inline int2& int2::operator*= (float S)
{
	this->x = static_cast<int>(this->x * S);
	this->y = static_cast<int>(this->y * S);

	return *this;
}

inline int2& int2::operator/= (float S)
{
	assert(S != 0.0f);

	this->x = static_cast<int>(this->x / S);
	this->y = static_cast<int>(this->y / S);

	return *this;
}

//------------------------------------------------------------------------------
// Binary operators
//------------------------------------------------------------------------------

inline int2 operator+ (const int2& V1, const int2& V2)
{
	return int2(V1.x + V2.x, V1.y + V2.y);
}

inline int2 operator- (const int2& V1, const int2& V2)
{
	return int2(V1.x - V2.x, V1.y - V2.y);
}

inline int2 operator* (const int2& V1, const int2& V2)
{
	return int2(V1.x * V2.x, V1.y * V2.y);
}

inline int2 operator* (const int2& V, float S)
{
	return int2(static_cast<int>(V.x * S), static_cast<int>(V.y * S));
}

inline int2 operator/ (const int2& V1, const int2& V2)
{
	return int2(V1.x / V2.x, V1.y / V2.y);
}

inline int2 operator* (float S, const int2& V)
{
	return int2(static_cast<int>(V.x * S), static_cast<int>(V.y * S));
}

/****************************************************************************
*
* int3
*
****************************************************************************/

//------------------------------------------------------------------------------
// Comparision operators
//------------------------------------------------------------------------------

inline bool int3::operator == (const int3& V) const
{
	return this->x == V.x && this->y == V.y && this->z == V.z;
}

inline bool int3::operator != (const int3& V) const
{
	return (this->x == V.x && this->y == V.y && this->z == V.z) == false;
}

//------------------------------------------------------------------------------
// Assignment operators
//------------------------------------------------------------------------------

inline int3& int3::operator+= (const int3& V)
{
	this->x += V.x;
	this->y += V.y;
	this->z += V.z;

	return *this;
}

inline int3& int3::operator-= (const int3& V)
{
	this->x -= V.x;
	this->y -= V.y;
	this->z -= V.z;

	return *this;
}

inline int3& int3::operator*= (const int3& V)
{
	this->x *= V.x;
	this->y *= V.y;
	this->z *= V.z;

	return *this;
}

inline int3& int3::operator*= (float S)
{
	this->x = static_cast<int>(this->x / S);
	this->y = static_cast<int>(this->y / S);
	this->z = static_cast<int>(this->z / S);

	return *this;
}

inline int3& int3::operator/= (float S)
{
	assert(S != 0.0f);

	this->x = static_cast<int>(this->x / S);
	this->y = static_cast<int>(this->y / S);
	this->z = static_cast<int>(this->z / S);

	return *this;
}

//------------------------------------------------------------------------------
// Binary operators
//------------------------------------------------------------------------------

inline int3 operator+ (const int3& V1, const int3& V2)
{
	return int3(V1.x + V2.x, V1.y + V2.y, V1.z + V2.z);
}

inline int3 operator- (const int3& V1, const int3& V2)
{
	return int3(V1.x - V2.x, V1.y - V2.y, V1.z - V2.z);
}

inline int3 operator* (const int3& V1, const int3& V2)
{
	return int3(V1.x * V2.x, V1.y * V2.y, V1.z * V2.z);
}

inline int3 operator* (const int3& V, float S)
{
	return int3(static_cast<int>(V.x * S), static_cast<int>(V.y * S), static_cast<int>(V.z * S));
}

inline int3 operator/ (const int3& V1, const int3& V2)
{
	return int3(V1.x / V2.x, V1.y / V2.y, V1.z / V2.z);
}

inline int3 operator* (float S, const int3& V)
{
	return int3(static_cast<int>(V.x * S), static_cast<int>(V.y * S), static_cast<int>(V.z * S));
}

/****************************************************************************
*
* int4
*
****************************************************************************/

//------------------------------------------------------------------------------
// Comparision operators
//------------------------------------------------------------------------------

inline bool int4::operator == (const int4& V) const
{
	return this->x == V.x && this->y == V.y && this->z == V.z && this->w == V.w;
}

inline bool int4::operator != (const int4& V) const
{
	return (this->x == V.x && this->y == V.y && this->z == V.z && this->w == V.w) == false;
}

//------------------------------------------------------------------------------
// Assignment operators
//------------------------------------------------------------------------------

inline int4& int4::operator+= (const int4& V)
{
	this->x += V.x;
	this->y += V.y;
	this->z += V.z;
	this->w += V.w;

	return *this;
}

inline int4& int4::operator-= (const int4& V)
{
	this->x -= V.x;
	this->y -= V.y;
	this->z -= V.z;
	this->w -= V.w;

	return *this;
}

inline int4& int4::operator*= (const int4& V)
{
	this->x *= V.x;
	this->y *= V.y;
	this->z *= V.z;
	this->w *= V.w;

	return *this;
}

inline int4& int4::operator*= (float S)
{
	this->x = static_cast<int>(this->x * S);
	this->y = static_cast<int>(this->y * S);
	this->z = static_cast<int>(this->z * S);
	this->w = static_cast<int>(this->w * S);

	return *this;
}

inline int4& int4::operator/= (float S)
{
	assert(S != 0.0f);

	this->x = static_cast<int>(this->x / S);
	this->y = static_cast<int>(this->y / S);
	this->z = static_cast<int>(this->z / S);
	this->w = static_cast<int>(this->w / S);

	return *this;
}

//------------------------------------------------------------------------------
// Binary operators
//------------------------------------------------------------------------------

inline int4 operator+ (const int4& V1, const int4& V2)
{
	return int4(V1.x + V2.x, V1.y + V2.y, V1.z + V2.z, V1.w + V2.w);
}

inline int4 operator- (const int4& V1, const int4& V2)
{
	return int4(V1.x - V2.x, V1.y - V2.y, V1.z - V2.z, V1.w - V2.w);
}

inline int4 operator* (const int4& V1, const int4& V2)
{
	return int4(V1.x * V2.x, V1.y * V2.y, V1.z * V2.z, V1.w * V2.w);
}

inline int4 operator* (const int4& V, float S)
{
	return int4(static_cast<int>(V.x * S), static_cast<int>(V.y * S), static_cast<int>(V.z * S), static_cast<int>(V.w * S));
}

inline int4 operator/ (const int4& V1, const int4& V2)
{
	return int4(V1.x / V2.x, V1.y / V2.y, V1.z / V2.z, V1.w / V2.w);
}

inline int4 operator* (float S, const int4& V)
{
	return int4(static_cast<int>(V.x * S), static_cast<int>(V.y * S), static_cast<int>(V.z * S), static_cast<int>(V.w * S));
}

/****************************************************************************
*
* uint2
*
****************************************************************************/

//------------------------------------------------------------------------------
// Comparision operators
//------------------------------------------------------------------------------

inline bool uint2::operator == (const uint2& V) const
{
	return this->x == V.x && this->y == V.y;
}

inline bool uint2::operator != (const uint2& V) const
{
	return (this->x == V.x && this->y == V.y) == false;
}

//------------------------------------------------------------------------------
// Assignment operators
//------------------------------------------------------------------------------

inline uint2& uint2::operator+= (const uint2& V)
{
	this->x += V.x;
	this->y += V.y;

	return *this;
}

inline uint2& uint2::operator-= (const uint2& V)
{
	this->x -= V.x;
	this->y -= V.y;

	return *this;
}

inline uint2& uint2::operator*= (const uint2& V)
{
	this->x *= V.x;
	this->y *= V.y;

	return *this;
}

inline uint2& uint2::operator*= (float S)
{
	this->x = static_cast<uint32_t>(this->x * S);
	this->y = static_cast<uint32_t>(this->y * S);

	return *this;
}

inline uint2& uint2::operator/= (float S)
{
	assert(S != 0.0f);

	this->x = static_cast<uint32_t>(this->x / S);
	this->y = static_cast<uint32_t>(this->y / S);

	return *this;
}

//------------------------------------------------------------------------------
// Binary operators
//------------------------------------------------------------------------------

inline uint2 operator+ (const uint2& V1, const uint2& V2)
{
	return uint2(V1.x + V2.x, V1.y + V2.y);
}

inline uint2 operator- (const uint2& V1, const uint2& V2)
{
	return uint2(V1.x - V2.x, V1.y - V2.y);
}

inline uint2 operator* (const uint2& V1, const uint2& V2)
{
	return uint2(V1.x * V2.x, V1.y * V2.y);
}

inline uint2 operator* (const uint2& V, float S)
{
	return uint2(static_cast<uint32_t>(V.x * S), static_cast<uint32_t>(V.y * S));
}

inline uint2 operator/ (const uint2& V1, const uint2& V2)
{
	return uint2(V1.x / V2.x, V1.y / V2.y);
}

inline uint2 operator* (float S, const uint2& V)
{
	return uint2(static_cast<uint32_t>(V.x * S), static_cast<uint32_t>(V.y * S));
}

/****************************************************************************
*
* uint3
*
****************************************************************************/

//------------------------------------------------------------------------------
// Comparision operators
//------------------------------------------------------------------------------

inline bool uint3::operator == (const uint3& V) const
{
	return this->x == V.x && this->y == V.y && this->z == V.z;
}

inline bool uint3::operator != (const uint3& V) const
{
	return (this->x == V.x && this->y == V.y && this->z == V.z) == false;
}

//------------------------------------------------------------------------------
// Assignment operators
//------------------------------------------------------------------------------

inline uint3& uint3::operator+= (const uint3& V)
{
	this->x += V.x;
	this->y += V.y;
	this->z += V.z;

	return *this;
}

inline uint3& uint3::operator-= (const uint3& V)
{
	this->x -= V.x;
	this->y -= V.y;
	this->z -= V.z;

	return *this;
}

inline uint3& uint3::operator*= (const uint3& V)
{
	this->x *= V.x;
	this->y *= V.y;
	this->z *= V.z;

	return *this;
}

inline uint3& uint3::operator*= (float S)
{
	this->x = static_cast<uint32_t>(this->x * S);
	this->y = static_cast<uint32_t>(this->y * S);
	this->z = static_cast<uint32_t>(this->z * S);

	return *this;
}

inline uint3& uint3::operator/= (float S)
{
	assert(S != 0.0f);

	this->x = static_cast<uint32_t>(this->x / S);
	this->y = static_cast<uint32_t>(this->y / S);
	this->z = static_cast<uint32_t>(this->z / S);

	return *this;
}

//------------------------------------------------------------------------------
// Binary operators
//------------------------------------------------------------------------------

inline uint3 operator+ (const uint3& V1, const uint3& V2)
{
	return uint3(V1.x + V2.x, V1.y + V2.y, V1.z + V2.z);
}

inline uint3 operator- (const uint3& V1, const uint3& V2)
{
	return uint3(V1.x - V2.x, V1.y - V2.y, V1.z - V2.z);
}

inline uint3 operator* (const uint3& V1, const uint3& V2)
{
	return uint3(V1.x * V2.x, V1.y * V2.y, V1.z * V2.z);
}

inline uint3 operator* (const uint3& V, float S)
{
	return uint3(static_cast<uint32_t>(V.x * S), static_cast<uint32_t>(V.y * S), static_cast<uint32_t>(V.z * S));
}

inline uint3 operator/ (const uint3& V1, const uint3& V2)
{
	return uint3(V1.x / V2.x, V1.y / V2.y, V1.z / V2.z);
}

inline uint3 operator* (float S, const uint3& V)
{
	return uint3(static_cast<uint32_t>(V.x * S), static_cast<uint32_t>(V.y * S), static_cast<uint32_t>(V.z * S));
}

/****************************************************************************
*
* uint4
*
****************************************************************************/

//------------------------------------------------------------------------------
// Comparision operators
//------------------------------------------------------------------------------

inline bool uint4::operator == (const uint4& V) const
{
	return this->x == V.x && this->y == V.y && this->z == V.z && this->w == V.w;
}

inline bool uint4::operator != (const uint4& V) const
{
	return (this->x == V.x && this->y == V.y && this->z == V.z && this->w == V.w) == false;
}

//------------------------------------------------------------------------------
// Assignment operators
//------------------------------------------------------------------------------

inline uint4& uint4::operator+= (const uint4& V)
{
	this->x += V.x;
	this->y += V.y;
	this->z += V.z;
	this->w += V.w;

	return *this;
}

inline uint4& uint4::operator-= (const uint4& V)
{
	this->x -= V.x;
	this->y -= V.y;
	this->z -= V.z;
	this->w -= V.w;

	return *this;
}

inline uint4& uint4::operator*= (const uint4& V)
{
	this->x *= V.x;
	this->y *= V.y;
	this->z *= V.z;
	this->w *= V.w;

	return *this;
}

inline uint4& uint4::operator*= (float S)
{
	this->x = static_cast<uint32_t>(this->x * S);
	this->y = static_cast<uint32_t>(this->y * S);
	this->z = static_cast<uint32_t>(this->z * S);
	this->w = static_cast<uint32_t>(this->w * S);

	return *this;
}

inline uint4& uint4::operator/= (float S)
{
	assert(S != 0.0f);

	this->x = static_cast<uint32_t>(this->x / S);
	this->y = static_cast<uint32_t>(this->y / S);
	this->z = static_cast<uint32_t>(this->z / S);
	this->w = static_cast<uint32_t>(this->w / S);

	return *this;
}

//------------------------------------------------------------------------------
// Binary operators
//------------------------------------------------------------------------------

inline uint4 operator+ (const uint4& V1, const uint4& V2)
{
	return uint4(V1.x + V2.x, V1.y + V2.y, V1.z + V2.z, V1.w + V2.w);
}

inline uint4 operator- (const uint4& V1, const uint4& V2)
{
	return uint4(V1.x - V2.x, V1.y - V2.y, V1.z - V2.z, V1.w - V2.w);
}

inline uint4 operator* (const uint4& V1, const uint4& V2)
{
	return uint4(V1.x * V2.x, V1.y * V2.y, V1.z * V2.z, V1.w * V2.w);
}

inline uint4 operator* (const uint4& V, float S)
{
	return uint4(static_cast<uint32_t>(V.x * S), static_cast<uint32_t>(V.y * S), static_cast<uint32_t>(V.z * S), static_cast<uint32_t>(V.w * S));
}

inline uint4 operator/ (const uint4& V1, const uint4& V2)
{
	return uint4(V1.x / V2.x, V1.y / V2.y, V1.z / V2.z, V1.w / V2.w);
}

inline uint4 operator* (float S, const uint4& V)
{
	return uint4(static_cast<uint32_t>(V.x * S), static_cast<uint32_t>(V.y * S), static_cast<uint32_t>(V.z * S), static_cast<uint32_t>(V.w * S));
}

/****************************************************************************
*
* Viewport
*
****************************************************************************/

//------------------------------------------------------------------------------
// Comparision operators
//------------------------------------------------------------------------------

inline bool Viewport::operator == (const Viewport& vp) const
{
	return (x == vp.x && y == vp.y
		&& width == vp.width && height == vp.height
		&& minDepth == vp.minDepth && maxDepth == vp.maxDepth);
}

inline bool Viewport::operator != (const Viewport& vp) const
{
	return (x != vp.x || y != vp.y
		|| width != vp.width || height != vp.height
		|| minDepth != vp.minDepth || maxDepth != vp.maxDepth);
}

//------------------------------------------------------------------------------
// Assignment operators
//------------------------------------------------------------------------------

inline Viewport& Viewport::operator= (const Viewport& vp)
{
	x = vp.x; y = vp.y;
	width = vp.width; height = vp.height;
	minDepth = vp.minDepth; maxDepth = vp.maxDepth;
	return *this;
}

inline Viewport& Viewport::operator= (const Rect& rct)
{
	x = float(rct.left); y = float(rct.top);
	width = float(rct.right - rct.left);
	height = float(rct.bottom - rct.top);
	minDepth = 0.f; maxDepth = 1.f;
	return *this;
}

//------------------------------------------------------------------------------
// Viewport operations
//------------------------------------------------------------------------------

inline float Viewport::AspectRatio() const
{
	if (width == 0.f || height == 0.f)
		return 0.f;

	return (width / height);
}