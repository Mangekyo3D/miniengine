#pragma once
#include <stdint.h>

class Vec2
{
public:
	Vec2();
	Vec2(float x, float y);
	Vec2(const float* data);

	// get data in provided pointer. Pointer must have size 3 * float
	void getData(float* data) const;
	float x() const {return m_x;}
	float y() const {return m_y;}

	Vec2& operator *=(float fac);

private:
	float m_x, m_y;
};

class Vec3
{
public:
	Vec3();
	Vec3(const Vec3&);
	Vec3(float x, float y, float z);
	Vec3(const float* data);

	// get data in provided pointer. Pointer must have size 3 * float
	void getData(float* data) const;
	float x() const {return m_x;}
	float y() const {return m_y;}
	float z() const {return m_z;}

	void setx(float x) {m_x = x;}
	void sety(float y) {m_y = y;}
	void setz(float z) {m_z = z;}

	Vec3& operator *=(float fac);
	Vec3 operator -();
	void operator -= (const Vec3& v);
	void operator += (const Vec3& v);

	Vec3 operator *(float fac);
	Vec3 operator + (const Vec3& v);

	operator uint32_t ();

	float normalize();
	float length();

private:
	float m_x, m_y, m_z;
};

Vec2 operator + (const Vec2& v1, const Vec2& v2);
Vec2 operator - (const Vec2& v1, const Vec2& v2);
Vec2 operator * (const float fac, const Vec2& v1);

Vec3 cross(const Vec3& v1, const Vec3& v2);
Vec3 operator + (const Vec3& v1, const Vec3& v2);
Vec3 operator - (const Vec3& v1, const Vec3& v2);
Vec3 operator * (const float fac, const Vec3& v1);

float dot(const Vec3& v1, const Vec3& v2);
