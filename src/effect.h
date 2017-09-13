#pragma once
#include "Util/vertex.h"
#include "ALcontext.h"

class Effect
{
	public:
		Effect(Vec3 l)
			: m_location(l)
			, m_timer(0)
			, m_bActive(true)
		{
		}
		virtual ~Effect() {}
		virtual void draw() = 0;
		virtual void update() = 0;
		void setActive(bool b) { m_bActive = b; }

		bool getActive() const { return m_bActive; }
		const Vec3& getLocation() const { return m_location;}

	protected:
		Vec3 m_location;
		int  m_timer;
		bool m_bActive;
};

class Explosion: public Effect
{
	private:
		unsigned int m_tex;
		//ALuint m_sound;

	public:
		static void cleanUpSound();
		static void initializeSound();
		//static ALuint soundBuffer;
		static unsigned int texture[3];
		Explosion(Vec3);
		~Explosion();
		void draw();
		void update();
};
