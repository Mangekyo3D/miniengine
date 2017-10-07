#include "effect.h"

using namespace std;
/*
ALuint Explosion::soundBuffer;
*/
uint32_t Explosion::texture[3];

Explosion::Explosion(Vec3 l)
	: Effect(l)
{
	m_tex = texture[0];
	/*
	alGenSources(1, &sound);
	alSourcei(sound, AL_BUFFER, soundBuffer);
	alSource3f(sound, AL_POSITION, m_location.x(), m_location.y(), m_location.z());
	alSourcef(sound, AL_ROLLOFF_FACTOR, 0.5);

	alSourcePlay(sound);
	*/
}

Explosion::~Explosion()
{ 
//	alDeleteSources(1, &sound); 
}

void Explosion::draw()
{
/*
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex2f(-0.2, -0.2);
	glTexCoord2f(1.0, 0.0);
	glVertex2f(0.2, -0.2);
	glTexCoord2f(1.0, 1.0);
	glVertex2f(0.2, 0.2);
	glTexCoord2f(0.0, 1.0);
	glVertex2f(-0.2, 0.2);
	glEnd();
	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
*/
}

void Explosion::update()
{
	m_timer++;
	if(m_timer > 5)
		m_tex = texture[1];
	if(m_timer > 10)
		m_tex = texture[2];
	if(m_timer > 15)
		setActive(false);
}

void Explosion::initializeSound()
{
	/*
	soundBuffer = alutCreateBufferFromFile("audio/explosion.wav");
	*/
}

void Explosion::cleanUpSound()
{
	/*
	alDeleteBuffers(1, &soundBuffer);
	*/
}
