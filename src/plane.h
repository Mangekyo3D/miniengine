#include "worldentity.h"
#include "bitmap.h"
#include "audiointerface.h"
#include "controller.h"

class CIndexedInstancedBatch;
class CDynamicArrayBatch;
class IAudioResource;
class IAudioInstance;
struct SAssets;

class Plane : public WorldEntity
{
	public:
		Plane(Vec3, Engine& engine);
		~Plane() override;

		void stopSound();
		void setColor(float *);
		void fire(Engine& engine);
		void pitch(float fpitch);
		void roll(float froll);
		void accelerate(float throttle);
		Vec3& getPosition(void);
		float getSpeed() const { return m_speed; }
		void update(Engine& engine) override;

		// temporary, make a unified way for components to request class wide components
		static SAssets s_assets;

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
		void update(Engine& engine) override;

	private:
		Plane* m_plane;
};

class PlanePlayerController : public Controller
{
	public:
		PlanePlayerController(Plane* plane);
		void update(Engine& engine) override;

	private:
		Plane* m_plane;
};


class Bullet : public WorldEntity
{
	public:
		Bullet(WorldEntity&, Vec3 position, Vec3 heading, Engine&);
		~Bullet() override;
		virtual void update(Engine& engine) override;

	private:
		Vec3 m_heading;
		const WorldEntity* m_emitter;
		static CDynamicArrayBatch* s_batch;
};
