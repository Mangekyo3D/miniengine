#include "camera.h"
#include <cmath>
#include "Util/mathutils.h"
#include <algorithm>

Camera::Camera()
{
	setViewport(800, 600, 0.1f, 100.0f, 30.0f);
	// position camera a bit high on z axis
	setPosition(Vec3(32.0f, 32.0f, 80.0f));
}

Camera::~Camera()
{
}

Vec3 Camera::getViewVector()
{
	return getObjectToWorldMatrix().getColumn(2);
}

const Matrix34& Camera::getViewMatrix()
{
	return getWorldToObjectMatrix();
}

const Matrix34&Camera::getViewMatrixInverse()
{
	return getObjectToWorldMatrix();
}

const Matrix44& Camera::getProjectionMatrix()
{
	if (m_invalidPerspMatrix)
	{
		m_invalidPerspMatrix = false;
		m_perspectiveMatrix = Matrix44::generatePerspective(m_nearPlane, m_farPlane, m_width / (float)m_height, m_fov);
	}
	return m_perspectiveMatrix;
}

void Camera::setViewport(uint32_t width, uint32_t height, float near, float far, float fov)
{
	m_width = width;
	m_height = height;
	m_nearPlane = near;
	m_farPlane = far;
	m_fov = fov;

	m_invalidPerspMatrix = true;
}

void Camera::lookAtWorldPosition(Vec3 pos, Vec3 up)
{
	// we use look from because this will be the positive z axis of the camera (camera looks down the z axis)
	Vec3 lookFromVector =  m_position - pos;

	if (lookFromVector.normalize() > 0.0f && up.normalize() > 0.0f)
	{
		Vec3 rotAxis = cross(up, lookFromVector);

		if (rotAxis.normalize() != 0.0f)
		{
			Vec3 upVector = cross(lookFromVector, rotAxis);

			Matrix33 m(rotAxis, upVector, lookFromVector);
			Quaternion q = m;
			// by calling set function, we make sure that flags are set properly
			setRotation(q);
		}
		else
		{
			float sign = copysignf(1.0f, lookFromVector.z());
			const float a = -1.0f / (sign + lookFromVector.z());
			const float b = lookFromVector.x() * lookFromVector.y() * a;
			Vec3 v1 = Vec3(1.0f + sign * lookFromVector.x() * lookFromVector.x() * a, sign * b, -sign * lookFromVector.x());
			Vec3 v2 = Vec3(b, sign + lookFromVector.y() * lookFromVector.y() * a, -lookFromVector.y());
			Matrix33 m(v1, v2, lookFromVector);
			Quaternion q = m;
			// by calling set function, we make sure that flags are set properly
			setRotation(q);
		}
	}
}

void Camera::followFromBehind(WorldEntity& entity, float distance, float maxTurnDegrees, float maxDiffDegrees)
{
	Matrix33 transform(entity.getRotation());
	// construct new transform for camera
	Matrix33 newTransform(transform.getColumn(0), transform.getColumn(2), -transform.getColumn(1));

	// construct a quaternion from the new 3x3 matrix
	Quaternion finalRot(newTransform);

	Quaternion rot = m_rotation;
	if (dot(m_rotation, finalRot) < 0.0f)
	{
		finalRot = -finalRot;
	}

	// The quaternion to rotate from m_rotation to q is q * m_rotation ^ -1
	rot.invertUnit();
	rot = finalRot * rot;
	Vec3 rotAxis = rot.getRotationAxis();
	float rotAngle = rot.getRotationAngle();

	// don't change the rotation if we are less than an angle off. It keeps the camera from jittering
	if (rotAngle > degreesToRads(1.0f))
	{
		const float fMaxTurnRads = degreesToRads(maxTurnDegrees);
		const float fMaxDiffRads = degreesToRads(maxDiffDegrees);
		// first, clamp the rate of rotation by the maximum per frame rotation
		float fRot = std::min(fMaxTurnRads, rotAngle);

		// if camera is way off, then also keep the angle difference clamped
		if (rotAngle - fRot > fMaxDiffRads)
		{
			fRot = rotAngle - fMaxDiffRads;
		}

		rot = Quaternion(rotAxis, fRot);

		setRotation(rot * m_rotation);
	}

	setPosition(entity.getPosition() + distance * getObjectToWorldMatrix().getColumn(2).getNormalized());
}
