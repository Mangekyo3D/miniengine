#include "icommandbuffer.h"
#include "irenderpass.h"

ICommandBuffer::CScopedRenderPass::CScopedRenderPass(ICommandBuffer& cmd, IRenderPass& renderpass, const float* vClearColor, const float* clearDepth)
	: m_renderpass(&renderpass)
	, m_cmd(&cmd)
{
	m_cmd->beginRenderPass(renderpass, vClearColor, clearDepth);
}

ICommandBuffer::CScopedRenderPass::~CScopedRenderPass()
{
	m_cmd->endRenderPass();
}
