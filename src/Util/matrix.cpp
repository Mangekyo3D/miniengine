#include "matrix.h"
#define _USE_MATH_DEFINES
#include <cmath>
#include <utility>
#include <algorithm>

Quaternion::Quaternion()
	: m_w(1.0f)
	, m_x(0.0f)
	, m_y(0.0f)
	, m_z(0.0f)
{
}

Quaternion::Quaternion(float w, float x, float y, float z)
	: m_w(w)
	, m_x(x)
	, m_y(y)
	, m_z(z)
{
}

Quaternion::Quaternion(Vec3 axis, float angle)
{
	float halfAngle = angle * 0.5f;

	axis.normalize();
	axis *= sin(halfAngle);

	m_w = cos(halfAngle);
	m_x = axis.x();
	m_y = axis.y();
	m_z = axis.z();
}

// we assume that m is orthogonal
Quaternion::Quaternion(Matrix33& m)
{
	m_w = sqrtf (std::max(0.0f, 1.0f + m.getData() [0] + m.getData() [4] + m.getData() [8])) * 0.5f;
	float w4 = 1.0f / (4.0f * m_w);
	m_x = (m.getData() [5] - m.getData() [7]) * w4;
	m_y = (m.getData() [6] - m.getData() [2]) * w4;
	m_z = (m.getData() [1] - m.getData() [3]) * w4;
}

Quaternion Quaternion::operator* (const Quaternion& q)
{
	return Quaternion(
				m_w * q.m_w - m_x * q.m_x - m_y * q.m_y - m_z * q.m_z,
				m_w * q.m_x + m_x * q.m_w + m_y * q.m_z - m_z * q.m_y,
				m_w * q.m_y - m_x * q.m_z + m_y * q.m_w + m_z * q.m_x,
				m_w * q.m_z + m_x * q.m_y - m_y * q.m_x + m_z * q.m_w
				);
}

void Quaternion::normalize()
{
	float lenSq = m_x * m_x + m_y * m_y + m_z * m_z + m_w * m_w;
	float invLen = 1.0f / sqrt(lenSq);

	m_x *= invLen;
	m_y *= invLen;
	m_z *= invLen;
	m_w *= invLen;
}

Matrix44::Matrix44()
{
	for (int i = 0; i < 16; ++i)
	{
		m_data[i] = (i % 5 == 0) ? 1.0f : 0.0f;
	}
}

Matrix44::Matrix44(const Matrix34 &m)
{
	const float* data = m.getConstData();
	m_data[0] = data[0];
	m_data[1] = data[1];
	m_data[2] = data[2];

	m_data[4] = data[3];
	m_data[5] = data[4];
	m_data[6] = data[5];

	m_data[8] = data[6];
	m_data[9] = data[7];
	m_data[10] = data[8];

	m_data[12] = data[9];
	m_data[13] = data[10];
	m_data[14] = data[11];

	m_data[3] = 0.0f;
	m_data[7] = 0.0f;
	m_data[11] = 0.0f;
	m_data[15] = 1.0f;
}

void Matrix44::getData(float* rdata) const
{
	for (int i = 0; i < 16; ++i)
	{
		rdata[i] = m_data[i];
	}
}

Matrix34::Matrix34()
{
	for (int i = 0; i < 12; ++i)
	{
		m_data[i] = (i % 4 == 0) ? 1.0f : 0.0f;
	}
}

Matrix34::Matrix34(const Matrix33& m)
{
	for (int i = 0; i < 9; ++i)
	{
		m_data[i] = m.getData()[i];
	}

	m_data[9] = m_data[10] = m_data[11] = 0.0f;
}

Matrix34::Matrix34(const Matrix34& m)
{
	for (int i = 0; i < 12; ++i)
	{
		m_data[i] = m.getConstData()[i];
	}
}

Matrix34::Matrix34(const Vec3& v1, const Vec3& v2, const Vec3& v3, const Vec3& v4)
{
	v1.getData(m_data);
	v2.getData(m_data + 3);
	v3.getData(m_data + 6);
	v4.getData(m_data + 9);
}

Matrix34::Matrix34(const Quaternion& q, const Vec3& position)
	: Matrix34(Matrix33(q))
{
	position.getData(m_data + 9);
}

Matrix34::Matrix34(const Quaternion& q, float scale, const Vec3& position)
	: Matrix34(Matrix33(q, scale))
{
	position.getData(m_data + 9);
}


Vec3 Matrix34::getColumn(int column) const
{
	return Vec3(m_data + column * 3);
}

