#pragma once
#include "worldentity.h"

class Camera : public WorldEntity
{
	public:
		Camera();
		virtual ~Camera();
		void lookAtWorldPosition(Vec3 pos, Vec3 up);
		const Matrix34 &getViewMatrix();
		const Matrix44& getProjectionMatrix();
		void setViewport(uint32_t width, uint32_t height, float near, float far, float fov);
		Vec3 getViewVector();

	private:
		float m_nearPlane;
		float m_farPlane;
		float m_fov;
		uint32_t m_width;
		uint32_t m_height;

		bool m_invalidPerspMatrix;

		Matrix44 m_perspectiveMatrix;
};
