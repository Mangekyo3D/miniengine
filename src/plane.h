#include "worldentity.h"
#include "bitmap.h"
#include "audiointerface.h"
#include "controller.h"

class CIndexedInstancedBatch;
class CDynamicArrayBatch;
class IAudioResource;

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
		float getSpeed() const { return m_speed; }
		void update() override;

		static void initializeSound();
		static void cleanUpSound();

		// temporary, make a unified way for components to request class wide components
		static CIndexedInstancedBatch* s_batch;
		static IAudioResource* s_engineAudio;
		static IAudioResource* s_laserAudio;

	private:
		bool  m_gun;
		float m_speed;
		int   m_shootDelay;
		int   m_health;

		float  m_color[3];

		IAudioInstance* m_engineAudio;
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
		static CDynamicArrayBatch* s_batch;
};
