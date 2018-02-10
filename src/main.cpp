#include <vector>
#include <cmath>
#include "plane.h"
#include "Util/perlin.h"
#include "effect.h"
#include "audiointerface.h"
#include "SMDmodel.h"
#include <memory>
#include <iostream>
#include "engine.h"
#include <cstring>

//const Vec4 noonColor(0.3, 0.6, 0.7, 1.0);
//const vec4 duskColor(0.5, 0.2, 0.1, 1.0);
//const vec4 nightColor(0.15, 0.15, 0.3, 1.0);
//const vec4 noColor(0.0, 0.0, 0.0, 1.0);
//const vec4 fullColor(1.0, 1.0, 1.0, 1.0);

void interpretCommandLineOptions(int argc, char** argv, SCommandLineOptions& options)
{
	for (int i = 0; i < argc; ++i)
	{
		if (strcmp(argv[i], "--debug-gpu") == 0)
		{
			options.bDebugContext = true;
		}
	}
}

int main(int argc, char** argv)
{
	SCommandLineOptions options;
	interpretCommandLineOptions(argc, argv, options);

	auto& engine = Engine::get();
	engine.startup(options);

	// ALuint music, msrc;

	//alDistanceModel(AL_INVERSE_DISTANCE_CLAMPED);
	//alSpeedOfSound(3.0);
	//alDopplerFactor(4.0);

	//	Plane::initializeSound();
	//	Explosion::initializeSound();
	/*
	   music = alutCreateBufferFromFile("audio/lvl1.wav");
	   alGenSources(1, &msrc);
	   alSourcei(msrc, AL_BUFFER, music);
	   alSourcef(msrc, AL_GAIN, 1.0);
	   alSourcei(msrc, AL_LOOPING, AL_TRUE);
	   alSourcePlay(msrc);
	*/

	float plane1color[] = {0.8f, 0.8f, 0.8f};
	float plane2color[] = {0.0f, 0.4f, 0.4f};

	auto p1 = std::make_unique <Plane>(Vec3(32, 3, 1.0));
	auto p2 = std::make_unique <Plane>(Vec3(32, 61, 11.0));

	auto controller1 = std::make_unique <PlanePlayerController>(p1.get());
	auto controller2 = std::make_unique <PlaneAIController>(p2.get());

	p1->setColor(plane1color);
	p2->setColor(plane2color);

	engine.setPlayerEntity(p1.get());
	engine.addWorldEntity(std::move(p1));
	engine.addWorldEntity(std::move(p2));
	engine.addController(std::move(controller1));
	engine.addController(std::move(controller2));

	/* program main loop */
	engine.enterGameLoop();

	//	alDeleteSources(1, &msrc);
	//	alDeleteBuffers(1, &music);
	Plane::cleanUpSound();
	Explosion::cleanUpSound();
}

void CreateLists()
{
	/*
#define GRASS_SCALE 0.5
	//The ground
	glNewList(1, GL_COMPILE);
	glColor3f(1.0, 1.0, 1.0);
	for(int i = 0; i < HMAPSIDE-1; i++)
	{
	glBegin(GL_TRIANGLE_STRIP);
	glNormal3fv(normalMap[i][0]);
	glTexCoord2f(GRASS_SCALE*i, 0);
	glVertex3f(i, 0, heightMap[i][0]);
	glNormal3fv(normalMap[i+1][0]);
	glTexCoord2f(GRASS_SCALE*(i+1), 0);
	glVertex3f(i+1, 0, heightMap[i+1][0]);
	for(int j = 0; j < HMAPSIDE-1; j++)
	{
	glNormal3fv(normalMap[i][j+1]);
	glTexCoord2f(GRASS_SCALE*i, GRASS_SCALE*(j+1));
	glVertex3f(i, j+1, heightMap[i][j+1]);
	glNormal3fv(normalMap[i+1][j+1]);
	glTexCoord2f(GRASS_SCALE*(i+1), GRASS_SCALE*(j+1));
	glVertex3f(i+1, j+1, heightMap[i+1][j+1]);
	}
	glEnd();
	}
	glEndList();

	//The sun
	glNewList(3, GL_COMPILE);
	glBindTexture(GL_TEXTURE_2D, tex[1]);
	glEnable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	glDepthMask(GL_FALSE);
	glEnable(GL_BLEND);
	glDisable(GL_FOG);
	glDisable(GL_LIGHTING);
	glBegin(GL_QUADS);
	glTexCoord2f(0.0, 0.0);
	glVertex2f(-2, -2);
	glTexCoord2f(1.0, 0.0);
	glVertex2f(2, -2);
	glTexCoord2f(1.0, 1.0);
	glVertex2f(2, 2);
	glTexCoord2f(0.0, 1.0);
	glVertex2f(-2, 2);
	glEnd();
	glDisable(GL_BLEND);
	glEnable(GL_LIGHTING);
	glEnable(GL_FOG);
	glDepthMask(GL_TRUE);
	glDisable(GL_TEXTURE_2D);
	glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glEndList();

	//The sea
	glNewList(4, GL_COMPILE);
	glBindTexture(GL_TEXTURE_2D, tex[0]);
	glBegin(GL_QUADS);
	for(int i = 0; i < HMAPSIDE - 1; i+=2)
	{
	for(int j = 0; j < HMAPSIDE - 1; j+=2)
	{
	glTexCoord2f(i, j);
	glVertex2f(i, j);
	glTexCoord2f(2.0 + i, j);
	glVertex2f(i+2, j);
	glTexCoord2f(2.0 + i, 2.0 + j);
	glVertex2f(i+2, j+2);
	glTexCoord2f(i, 2.0 + j);
	glVertex2f(i, j+2);
	}
}
glEnd();
glEndList();*/
}
