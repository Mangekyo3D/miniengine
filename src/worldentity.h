#pragma once
#include "Util/vertex.h"
#include "Util/matrix.h"

class WorldEntity
{
	public:
		WorldEntity();
		WorldEntity(Vec3 position);
		virtual ~WorldEntity();
		Vec3 getPosition();
		void setPosition(Vec3 val);
		void setRotation(Quaternion q);
		Matrix34& getObjectToWorldMatrix();
		Matrix34& getWorldToObjectMatrix();
		// this returns false when the entity has died
		virtual bool getActive() { return true; }
		virtual void update() {}

	protected:
		Vec3       m_position;
		Quaternion m_rotation;

		Matrix34   m_objectToWorldMatrix;
		Matrix34   m_worldToObjectMatrix;


		enum EEntityFlags
		{
			eInvalidWorldTransform        = (1),
			eInvalidInverseWorldTransform = (1 << 1)
		};

		uint32_t m_flags;
};
