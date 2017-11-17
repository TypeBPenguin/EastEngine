#pragma once

/****************************************************************************
*
* Int2
*
****************************************************************************/

//------------------------------------------------------------------------------
// Comparision operators
//------------------------------------------------------------------------------

inline bool Int2::operator == (const Int2& V) const
{
	return this->x == V.x && this->y == V.y;
}

inline bool Int2::operator != (const Int2& V) const
{
	return (this->x == V.x && this->y == V.y) == false;
}

//------------------------------------------------------------------------------
// Assignment operators
//------------------------------------------------------------------------------

inline Int2& Int2::operator+= (const Int2& V)
{
	this->x += V.x;
	this->y += V.y;

	return *this;
}

inline Int2& Int2::operator-= (const Int2& V)
{
	this->x -= V.x;
	this->y -= V.y;

	return *this;
}

inline Int2& Int2::operator*= (const Int2& V)
{
	this->x *= V.x;
	this->y *= V.y;

	return *this;
}

inline Int2& Int2::operator*= (float S)
{
	this->x = static_cast<int>(this->x * S);
	this->y = static_cast<int>(this->y * S);

	return *this;
}

inline Int2& Int2::operator/= (float S)
{
	assert(S != 0.0f);

	this->x = static_cast<int>(this->x / S);
	this->y = static_cast<int>(this->y / S);

	return *this;
}

//------------------------------------------------------------------------------
// Binary operators
//------------------------------------------------------------------------------

inline Int2 operator+ (const Int2& V1, const Int2& V2)
{
	return Int2(V1.x + V2.x, V1.y + V2.y);
}

inline Int2 operator- (const Int2& V1, const Int2& V2)
{
	return Int2(V1.x - V2.x, V1.y - V2.y);
}

inline Int2 operator* (const Int2& V1, const Int2& V2)
{
	return Int2(V1.x * V2.x, V1.y * V2.y);
}

inline Int2 operator* (const Int2& V, float S)
{
	return Int2(static_cast<int>(V.x * S), static_cast<int>(V.y * S));
}

inline Int2 operator/ (const Int2& V1, const Int2& V2)
{
	return Int2(V1.x / V2.x, V1.y / V2.y);
}

inline Int2 operator* (float S, const Int2& V)
{
	return Int2(static_cast<int>(V.x * S), static_cast<int>(V.y * S));
}

/****************************************************************************
*
* Int3
*
****************************************************************************/

//------------------------------------------------------------------------------
// Comparision operators
//------------------------------------------------------------------------------

inline bool Int3::operator == (const Int3& V) const
{
	return this->x == V.x && this->y == V.y && this->z == V.z;
}

inline bool Int3::operator != (const Int3& V) const
{
	return (this->x == V.x && this->y == V.y && this->z == V.z) == false;
}

//------------------------------------------------------------------------------
// Assignment operators
//------------------------------------------------------------------------------

inline Int3& Int3::operator+= (const Int3& V)
{
	this->x += V.x;
	this->y += V.y;
	this->z += V.z;

	return *this;
}

inline Int3& Int3::operator-= (const Int3& V)
{
	this->x -= V.x;
	this->y -= V.y;
	this->z -= V.z;

	return *this;
}

inline Int3& Int3::operator*= (const Int3& V)
{
	this->x *= V.x;
	this->y *= V.y;
	this->z *= V.z;

	return *this;
}

inline Int3& Int3::operator*= (float S)
{
	this->x = static_cast<int>(this->x / S);
	this->y = static_cast<int>(this->y / S);
	this->z = static_cast<int>(this->z / S);

	return *this;
}

inline Int3& Int3::operator/= (float S)
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

inline Int3 operator+ (const Int3& V1, const Int3& V2)
{
	return Int3(V1.x + V2.x, V1.y + V2.y, V1.z + V2.z);
}

inline Int3 operator- (const Int3& V1, const Int3& V2)
{
	return Int3(V1.x - V2.x, V1.y - V2.y, V1.z - V2.z);
}

inline Int3 operator* (const Int3& V1, const Int3& V2)
{
	return Int3(V1.x * V2.x, V1.y * V2.y, V1.z * V2.z);
}

inline Int3 operator* (const Int3& V, float S)
{
	return Int3(static_cast<int>(V.x * S), static_cast<int>(V.y * S), static_cast<int>(V.z * S));
}

inline Int3 operator/ (const Int3& V1, const Int3& V2)
{
	return Int3(V1.x / V2.x, V1.y / V2.y, V1.z / V2.z);
}

inline Int3 operator* (float S, const Int3& V)
{
	return Int3(static_cast<int>(V.x * S), static_cast<int>(V.y * S), static_cast<int>(V.z * S));
}

/****************************************************************************
*
* Int4
*
****************************************************************************/

//------------------------------------------------------------------------------
// Comparision operators
//------------------------------------------------------------------------------

inline bool Int4::operator == (const Int4& V) const
{
	return this->x == V.x && this->y == V.y && this->z == V.z && this->w == V.w;
}

inline bool Int4::operator != (const Int4& V) const
{
	return (this->x == V.x && this->y == V.y && this->z == V.z && this->w == V.w) == false;
}

