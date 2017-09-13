#include "worldentity.h"

WorldEntity::WorldEntity()
{
}

WorldEntity::WorldEntity(Vec3 position)
	: m_position(position)
{
}

WorldEntity::~WorldEntity()
{
}

Vec3 WorldEntity::getPosition() 
{
	return m_position; 
}


void WorldEntity::setPosition(Vec3 val) 
{
	m_position = val; 
	m_flags |= eInvalidWorldTransform | eInvalidInverseWorldTransform;
}

void WorldEntity::setRotation(Quaternion q)
{
	m_rotation = q;
	m_flags |= eInvalidWorldTransform | eInvalidInverseWorldTransform;
}

Matrix34& WorldEntity::getObjectToWorldMatrix()
{
	if (m_flags & eInvalidWorldTransform)
	{
		m_objectToWorldMatrix = Matrix34(m_rotation, m_position);
		m_flags &= ~eInvalidWorldTransform;
	}

	return m_objectToWorldMatrix;
}

Matrix34&WorldEntity::getWorldToObjectMatrix()
{
	if (m_flags & eInvalidInverseWorldTransform)
	{
		m_worldToObjectMatrix = getObjectToWorldMatrix();
		m_worldToObjectMatrix.invertFast();
		m_flags &= ~eInvalidInverseWorldTransform;
	}

	return m_worldToObjectMatrix;
}
