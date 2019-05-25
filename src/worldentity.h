#pragma once
#include "Util/vertex.h"
#include "Util/matrix.h"

class Engine;

class WorldEntity
{
	public:
		WorldEntity();
		WorldEntity(Vec3 position);
		virtual ~WorldEntity();
		Vec3 getPosition() const;
		void setPosition(Vec3 val);
		const Quaternion& getRotation() const;
		void setRotation(Quaternion q);
		void setScale(float scale);
		Matrix34& getObjectToWorldMatrix();
		Matrix34& getWorldToObjectMatrix();
		// this returns false when the entity has died
		bool getActive() { return (m_flags & eInactive) == 0; }
		virtual void update(Engine&) {}

	protected:
		Vec3       m_position;
		Quaternion m_rotation;
		float      m_scale = 1.0f;

		Matrix34   m_objectToWorldMatrix;
		Matrix34   m_worldToObjectMatrix;


		enum EEntityFlags : uint32_t
		{
			eInvalidWorldTransform        = (1),
			eInvalidInverseWorldTransform = (1 << 1),
			eInactive                     = (1 << 2)
		};

		uint32_t m_flags;
};