//------------------------------------------------------------------------------
// Assignment operators
//------------------------------------------------------------------------------

inline Int4& Int4::operator+= (const Int4& V)
{
	this->x += V.x;
	this->y += V.y;
	this->z += V.z;
	this->w += V.w;

	return *this;
}

inline Int4& Int4::operator-= (const Int4& V)
{
	this->x -= V.x;
	this->y -= V.y;
	this->z -= V.z;
	this->w -= V.w;

	return *this;
}

inline Int4& Int4::operator*= (const Int4& V)
{
	this->x *= V.x;
	this->y *= V.y;
	this->z *= V.z;
	this->w *= V.w;

	return *this;
}

inline Int4& Int4::operator*= (float S)
{
	this->x = static_cast<int>(this->x * S);
	this->y = static_cast<int>(this->y * S);
	this->z = static_cast<int>(this->z * S);
	this->w = static_cast<int>(this->w * S);

	return *this;
}

inline Int4& Int4::operator/= (float S)
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

inline Int4 operator+ (const Int4& V1, const Int4& V2)
{
	return Int4(V1.x + V2.x, V1.y + V2.y, V1.z + V2.z, V1.w + V2.w);
}

inline Int4 operator- (const Int4& V1, const Int4& V2)
{
	return Int4(V1.x - V2.x, V1.y - V2.y, V1.z - V2.z, V1.w - V2.w);
}

inline Int4 operator* (const Int4& V1, const Int4& V2)
{
	return Int4(V1.x * V2.x, V1.y * V2.y, V1.z * V2.z, V1.w * V2.w);
}

inline Int4 operator* (const Int4& V, float S)
{
	return Int4(static_cast<int>(V.x * S), static_cast<int>(V.y * S), static_cast<int>(V.z * S), static_cast<int>(V.w * S));
}

inline Int4 operator/ (const Int4& V1, const Int4& V2)
{
	return Int4(V1.x / V2.x, V1.y / V2.y, V1.z / V2.z, V1.w / V2.w);
}

inline Int4 operator* (float S, const Int4& V)
{
	return Int4(static_cast<int>(V.x * S), static_cast<int>(V.y * S), static_cast<int>(V.z * S), static_cast<int>(V.w * S));
}

/****************************************************************************
*
* UInt2
*
****************************************************************************/

//------------------------------------------------------------------------------
// Comparision operators
//------------------------------------------------------------------------------

inline bool UInt2::operator == (const UInt2& V) const
{
	return this->x == V.x && this->y == V.y;
}

inline bool UInt2::operator != (const UInt2& V) const
{
	return (this->x == V.x && this->y == V.y) == false;
}

//------------------------------------------------------------------------------
// Assignment operators
//------------------------------------------------------------------------------

inline UInt2& UInt2::operator+= (const UInt2& V)
{
	this->x += V.x;
	this->y += V.y;

	return *this;
}

inline UInt2& UInt2::operator-= (const UInt2& V)
{
	this->x -= V.x;
	this->y -= V.y;

	return *this;
}

inline UInt2& UInt2::operator*= (const UInt2& V)
{
	this->x *= V.x;
	this->y *= V.y;

	return *this;
}

inline UInt2& UInt2::operator*= (float S)
{
	this->x = static_cast<uint32_t>(this->x * S);
	this->y = static_cast<uint32_t>(this->y * S);

	return *this;
}

inline UInt2& UInt2::operator/= (float S)
{
	assert(S != 0.0f);

	this->x = static_cast<uint32_t>(this->x / S);
	this->y = static_cast<uint32_t>(this->y / S);

	return *this;
}

//------------------------------------------------------------------------------
// Binary operators
//------------------------------------------------------------------------------

inline UInt2 operator+ (const UInt2& V1, const UInt2& V2)
{
	return UInt2(V1.x + V2.x, V1.y + V2.y);
}

inline UInt2 operator- (const UInt2& V1, const UInt2& V2)
{
	return UInt2(V1.x - V2.x, V1.y - V2.y);
}

inline UInt2 operator* (const UInt2& V1, const UInt2& V2)
{
	return UInt2(V1.x * V2.x, V1.y * V2.y);
}

inline UInt2 operator* (const UInt2& V, float S)
{
	return UInt2(static_cast<uint32_t>(V.x * S), static_cast<uint32_t>(V.y * S));
}

inline UInt2 operator/ (const UInt2& V1, const UInt2& V2)
{
	return UInt2(V1.x / V2.x, V1.y / V2.y);
}

inline UInt2 operator* (float S, const UInt2& V)
{
	return UInt2(static_cast<uint32_t>(V.x * S), static_cast<uint32_t>(V.y * S));
}

/****************************************************************************
*
* UInt3
*
****************************************************************************/

//------------------------------------------------------------------------------
// Comparision operators
//------------------------------------------------------------------------------

inline bool UInt3::operator == (const UInt3& V) const
{
	return this->x == V.x && this->y == V.y && this->z == V.z;
}

