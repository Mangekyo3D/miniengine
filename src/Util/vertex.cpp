#include "vertex.h"
#include <cmath>
#include <algorithm>

Vec2::Vec2()
	: m_x(0.0f)
	, m_y(0.0f)
{
}

Vec2::Vec2(float x, float y)
	: m_x(x)
	, m_y(y)
{

}

Vec2::Vec2(const float* data)
{
	m_x = data[0];
	m_y = data[1];
}

void Vec2::getData(float *data) const
{
	data[0] = m_x;
	data[1] = m_y;
}

Vec2& Vec2::operator *=(float fac)
{
	m_x *= fac;
	m_y *= fac;

	return *this;
}


Vec3::Vec3()
	: m_x(0.0f)
	, m_y(0.0f)
	, m_z(0.0f)
{
}

Vec3::Vec3(float x, float y, float z)
	: m_x(x)
	, m_y(y)
	, m_z(z)
{
}

Vec3::Vec3(const Vec3& v)
	: m_x(v.m_x)
	, m_y(v.m_y)
	, m_z(v.m_z)
{
}

Vec3::Vec3(const float* data)
{
	m_x = data[0];
	m_y = data[1];
	m_z = data[2];
}

void Vec3::getData(float *data) const
{
	data[0] = m_x;
	data[1] = m_y;
	data[2] = m_z;
}

Vec3& Vec3::operator *=(float fac)
{
	m_x *= fac;
	m_y *= fac;
	m_z *= fac;

	return *this;
}

Vec3 Vec3::operator -()
{
	return Vec3(-m_x, -m_y, -m_z);
}

float Vec3::length()
{
	float len_sq = m_x * m_x + m_y * m_y + m_z * m_z;
	return sqrt(len_sq);
}

float Vec3::normalize()
{
	float l = length();

	if (l != 0.0f)
	{
		float inv_len = 1.0f / length();

		m_x *= inv_len;
		m_y *= inv_len;
		m_z *= inv_len;
	}
	return l;
}

Vec3 Vec3::getNormalized()
{
	Vec3 result(*this);
	result.normalize();
	return result;
}



Vec3 cross(const Vec3& v1, const Vec3& v2)
{
	return Vec3(v1.y() * v2.z() - v2.y() * v1.z(),
				v2.x() * v1.z() - v1.x() * v2.z(),
				v1.x() * v2.y() - v2.x() * v1.y());
}


Vec3 operator +(const Vec3 &v1, const Vec3 &v2)
{
	return Vec3(v1.x() + v2.x(),
				v1.y() + v2.y(),
				v1.z() + v2.z());
}

Vec3 operator -(const Vec3 &v1, const Vec3 &v2)
{
	return Vec3(v1.x() - v2.x(),
				v1.y() - v2.y(),
				v1.z() - v2.z());
}

Vec3 operator *(const float fac, const Vec3 &v1)
{
	Vec3 res = v1;
	res *= fac;
	return res;
}


void Vec3::operator -= (const Vec3& v)
{
	m_x -= v.x();
	m_y -= v.y();
	m_z -= v.z();
}

void Vec3::operator += (const Vec3& v)
{
	m_x += v.x();
	m_y += v.y();
	m_z += v.z();
}

Vec3 Vec3::operator *(float fac)
{
	Vec3 result(*this);
	result *= fac;
	return result;
}

Vec3 Vec3::operator + (const Vec3& v)
{
	Vec3 result(*this);
	result += v;
	return result;
}


struct Vec3IntegerPack
{
	int x : 10;
	int y : 10;
	int z : 10;
	int w : 2;
};

Vec3::operator uint32_t ()
{
	uint32_t v = 0;

	Vec3IntegerPack& packer = reinterpret_cast <Vec3IntegerPack&> (v);

	float tmp = std::min(std::max(m_x * 511.0f, -511.0f), 511.f);
	packer.x = static_cast<unsigned short> (tmp);
	tmp = std::min(std::max(m_y * 511.0f, -511.0f), 511.f);
	packer.y = static_cast<unsigned short> (tmp);
	tmp = std::min(std::max(m_z * 511.0f, -511.0f), 511.f);
	packer.z = static_cast<unsigned short> (tmp);

	return v;
}

float dot(const Vec3 &v1, const Vec3 &v2)
{
	return v1.x() * v2.x() + v1.y() * v2.y() + v1.z() * v2.z();
}
