#include "plane.h"
#include "effect.h"
#include "SMDmodel.h"
#include "camera.h"
#include <vector>
#include "worldtile.h"
#include <cmath>
#include "engine.h"
#include "resourcemanager.h"
#include "render/compositingpipeline.h"

struct SAssets {
	CIndexedInstancedBatch* batch;
	IAudioResource* engineAudio;
	IAudioResource* laserAudio;
	bool bInitialized;
};

SAssets Plane::s_assets = {nullptr, nullptr, nullptr, false};

Plane::Plane(Vec3 initPos, Engine& engine)
	: m_health(1)
{
	m_position = initPos;

	for(int i = 0; i < 3; i++)
	{
		m_color[i] = 1.0f;
	}

	m_speed = 0.01f;
	m_shootDelay = 0;
	m_gun = true;

	if (!s_assets.bInitialized)
	{
		ResourceManager* resourceManager = engine.getResourceManager();
		auto model = resourceManager->loadModel("plane2.smd");
		if (model)
		{
			Renderer* renderer = engine.getRenderer();
			s_assets.batch = model->createBatch(*renderer);
		}
		s_assets.engineAudio = resourceManager->loadAudio("engine.wav");
		s_assets.laserAudio = resourceManager->loadAudio("laser.wav");

		s_assets.bInitialized = true;
	}

	setScale(0.01f);

	IAudioDevice& audioDevice = IAudioDevice::get();
	SAudioInitParams params;

	params.position = m_position;
	params.gain = 0.3f;

	m_engineAudio = audioDevice.loopResource(*s_assets.engineAudio, params);
}

Plane::~Plane()
{
	stopSound();
}

void Plane::stopSound()
{
	IAudioDevice& audioDevice = IAudioDevice::get();
	audioDevice.deleteResource(m_engineAudio);
}

void Plane::setColor(float *initColor)
{
	for(int i = 0; i < 3; i++)
	{
		m_color[i] = initColor[i];
	}
}

void Plane::accelerate(float throttle)
{
	m_speed *= throttle;
	if(m_speed <= 0.01f)
		m_speed = 0.01f;
	if(m_speed > 0.11f)
		m_speed = 0.11f;

	m_engineAudio->setPitchMultiplier(1.0f + 2.5f*(m_speed-0.01f));
}

Vec3 &Plane::getPosition()
{
	return m_position;
}

void Plane::fire(Engine& engine)
{
	if (m_shootDelay > 0)
	{
		--m_shootDelay;
		return;
	}

	const auto& worldMat = getObjectToWorldMatrix();

	Vec3 xDir = worldMat.getColumn(0).getNormalized();
	Vec3 heading = worldMat.getColumn(1).getNormalized();

	Vec3 weaponOffset = (m_gun == true) ? (0.05f *xDir) : (-0.05f * xDir);

	// switch the active gun
	m_gun = !m_gun;

	auto bullet = std::make_unique <Bullet> (*this, m_position + weaponOffset, heading, engine);

	IAudioDevice& audioDevice = IAudioDevice::get();
	SAudioInitParams params;

	params.position = bullet->getPosition();
	params.decayDistance = 0.5f;
	params.gain = 0.3f;

	audioDevice.playResourceOnce(*s_assets.laserAudio, params);

	engine.addWorldEntity(std::move(bullet));

	m_shootDelay = 3;
}

void Plane::update(Engine& engine)
{
	auto& tile = engine.getWorld();
	uint16_t resolution = tile.getResolution();
	float ground = tile.getHeightAt(m_position.x(), m_position.y());

	const auto& worldMat = getObjectToWorldMatrix();
	Vec3 heading = worldMat.getColumn(1).getNormalized();

	Vec3 position = m_position + m_speed * heading;
	if(position.z() < ground)
	{
		position.setz(ground);
	}

	if(position.z() < 0.2f)
		position.setz(0.2f);
	if(position.y() > resolution)
		position.sety(resolution);
	if(position.x() > resolution)
		position.setx(resolution);
	if(position.y() < 0.0f)
		position.sety(0.0f);
	if(position.x() < 0.0f)
		position.setx(0.0f);
	if(position.z() > 15.0f)
		position.setz(15.0f);

	setPosition(position);

	Vec3 thead = m_speed * heading;

	m_engineAudio->setPosition(m_position);
	m_engineAudio->setVelocity(thead);

	if (s_assets.batch)
	{
		MeshInstanceData data;
		Matrix44 modelMatrix = getObjectToWorldMatrix();
		modelMatrix.copyData(data.modelMatrix);
		s_assets.batch->addMeshInstance(data);
	}
}

void Plane::roll(float froll)
{
	Quaternion qRoll(Vec3(0.0, 1.0, 0.0), froll);
	m_rotation = m_rotation * qRoll;
	m_rotation.normalize();
}

