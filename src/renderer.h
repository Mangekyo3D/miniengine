#pragma once

#include <vector>
#include <memory>

class GameWindow;
class IDevice;
class Camera;
class CGPUBuffer;
class CBatch;
class Material;
struct IMesh;
class CTexture;

class Renderer
{
public:
	~Renderer();
	Renderer(const Renderer&) = delete;
	Renderer& operator = (const Renderer&) = delete;

	static Renderer& get() { return s_renderer; }

	void initialize(GameWindow&, bool bDebugContext);
	CBatch* addNewBatch(std::unique_ptr<CBatch> batch);
	void updateFrameUniforms(Camera& camera);
	void drawFrame();
	void setViewport(uint32_t width, uint32_t height);

	void shutdown();

private:
	Renderer();
	static Renderer s_renderer;

	/* batches that will be sent to GPU for rendering */
	std::vector <std::unique_ptr<CBatch> > m_batches;
	std::unique_ptr <IDevice> m_device;
	std::unique_ptr <CGPUBuffer> m_cameraUniform;
	std::unique_ptr <CGPUBuffer> m_lightUniform;

	uint32_t m_viewportWidth;
	uint32_t m_viewportHeight;
};