void Matrix34::invertFast()
{
	Matrix33 rotationPart(*this);
	rotationPart.invertFast();
	Vec3 invPos = rotationPart * Vec3(&m_data[9]);

	for (int i = 0; i < 9; ++i)
	{
		m_data[i] = rotationPart.getData()[i];
	}
	m_data[9] = -invPos.x();
	m_data[10] = -invPos.y();
	m_data[11] = -invPos.z();
}


Matrix33::Matrix33()
{
	m_data[0] = m_data[4] = m_data[8] = 1.0f;
	m_data[1] = m_data[2] = m_data[3] =
	m_data[5] = m_data[6] = m_data[7] = 0.0f;
}

Matrix33::Matrix33(Matrix33& m)
{
	for (int i = 0; i < 9; ++i)
	{
		m_data[i] = m.m_data[i];
	}
}

Matrix33::Matrix33(const Vec3& v1, const Vec3& v2, const Vec3& v3)
{
	v1.getData(m_data);
	v2.getData(m_data + 3);
	v3.getData(m_data + 6);
}

Matrix33::Matrix33(const Matrix34& m)
{
	for (int i = 0; i < 9; ++i)
	{
		m_data[i] = m.getConstData()[i];
	}
}

Matrix33::Matrix33(const Quaternion& q)
{
	float xw = q.x() * q.w();
	float yw = q.y() * q.w();
	float zw = q.z() * q.w();
	float xy = q.x() * q.y();
	float zy = q.z() * q.y();
	float xz = q.z() * q.x();
	float xx = q.x() * q.x();
	float yy = q.y() * q.y();
	float zz = q.z() * q.z();

	m_data[0] = 1.0f - 2.0f * yy - 2.0f * zz;
	m_data[1] = 2.0f * xy + 2.0f * zw;
	m_data[2] = 2.0f * xz - 2.0f * yw;

	m_data[3] = 2.0f * xy - 2.0f * zw;
	m_data[4] = 1.0f - 2.0f * xx - 2.0f * zz;
	m_data[5] = 2.0f * zy + 2.0f * xw;

	m_data[6] = 2.0f * xz + 2.0f * yw;
	m_data[7] = 2.0f * zy - 2.0f * xw;
	m_data[8] = 1.0f - 2.0f * xx - 2.0f * yy;
}

Matrix33::Matrix33(float scale)
{
	m_data[0] = m_data[4] = m_data[8] = scale;
	m_data[1] = m_data[2] = m_data[3] =
	m_data[5] = m_data[6] = m_data[7] = 0.0f;
}

Matrix33::Matrix33(const Quaternion& q, float scale)
{
	float xw = q.x() * q.w();
	float yw = q.y() * q.w();
	float zw = q.z() * q.w();

	float xy = q.x() * q.y();
	float yy = q.y() * q.y();
	float zy = q.z() * q.y();

	float xz = q.z() * q.x();
	float xx = q.x() * q.x();
	float zz = q.z() * q.z();

	m_data[0] = scale * (1.0f - 2.0f * yy - 2.0f * zz);
	m_data[1] = scale * (2.0f * xy + 2.0f * zw);
	m_data[2] = scale * (2.0f * xz - 2.0f * yw);

	m_data[3] = scale * (2.0f * xy - 2.0f * zw);
	m_data[4] = scale * (1.0f - 2.0f * xx - 2.0f * zz);
	m_data[5] = scale * (2.0f * zy + 2.0f * xw);

	m_data[6] = scale * (2.0f * xz + 2.0f * yw);
	m_data[7] = scale * (2.0f * zy - 2.0f * xw);
	m_data[8] = scale * (1.0f - 2.0f * xx - 2.0f * yy);
}

Vec3 Matrix33::getColumn(int column) const
{
	return Vec3(m_data + column * 3);
}

Matrix33::Matrix33(const Matrix44& m)
{
	m_data[0] = m.getData()[0];
	m_data[1] = m.getData()[1];
	m_data[2] = m.getData()[2];

	m_data[3] = m.getData()[4];
	m_data[4] = m.getData()[5];
	m_data[5] = m.getData()[6];

	m_data[6] = m.getData()[8];
	m_data[7] = m.getData()[9];
	m_data[8] = m.getData()[10];
}

void Matrix33::invertFast()
{
	Vec3 column1(m_data);
	Vec3 column2(m_data + 3);
	Vec3 column3(m_data + 6);

	float f1 = 1.0f / column1.normalize();
	float f2 = 1.0f / column2.normalize();
	float f3 = 1.0f / column3.normalize();

	m_data[0] = f1 * column1.x();
	m_data[1] = f1 * column2.x();
	m_data[2] = f1 * column3.x();

	m_data[3] = f2 * column1.y();
	m_data[4] = f2 * column2.y();
	m_data[5] = f2 * column3.y();

	m_data[6] = f3 * column1.z();
	m_data[7] = f3 * column2.z();
	m_data[8] = f3 * column3.z();
}