void Plane::pitch(float fpitch)
{
	Quaternion qPitch(Vec3(1.0, 0.0, 0.0), fpitch);
	m_rotation = m_rotation * qPitch;
	m_rotation.normalize();
}

CDynamicArrayBatch* Bullet::s_batch = nullptr;

Bullet::Bullet(WorldEntity& emitter, Vec3 position, Vec3 heading, Engine& engine)
	: WorldEntity(position)
	, m_heading(heading)
	, m_emitter(&emitter)
{
	if (!s_batch)
	{
		Renderer* renderer = engine.getRenderer();
        s_batch = renderer->addNewBatch <CDynamicArrayBatch>(CSceneRenderPass::eBulletPipeline);
	}
}

Bullet::~Bullet()
{
}

void Bullet::update(Engine& engine)
{
	auto& tile = engine.getWorld();
	uint16_t resolution = tile.getResolution();

	auto& entities = engine.getEntities();

	for (auto& entity : entities)
	{
		if (entity.get() == m_emitter || entity.get() == this)
			continue;

		if(m_position.x() > resolution || m_position.x() < 0 ||
		   m_position.y() > resolution || m_position.y() < 0 ||
		   m_position.z() > resolution || m_position.z() < 0)
		{
			m_flags |= eInactive;
		}

		Vec3 t = entity->getPosition() - m_position;
		if (cross(t, m_heading).length() <= 0.05f && t.length() < 1.0f)
		{
			m_flags |= eInactive;
			auto effect = std::make_unique <Explosion> (m_position);
			engine.addEffect(std::move(effect));
		}
	}

	m_position = m_position + 1.0f * m_heading;

    if (s_batch)
    {
        s_batch->addMeshData(nullptr, 0);
    }
}

PlaneAIController::PlaneAIController(Plane* plane)
	: m_plane(plane)
{
}

void PlaneAIController::update(Engine& engine)
{
	//find other plane and chase it
	const auto& worldMat = m_plane->getObjectToWorldMatrix();

	auto& target = engine.getPlayerEntity();

	Vec3 vToTarget = target.getPosition() - m_plane->getPosition();
	float l = vToTarget.normalize();
	float di = dot(vToTarget, worldMat.getColumn(0).getNormalized());
	float dj = dot(vToTarget, worldMat.getColumn(1).getNormalized());
	float dk = dot(vToTarget, worldMat.getColumn(2).getNormalized());

	if(dj >= 0.95f && l < 5.0f)
	{
		m_plane->fire(engine);
	}

	// acceleration
	float throttle = 1;
	if(dj < 0.0f && l < 5.0f)
	{
		throttle = powf(1.1f, 1);
	}
	else if(dj < 0.95f && l < 4.0f)
	{
		throttle = powf(1.1f, -1);
	}
	else {
		throttle = powf(1.1f, 1);
	}
	m_plane->accelerate(throttle);

	// roll
	if (di > 0.01f)
	{
		m_plane->roll(0.05f);
	}
	else if (di < -0.01f)
	{
		m_plane->roll(-0.05f);
	}

	// pitch
	if (dk > 0.01f || dj < 0.0f)
	{
		m_plane->pitch(0.05f);
	}
	else if (dk < -0.01f)
	{
		m_plane->pitch(-0.05f);
	}
}

PlanePlayerController::PlanePlayerController(Plane* plane)
	: m_plane(plane)
{
}

void PlanePlayerController::update(Engine& engine)
{
	const SUserInputState& inputState = engine.getInputState();

	const float fPitchSpeed = 0.5f * m_plane->getSpeed();
	const float fRollSpeed = 1.0f * m_plane->getSpeed();

	if(inputState.firePressed)
	{
		m_plane->fire(engine);
	}
	if(inputState.leftPressed)
	{
		m_plane->roll(-fRollSpeed);
	}
	if(inputState.rightPressed)
	{
		m_plane->roll(fRollSpeed);
	}
	if(inputState.upPressed)
	{
		m_plane->pitch(-fPitchSpeed);
	}
	if(inputState.downPressed)
	{
		m_plane->pitch(fPitchSpeed);
	}
	if(inputState.menuPressed)
	{
		return;
	}
	if(inputState.accelaratePressed)
	{
	}
	if(inputState.deccelaratePressed)
	{
	}
	if (inputState.accelarateTick != 0)
	{
		float throttle = powf(1.1f, static_cast<float> (inputState.accelarateTick));
		m_plane->accelerate(throttle);
	}
	if (fabs(inputState.left_right_value) > 0.1f)
	{
		m_plane->roll(inputState.left_right_value * fRollSpeed);
	}
	if (fabs(inputState.up_down_value) > 0.1f)
	{
		m_plane->pitch(-inputState.up_down_value * fRollSpeed);
	}
}
