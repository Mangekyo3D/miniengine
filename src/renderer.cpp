#include "renderer.h"
#include "OS/GameWindow.h"
#include <iostream>
#include "device.h"
#include "camera.h"
#include "gpubuffer.h"
#include "batch.h"

struct SceneUniformBuffer
{
	float modelViewMatrix[16];
	float projectionMatrix[16];
	float normalMatrix[12];
};

struct LightUniformBuffer
{
	float color[4];
	Vec3 lightDir;
};

Renderer::Renderer ()
{
}

void Renderer::initialize(GameWindow& win, bool bDebugContext)
{
	m_device = IDevice::createDevice(win, bDebugContext);
	m_cameraUniform = std::make_unique <CGPUBuffer> (sizeof(SceneUniformBuffer));
	m_lightUniform = std::make_unique <CGPUBuffer> (sizeof(LightUniformBuffer));
}

Renderer::~Renderer()
{
	m_cameraUniform.reset();
}

void Renderer::add_mesh_instance(Mesh *mesh, Material *material)
{
	m_batches.push_back(std::make_unique <CBatch> (mesh, material));
}

void Renderer::updateFrameUniforms(Camera& camera)
{
	Matrix44 viewmat = camera.getViewMatrix();
	Matrix33 normalMat(viewmat);
	normalMat.invertFast();
	normalMat.transpose();

	if (SceneUniformBuffer* pBuffer = CGPUBuffer::CAutoLock <SceneUniformBuffer> (*m_cameraUniform))
	{
		Matrix44 projmat = camera.getProjectionMatrix();

		for (int i = 0; i < 16; ++i)
		{
			pBuffer->modelViewMatrix[i] = viewmat.getData()[i];
		}
		for (int i = 0; i < 16; ++i)
		{
			pBuffer->projectionMatrix[i] = projmat.getData()[i];
		}

		for (int i = 0; i < 3; ++i)
		{
			pBuffer->normalMatrix[i] = normalMat.getData()[i];
			pBuffer->normalMatrix[i + 4] = normalMat.getData()[i + 3];
			pBuffer->normalMatrix[i + 8] = normalMat.getData()[i + 6];
		}
	}

	if (LightUniformBuffer* pBuffer = CGPUBuffer::CAutoLock <LightUniformBuffer> (*m_lightUniform))
	{
		Vec3 lightDir = normalMat * Vec3(-1.0, -1.0, 1.0);
		lightDir.normalize();

		pBuffer->color[0] = 1.0f;
		pBuffer->color[1] = 0.8f;
		pBuffer->color[2] = 1.0f;
		pBuffer->color[3] = 1.0f;

		pBuffer->lightDir = lightDir;
	}
}


void Renderer::drawFrame()
{
	m_device->setViewport(m_viewportWidth, m_viewportHeight);
	m_device->clearFramebuffer(true);

	/* draw all batches */
	for (auto& batch : m_batches)
	{
		batch->draw(m_cameraUniform->getID(), m_lightUniform->getID());
	}

	/*
	// light position
	float sunHeight = sin(0.01*gtime);
	float sunX = cos(0.01*gtime);
	GLfloat lpos[] = {sunX, 1.0, sunHeight, 0.0};
	GLdouble clip[] = {0.0, 0.0, 1.0, 0.0};

	vec4 color;

	if(sunHeight > 0.6)
	{
		color = noonColor;
	}
	else if((sunHeight < 0.6) && (sunHeight > 0.2) && (sunX < 0))
	{
		color = noonColor*((sunHeight - 0.2)/0.4)
			+ duskColor*((0.6 - sunHeight)/0.4);
	}
	else if((sunHeight < 0.2) && (sunHeight > 0.0) && (sunX < 0))
	{
		color = duskColor*((sunHeight)/0.2)
			+ nightColor*((0.2 - sunHeight)/0.2);
		vec4 diff = fullColor*((sunHeight)/0.2)
			+ noColor*((0.2 - sunHeight)/0.2);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, (float *)&diff);
		glLightfv(GL_LIGHT0, GL_SPECULAR, (float *)&diff);

	}
	else if(sunHeight < 0.0)
	{
		color = nightColor;
		glLightfv(GL_LIGHT0, GL_DIFFUSE, (float *)&noColor);
		glLightfv(GL_LIGHT0, GL_SPECULAR, (float *)&noColor);
	}
	else if((sunHeight > 0.0) && (sunHeight < 0.6))
	{
		color = noonColor*((sunHeight)/0.6)
			+ nightColor*((0.6 - sunHeight)/0.6);
		vec4 diff = fullColor*((sunHeight)/0.6)
			+ noColor*((0.6 - sunHeight)/0.6);
		glLightfv(GL_LIGHT0, GL_DIFFUSE, (float *)&diff);
		glLightfv(GL_LIGHT0, GL_SPECULAR, (float *)&diff);
	}
	glClearColor(color.x, color.y, color.z, 0.0);
	glFogfv(GL_FOG_COLOR, (float *)&color);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//p1 (plane one) is the plane that generates the camera
	//(view) matrix
	player->generateCamera();

	glLightfv(GL_LIGHT0, GL_POSITION, lpos);

	player->draw();
	enemy->draw();
	for(int i = 0; i < bvec.size(); i++)
	    bvec[i].draw();

	glEnable(GL_TEXTURE_2D);

	glBindTexture(GL_TEXTURE_2D, pt);
	glClipPlane(GL_CLIP_PLANE0, clip);
	glEnable(GL_CLIP_PLANE0);
	glCallList(1);
	clip[2] = -1.0;

	glClipPlane(GL_CLIP_PLANE0, clip);
	glPushMatrix();

	glScalef(1.0, 1.0, -1.0);
	glFrontFace(GL_CW);
	glLightfv(GL_LIGHT0, GL_POSITION, lpos);

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuf);
		glClearColor(color.x, color.y, color.z, 0.0);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glCallList(1);
	glFrontFace(GL_CCW);
	glPopMatrix();
	glLightfv(GL_LIGHT0, GL_POSITION, lpos);
	glDisable(GL_CLIP_PLANE0);

		glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

	glNormal3f(0.0, 0.0, 1.0);

		glUseProgram(seaProgram);
		glUniform1f(timeUniform, gtime);
		glCallList(4);
		glUseProgram(0);


	for(int i = 0; i <evec.size(); i++)
	{
		vec4 v = evec[i]->getLocation();
		glPushMatrix();
		glTranslatef(v.x, v.y, v.z);
		glMultMatrixf(&billboardMatrix[0]);
		evec[i]->draw();
		glPopMatrix();
	}
	glDisable(GL_TEXTURE_2D);

	glFlush();
	gtime+=0.1;*/
}

void Renderer::setViewport(uint32_t width, uint32_t height)
{
	m_viewportHeight = height;
	m_viewportWidth = width;
}

void Renderer::shutdown()
{
	m_batches.clear();
}
