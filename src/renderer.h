#pragma once

#include <vector>
#include <memory>

class GameWindow;
class IDevice;
class Camera;
class CGPUBuffer;
class CBatch;
class Material;
struct Mesh;

class Renderer
{
public:
	Renderer();
	~Renderer();
	void initialize(GameWindow&, bool bDebugContext);
	void add_mesh_instance(Mesh*, Material *);
	void updateFrameUniforms(Camera& camera);
	void drawFrame();
	void setViewport(uint32_t width, uint32_t height);

	void shutdown();

private:
	/* batches that will be sent to GPU for rendering */
	std::vector <std::unique_ptr<CBatch> > m_batches;
	std::unique_ptr <IDevice> m_device;
	std::unique_ptr <CGPUBuffer> m_cameraUniform;
	std::unique_ptr <CGPUBuffer> m_lightUniform;

	uint32_t m_viewportWidth;
	uint32_t m_viewportHeight;
};
