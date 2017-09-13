#include "worldentity.h"
#include "bitmap.h"
#include "ALcontext.h"

class Plane : public WorldEntity
{
	public:
		Plane();
		Plane(Vec3);
		~Plane();

		void stopSound();
		void setColor(float *);
		void fire();
		void pitch(float fpitch);
		void roll(float froll);
		void accelerate(float throttle);
		void draw();
		Vec3& getPosition(void);
		void update() override;
		void calculateAIpathfinding(Plane &);

		static void initializeSound();
		static void cleanUpSound();

	private:
		bool  m_gun;
		float m_speed;
		int   m_shootDelay;
		int   m_health;

		float  m_color[3];
		//ALuint m_soundSource;
		//ALuint m_engineSource;
		//static ALuint buffer;
		//static ALuint engineBuf;
};


class Bullet : public WorldEntity
{
	public:
		Bullet();
		Bullet(Vec3, Vec3);
		~Bullet();
		void draw();
		void move();
		void setActive(bool);

		void setEmitter(Plane &);
		bool getActive() override { return m_bActive; }
		Vec3& getPosition() { return m_position; }

	private:
		Vec3 m_heading;
		bool m_bActive;
		Plane* m_emitter;
};
