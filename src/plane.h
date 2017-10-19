#include "worldentity.h"
#include "bitmap.h"
#include "ALcontext.h"
#include "controller.h"

class CBatch;
class CDynamicBatch;

class Plane : public WorldEntity
{
	public:
		Plane(Vec3);
		~Plane();

		void stopSound();
		void setColor(float *);
		void fire();
		void pitch(float fpitch);
		void roll(float froll);
		void accelerate(float throttle);
		Vec3& getPosition(void);
		void update() override;

		static void initializeSound();
		static void cleanUpSound();

		static CBatch* s_batch;

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

class PlaneAIController : public Controller
{
	public:
		PlaneAIController(Plane* plane);
		void update() override;

	private:
		Plane* m_plane;
};

class PlanePlayerController : public Controller
{
	public:
		PlanePlayerController(Plane* plane);
		void update() override;

	private:
		Plane* m_plane;
};


class Bullet : public WorldEntity
{
	public:
		Bullet();
		Bullet(Vec3, Vec3);
		~Bullet();
		void draw();
		void setEmitter(Plane &);
		virtual void update() override;

	private:
		Vec3 m_heading;
		Plane* m_emitter;
		static CDynamicBatch* s_batch;
};