inline bool UInt3::operator != (const UInt3& V) const
{
	return (this->x == V.x && this->y == V.y && this->z == V.z) == false;
}

//------------------------------------------------------------------------------
// Assignment operators
//------------------------------------------------------------------------------

inline UInt3& UInt3::operator+= (const UInt3& V)
{
	this->x += V.x;
	this->y += V.y;
	this->z += V.z;

	return *this;
}

inline UInt3& UInt3::operator-= (const UInt3& V)
{
	this->x -= V.x;
	this->y -= V.y;
	this->z -= V.z;

	return *this;
}

inline UInt3& UInt3::operator*= (const UInt3& V)
{
	this->x *= V.x;
	this->y *= V.y;
	this->z *= V.z;

	return *this;
}

inline UInt3& UInt3::operator*= (float S)
{
	this->x = static_cast<uint32_t>(this->x * S);
	this->y = static_cast<uint32_t>(this->y * S);
	this->z = static_cast<uint32_t>(this->z * S);

	return *this;
}

inline UInt3& UInt3::operator/= (float S)
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

inline UInt3 operator+ (const UInt3& V1, const UInt3& V2)
{
	return UInt3(V1.x + V2.x, V1.y + V2.y, V1.z + V2.z);
}

inline UInt3 operator- (const UInt3& V1, const UInt3& V2)
{
	return UInt3(V1.x - V2.x, V1.y - V2.y, V1.z - V2.z);
}

inline UInt3 operator* (const UInt3& V1, const UInt3& V2)
{
	return UInt3(V1.x * V2.x, V1.y * V2.y, V1.z * V2.z);
}

inline UInt3 operator* (const UInt3& V, float S)
{
	return UInt3(static_cast<uint32_t>(V.x * S), static_cast<uint32_t>(V.y * S), static_cast<uint32_t>(V.z * S));
}

inline UInt3 operator/ (const UInt3& V1, const UInt3& V2)
{
	return UInt3(V1.x / V2.x, V1.y / V2.y, V1.z / V2.z);
}

inline UInt3 operator* (float S, const UInt3& V)
{
	return UInt3(static_cast<uint32_t>(V.x * S), static_cast<uint32_t>(V.y * S), static_cast<uint32_t>(V.z * S));
}

/****************************************************************************
*
* UInt4
*
****************************************************************************/

//------------------------------------------------------------------------------
// Comparision operators
//------------------------------------------------------------------------------

inline bool UInt4::operator == (const UInt4& V) const
{
	return this->x == V.x && this->y == V.y && this->z == V.z && this->w == V.w;
}

inline bool UInt4::operator != (const UInt4& V) const
{
	return (this->x == V.x && this->y == V.y && this->z == V.z && this->w == V.w) == false;
}

//------------------------------------------------------------------------------
// Assignment operators
//------------------------------------------------------------------------------

inline UInt4& UInt4::operator+= (const UInt4& V)
{
	this->x += V.x;
	this->y += V.y;
	this->z += V.z;
	this->w += V.w;

	return *this;
}

inline UInt4& UInt4::operator-= (const UInt4& V)
{
	this->x -= V.x;
	this->y -= V.y;
	this->z -= V.z;
	this->w -= V.w;

	return *this;
}

inline UInt4& UInt4::operator*= (const UInt4& V)
{
	this->x *= V.x;
	this->y *= V.y;
	this->z *= V.z;
	this->w *= V.w;

	return *this;
}

inline UInt4& UInt4::operator*= (float S)
{
	this->x = static_cast<uint32_t>(this->x * S);
	this->y = static_cast<uint32_t>(this->y * S);
	this->z = static_cast<uint32_t>(this->z * S);
	this->w = static_cast<uint32_t>(this->w * S);

	return *this;
}

inline UInt4& UInt4::operator/= (float S)
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

inline UInt4 operator+ (const UInt4& V1, const UInt4& V2)
{
	return UInt4(V1.x + V2.x, V1.y + V2.y, V1.z + V2.z, V1.w + V2.w);
}

inline UInt4 operator- (const UInt4& V1, const UInt4& V2)
{
	return UInt4(V1.x - V2.x, V1.y - V2.y, V1.z - V2.z, V1.w - V2.w);
}

inline UInt4 operator* (const UInt4& V1, const UInt4& V2)
{
	return UInt4(V1.x * V2.x, V1.y * V2.y, V1.z * V2.z, V1.w * V2.w);
}

inline UInt4 operator* (const UInt4& V, float S)
{
	return UInt4(static_cast<uint32_t>(V.x * S), static_cast<uint32_t>(V.y * S), static_cast<uint32_t>(V.z * S), static_cast<uint32_t>(V.w * S));
}

inline UInt4 operator/ (const UInt4& V1, const UInt4& V2)
{
	return UInt4(V1.x / V2.x, V1.y / V2.y, V1.z / V2.z, V1.w / V2.w);
}

inline UInt4 operator* (float S, const UInt4& V)
{
	return UInt4(static_cast<uint32_t>(V.x * S), static_cast<uint32_t>(V.y * S), static_cast<uint32_t>(V.z * S), static_cast<uint32_t>(V.w * S));
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