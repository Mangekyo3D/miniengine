#pragma once
#include "vertex.h"

class Matrix44;
class Matrix34;
class Matrix33;
class Quaternion;

class Quaternion
{
public:
	// default quaternion with no rotation
	Quaternion();
	Quaternion(float w, float x, float y, float z);
	// quaternion with rotation around certain axis.
	Quaternion(Vec3 axis, float angle);
	Quaternion(Matrix33& m);

	// hamilton product of two quaternions
	Quaternion operator* (const Quaternion& q);
	Quaternion operator- ();

	void invertUnit();
	// normalize a quaternion to unit length
	void normalize();

	float getRotationAngle();
	Vec3  getRotationAxis();

	float w() const { return m_w; }
	float x() const { return m_x; }
	float y() const { return m_y; }
	float z() const { return m_z; }

private:
	float m_w;
	float m_x;
	float m_y;
	float m_z;
};

float dot(Quaternion& q1, Quaternion& q2);

class Matrix44
{
public:
	Matrix44();
	Matrix44(const Matrix34& m);
	const float* data() const {return m_data;}
	void copyData(float* rdata) const;

	static Matrix44 generatePerspective(float nearPlane, float farPlane, float aspect, float fov);

private:
	float m_data[16];
};

class Matrix34
{
public:
	Matrix34();
	Matrix34(const Vec3& v1, const Vec3& v2, const Vec3& v3, const Vec3& v4);
	Matrix34(const Matrix33& m);
	Matrix34(const Matrix34& m);
	Matrix34(const Quaternion& q, const Vec3& position);
	Matrix34(const Quaternion& q, float scale[3], const Vec3& position);

	const float* getConstData() const {return m_data;}
	float* data() {return m_data;}
	Vec3 getColumn(int column) const;

	static Matrix34 generateTranslation(float x, float y, float z);

private:
	float m_data[12];
};

class Matrix33
{
public:
	Matrix33();
	Matrix33(Matrix33& m);
	Matrix33(const Vec3& v1, const Vec3& v2, const Vec3& v3);
	Matrix33(const Matrix44& m);
	Matrix33(const Matrix34& m);
	Matrix33(const Quaternion& q);
	Matrix33(float scale[3]);
	Matrix33(const Quaternion& q, float scale[3]);

	const float* data() const { return m_data; }
	Vec3 getColumn(int column) const;

	// factor as rotation between z, y and x angles
	void factorZYX(float& z, float& y, float& x);
	void factorYXZ(float& y, float& x, float& z);

	void transpose();

	Vec3 operator * (const Vec3& v);
	Matrix33 operator * (const Matrix33& m);

private:
	float m_data[9];
};
