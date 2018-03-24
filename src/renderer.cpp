#include "renderer.h"
#include "OS/GameWindow.h"
#include <iostream>
#include "render/device.h"
#include "render/gpubuffer.h"
#include "camera.h"
#include "batch.h"
#include "resourcemanager.h"

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


class Renderer : public IRenderer
{
	public:
		Renderer(GameWindow& win, bool bDebugContext, bool bVulkanContext);
		~Renderer();
		Renderer(const Renderer&) = delete;
		Renderer& operator = (const Renderer&) = delete;

		virtual void addNewBatch(std::unique_ptr<IBatch> batch) override;

		virtual void updateFrameUniforms(Camera& camera);
		virtual void drawFrame();

		virtual void setViewport(uint32_t width, uint32_t height);
		static void shutdown();

	private:
		/* batches that will be sent to GPU for rendering */
		std::vector <std::unique_ptr<IBatch> > m_batches;

		std::unique_ptr<IGPUBuffer> m_cameraUniform;
		std::unique_ptr<IGPUBuffer> m_lightUniform;

		std::unique_ptr <CCompositingPipeline> m_compositor;
		std::unique_ptr <IDevice> m_device;
};

Renderer::Renderer (GameWindow& win, bool bDebugContext, bool bVulkanContext)
{
	m_device = IDevice::createDevice(win, bDebugContext, bVulkanContext);

	// it is important to load the default pipelines here so that compositor can be initialized properly
	ResourceManager& resourceManager = ResourceManager::get();
	resourceManager.loadDefaultPipelines();

	m_compositor = std::make_unique <CCompositingPipeline>();

	m_cameraUniform = m_device->createGPUBuffer(sizeof(SceneUniformBuffer));
	m_lightUniform = m_device->createGPUBuffer(sizeof(LightUniformBuffer));
}

Renderer::~Renderer()
{
	// custom destructor. ORDER IS IMPORTANT!

	// make sure all render data are released before the device is
	m_batches.clear();
	m_cameraUniform.reset();
	m_lightUniform.reset();

	// delete the compositing pipeline
	m_compositor.reset();

	// finally, cleanup the device
	m_device.reset();
}

void Renderer::addNewBatch(std::unique_ptr<IBatch> batch)
{
	m_batches.push_back(std::move (batch));
}

void Renderer::updateFrameUniforms(Camera& camera)
{
	Matrix44 viewmat = camera.getViewMatrix();
	Matrix33 normalMat(camera.getViewMatrixInverse());
	normalMat.transpose();

	if (auto lock = IGPUBuffer::CAutoLock <SceneUniformBuffer> (*m_cameraUniform))
	{
		SceneUniformBuffer* pBuffer = lock;
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

	if (auto lock = IGPUBuffer::CAutoLock <LightUniformBuffer> (*m_lightUniform))
	{
		LightUniformBuffer* pBuffer = lock;
		Vec3 lightDir = normalMat * Vec3(-1.0, -1.0, 1.0);
		lightDir.normalize();

		pBuffer->color[0] = 1.0f;
		pBuffer->color[1] = 1.0f;
		pBuffer->color[2] = 1.0f;
		pBuffer->color[3] = 1.0f;

		pBuffer->lightDir = lightDir;
	}
}


void Renderer::drawFrame()
{
	m_compositor->draw(m_batches, *m_cameraUniform, *m_lightUniform);
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

	gtime+=0.1;*/
}

void Renderer::setViewport(uint32_t width, uint32_t height)
{
	m_compositor->resize(width, height);
}


std::unique_ptr<IRenderer> IRenderer::create(GameWindow& win, bool bDebugContext, bool bVulkanContext)
{
	return std::make_unique <Renderer> (win, bDebugContext, bVulkanContext);
}

