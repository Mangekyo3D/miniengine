#include "worldentity.h"

WorldEntity::WorldEntity()
	: m_flags(0)
{
}

WorldEntity::WorldEntity(Vec3 position)
	: m_position(position)
	, m_flags(eInvalidWorldTransform | eInvalidInverseWorldTransform)
{
}

WorldEntity::~WorldEntity()
{
}

Vec3 WorldEntity::getPosition() const
{
	return m_position; 
}


void WorldEntity::setPosition(Vec3 val) 
{
	m_position = val; 
	m_flags |= eInvalidWorldTransform | eInvalidInverseWorldTransform;
}

const Quaternion& WorldEntity::getRotation() const
{
	return m_rotation;
}

void WorldEntity::setRotation(Quaternion q)
{
	m_rotation = q;
	m_flags |= eInvalidWorldTransform | eInvalidInverseWorldTransform;
}

void WorldEntity::setScale(float scale)
{
	m_scale = scale;
	m_flags |= eInvalidWorldTransform | eInvalidInverseWorldTransform;
}

Matrix34& WorldEntity::getObjectToWorldMatrix()
{
	if (m_flags & eInvalidWorldTransform)
	{
		m_objectToWorldMatrix = Matrix34(m_rotation, m_scale, m_position);
		m_flags &= ~eInvalidWorldTransform;
	}

	return m_objectToWorldMatrix;
}

Matrix34& WorldEntity::getWorldToObjectMatrix()
{
	if (m_flags & eInvalidInverseWorldTransform)
	{
		Quaternion invQuat = m_rotation;
		invQuat.invertUnit();
		Matrix33 rotationPart(invQuat);
		Vec3 invPos = rotationPart * m_position;

		m_worldToObjectMatrix = Matrix34(invQuat, 1.0f / m_scale, -invPos);
		m_flags &= ~eInvalidInverseWorldTransform;
	}

	return m_worldToObjectMatrix;
}