void Matrix33::orthonormalizeFast()
{
	Vec3 column1(m_data);
	Vec3 column2(m_data + 3);
	Vec3 column3(m_data + 6);

	float f1 = 1.0f / column1.normalize();
	float f2 = 1.0f / column2.normalize();
	float f3 = 1.0f / column3.normalize();

	m_data[0] *= f1;
	m_data[1] *= f1;
	m_data[2] *= f1;

	m_data[3] *= f2;
	m_data[4] *= f2;
	m_data[5] *= f2;

	m_data[6] *= f3;
	m_data[7] *= f3;
	m_data[8] *= f3;
}

void Matrix33::factorZYX(float &z, float &y, float &x)
{
	y = asin(-m_data[2]);
	if (y < M_PI / 2)
	{
		if (y > -M_PI / 2)
		{
			z = atan2(m_data[1], m_data[0]);
			x = atan2(m_data[5], m_data[8]);
		}
		else
		{
			z = -atan2(-m_data[3], m_data[6]);
			x = 0.0f;
		}
	}
	else
	{
		z = atan2(-m_data[3], m_data[6]);
		x = 0.0f;
	}
}

void Matrix33::factorYXZ(float &y, float &x, float &z)
{
	x = asin(-m_data[7]);
	if (x < M_PI / 2)
	{
		if (x > -M_PI / 2)
		{
			y = atan2(m_data[6], m_data[8]);
			z = atan2(m_data[1], m_data[4]);
		}
		else
		{
			y = -atan2(-m_data[3], m_data[0]);
			z = 0.0f;
		}
	}
	else
	{
		y = atan2(-m_data[3], m_data[0]);
		z = 0.0f;
	}
}

void Matrix33::transpose()
{
	std::swap(m_data[1], m_data[3]);
	std::swap(m_data[2], m_data[6]);
	std::swap(m_data[5], m_data[7]);
}

Vec3 Matrix33::operator * (const Vec3& v)
{
	return getColumn(0) * v.x() + getColumn(1) * v.y() + getColumn(2) * v.z();
}

Matrix33 Matrix33::operator *(const Matrix33& m)
{
	return Matrix33(*this * m.getColumn(0),
				*this * m.getColumn(1),
				*this * m.getColumn(2));
}


Matrix34 Matrix34::generateTranslation(float x, float y, float z)
{
	Matrix34 m;
	m.m_data[9] = x;
	m.m_data[10] = y;
	m.m_data[11] = z;

	return m;
}

Matrix33 Matrix33::generateRotation(float angle, Vec3 axis)
{
	Matrix33 m;

	return m;
}

Matrix33 Matrix33::generateScale(float x, float y, float z)
{
	Matrix33 m;

	m.m_data[0] = x;
	m.m_data[4] = y;
	m.m_data[8] = z;

	return m;
}

Matrix44 Matrix44::generatePerspective(float nearPlane, float farPlane, float aspect, float fov)
{
	Matrix44 m;

	float tanval = tan(static_cast <float> (fov * M_PI / 180.0));
	float width, height;

	// here we modify the standard perspective transform slightly to ensure that the wider
	// component is applied for fov calculation
	if (aspect < 1.0f)
	{
		height = tanval * nearPlane;
		width = aspect * height;
	}
	else
	{
		width = tanval * nearPlane;
		height =  width / aspect;
	}

	// column 1
	m.m_data[0] = nearPlane / width;
	m.m_data[1] = 0.0f;
	m.m_data[2] = 0.0f;
	m.m_data[3] = 0.0f;

	// column 2
	m.m_data[4] = 0.0f;
	m.m_data[5] = nearPlane / height;
	m.m_data[6] = 0.0f;
	m.m_data[7] = 0.0f;

	// column 3
	m.m_data[8] = 0.0f;
	m.m_data[9] = 0.0f;
	m.m_data[10] = -(nearPlane + farPlane) / (farPlane - nearPlane);
	m.m_data[11] = -1.0;

	// column 4
	m.m_data[12] = 0.0f;
	m.m_data[13] = 0.0f;
	m.m_data[14] = -2.0f * farPlane * nearPlane / (farPlane - nearPlane);
	m.m_data[15] = 0.0f;

	return m;
}

