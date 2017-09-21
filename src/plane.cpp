#include "plane.h"
#include "effect.h"
#include "SMDmodel.h"
#include "camera.h"
#include <vector>
#include "worldtile.h"
#include <cmath>
#include "engine.h"
#include "resourcemanager.h"

//ALuint Plane::buffer;
//ALuint Plane::engineBuf;

CBatch* Plane::s_batch = nullptr;

Plane::Plane(Vec3 initPos)
{
	m_position = initPos;

	for(int i = 0; i < 3; i++)
	{
		m_color[i] = 1.0f;
	}

	m_speed = 0.01f;
	m_shootDelay = 0;
	m_gun = true;

	if (!s_batch)
	{
		SMDModel* model = ResourceManager::get().loadModel("plane2.smd");
		if (model)
		{
			s_batch = model->getBatch();
		}
	}

	setScale(0.01f);

	/*
	alGenSources(1, &m_soundSource);
	alGenSources(1, &m_engineSource);

	alSourcei(m_soundSource, AL_BUFFER, buffer);
	alSourcef(m_soundSource, AL_REFERENCE_DISTANCE, 0.5);
	alSourcef(m_soundSource, AL_ROLLOFF_FACTOR, 1.0);
	alSourcef(m_engineSource, AL_GAIN, 0.3);


	alSourcei(m_soundSource, AL_BUFFER, engineBuf);
	alSourcef(m_soundSource, AL_REFERENCE_DISTANCE, 0.5);
	alSourcef(m_soundSource, AL_ROLLOFF_FACTOR, 0.1);
	alSourcef(m_soundSource, AL_GAIN, 0.3);
	alSourcef(m_soundSource, AL_MAX_GAIN, 0.3);
	alSourcei(m_soundSource, AL_LOOPING, AL_TRUE);

	alSourcePlay(m_engineSource);
*/
}

Plane::~Plane()
{
	stopSound();
}

void Plane::stopSound()
{
	/*
	alDeleteSources(1, &m_soundSource);
	alDeleteSources(1, &m_engineSource);
*/
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
	if(m_speed <= 0.0f)
		m_speed = 0.01f;
	if(m_speed > 0.11f)
		m_speed = 0.11f;
	//	alSourcef(m_engineSource, AL_PITCH, 1.0 + 2.5*(m_speed-0.01));
}

Vec3 &Plane::getPosition()
{
	return m_position;
}

void Plane::fire()
{
	if(m_shootDelay > 0)
	{
		--m_shootDelay;
		return;
	}

	auto worldMat = getObjectToWorldMatrix();

	Vec3 xDir = worldMat.getColumn(0).getNormalized();
	Vec3 heading = worldMat.getColumn(1).getNormalized();

	Vec3 weaponOffset = (m_gun == true) ? (0.05f *xDir) : (-0.05f * xDir);

	// switch the active gun
	m_gun = !m_gun;

	auto b1 = std::make_unique <Bullet> (m_position + weaponOffset, heading);
	b1->setEmitter(*this);

	auto& engine = Engine::get();
	engine.addWorldEntity(std::move(b1));

	/*
	alSourcefv(m_soundSource, AL_POSITION, (float *)&b1.getPosition());
	alSource3f(m_soundSource, AL_VELOCITY, 0.0, 0.0, 0.0);
	alSourcePlay(m_soundSource);
	*/

	m_shootDelay = 3;
}

void Plane::update()
{
	auto& engine = Engine::get();
	auto& tile = engine.getWorld();
	uint16_t resolution = tile.getResolution();
	float ground = tile.getHeightAt(m_position.x(), m_position.y());

	auto worldMat = getObjectToWorldMatrix();
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
	/*
	alSourcefv(m_engineSource, AL_POSITION, (float *)&m_position);
	alSourcefv(m_engineSource, AL_VELOCITY, (float*)&thead);
*/

	if (s_batch)
	{
		MeshInstanceData data;
		Matrix44 modelMatrix = getObjectToWorldMatrix();
		modelMatrix.getData(data.modelMatrix);
		s_batch->addMeshInstance(data);
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

void Plane::calculateAIpathfinding(Plane &target)
{
	//find other plane and chase it
	auto worldMat = getObjectToWorldMatrix();

	Vec3 d = target.getPosition() - m_position;
	float l = d.normalize();
	float di = dot(d, worldMat.getColumn(0).getNormalized());
	float dj = dot(d, worldMat.getColumn(1).getNormalized());
	float dk = dot(d, worldMat.getColumn(2).getNormalized());

	if(dj >= 0.95f && l < 5.0f)
	{
		fire();
	}
	if(dj < 0.0f && l < 5.0f)
	{
		accelerate(0.01f);
	}
	else if(dj < 0.95f && l < 4.0f)
	{
		accelerate(-0.01f);
	}
	else {
		accelerate(0.01f);
	}

	if (di > 0.01f)
	{
		roll(0.05f);
	}
	else if (di < -0.01f)
	{
		roll(-0.05f);
	}
	else if (dk > 0.01f || dj < 0.0f)
	{
		pitch(0.05f);
	}
	else if (dk < -0.01f)
	{
		pitch(-0.05f);
	}
}

Bullet::Bullet()
	: m_bActive(false)
{
}

Bullet::Bullet(Vec3 p, Vec3 h)
	: WorldEntity(p)
	, m_bActive(false)
	, m_heading(h)
{
}

Bullet::~Bullet()
{
}

void Bullet::draw()
{
	float l1pos[] = {m_position.x(), m_position.y(), m_position.z(), 1.0f};
	if (m_bActive)
	{
		/*		glDisable(GL_LIGHTING);
		glColor3f(1.0, 0.7, 0.3);
		glBegin(GL_LINES);
			glVertex3f(position.x, position.y, position.z);
			glVertex3f(position.x + 0.5*heading.x
				, position.y + 0.5*heading.y,
				position.z + 0.5*heading.z);
		glEnd();
		glEnable(GL_LIGHTING);
		glLightfv(GL_LIGHT1, GL_POSITION, l1pos);*/
	}
}

void Bullet::setActive(bool bActive)
{
	m_bActive = bActive;
}

void Bullet::setEmitter(Plane &p)
{
	m_emitter = &p;
}

void Bullet::move()
{
	if (m_bActive)
	{
		auto& engine = Engine::get();
		auto& tile = engine.getWorld();
		uint16_t resolution = tile.getResolution();

		auto& entities = engine.getEnities();

		for (auto& entity : entities)
		{
			if (entity.get() == m_emitter)
			{
				continue;
			}
			Vec3 t = entity->getPosition();

			if(m_position.x() > resolution || m_position.x() < 0 ||
			   m_position.y() > resolution || m_position.y() < 0 ||
			   m_position.z() > resolution || m_position.z() < 0)
			{
				setActive(false);
			}

			t = t - m_position;
			if (cross(t, m_heading).length() <= 0.05 && t.length() < 1.0f)
			{
				auto& engine = Engine::get();
				setActive(false);
				auto effect = std::make_unique <Explosion> (entity->getPosition());
				engine.addEffect(std::move(effect));
			}
		}

		m_position = m_position + 1.0f * m_heading;
	}
}


void Plane::initializeSound()
{
	//	buffer = alutCreateBufferFromFile("audio/laser.wav");
	//	engineBuf = alutCreateBufferFromFile("audio/engine.wav");
}

void Plane::cleanUpSound()
{
	//	alDeleteBuffers(1, &buffer);
	//	alDeleteBuffers(1, &engineBuf);
}
