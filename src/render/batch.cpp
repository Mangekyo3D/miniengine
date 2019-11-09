#include "batch.h"
#include "idevice.h"
#include "itexture.h"
#include "igpubuffer.h"
#include "icommandbuffer.h"

#include <cstring>
#include <algorithm>

CIndexedInstancedBatch::CIndexedInstancedBatch(IDevice& device, IMesh *m, enum EScenePipeline pipeline, const std::vector<ITexture *> *textures)
	: IBatch(pipeline)
	, m_numIndices(m->getNumIndices())
	, m_bShortIndices(m->getIndexSize() == sizeof(uint16_t))
	, m_numInstances(0)
{
	if (textures)
	{
		 m_textures = *textures;
	}

	m_vertexBuffer = device.createGPUBuffer(m->getVertexSize() * m->getNumVertices(), IGPUBuffer::Usage::eConstantVertex);
	if (auto lock = IGPUBuffer::CAutoLock<uint8_t>(*m_vertexBuffer))
	{
		uint8_t* ptr = lock;
		memcpy(ptr, m->getVertices(), m->getVertexSize() * m->getNumVertices());
	}

	m_indexBuffer = device.createGPUBuffer(m->getIndexSize() * m->getNumIndices(), IGPUBuffer::Usage::eIndex);
	if (auto lock = IGPUBuffer::CAutoLock<uint8_t>(*m_indexBuffer))
	{
		uint8_t* ptr = lock;
		memcpy(ptr, m->getIndices(), m->getIndexSize() * m->getNumIndices());
	}
}

CIndexedInstancedBatch::~CIndexedInstancedBatch()
{
}

void CIndexedInstancedBatch::draw(ICommandBuffer& cmd)
{
	if (m_instanceData.size() == 0)
	{
		return;
	}

	setupInstanceBuffer(cmd.getDevice());

	if (m_textures.size() > 0)
	{
		std::vector <SDescriptorSource> descriptorSources;
		std::for_each(m_textures.begin(), m_textures.end(), [&] (ITexture* tex)
		{
			descriptorSources.emplace_back(tex);
		});

		cmd.bindPerDrawDescriptors(descriptorSources.size(), descriptorSources.data());
	}

	cmd.setVertexStream(m_vertexBuffer.get(), m_instanceBuffer.get(), m_indexBuffer.get(), m_bShortIndices);

    cmd.drawIndexedInstanced(m_numIndices, 0, m_instanceData.size());

	m_instanceData.clear();
}

void CIndexedInstancedBatch::addMeshInstance(MeshInstanceData& instance)
{
	m_instanceData.push_back(instance);
}

void CIndexedInstancedBatch::setupInstanceBuffer(IDevice& device)
{
	// storage is immutable, so we have to reallocate
	if (m_instanceData.size() > m_numInstances)
	{
		m_instanceBuffer.reset();
	}

	if (!m_instanceBuffer)
	{
		m_instanceBuffer = device.createGPUBuffer(sizeof(MeshInstanceData) * m_instanceData.size(), IGPUBuffer::Usage::eConstantVertex);
		m_numInstances = static_cast <uint32_t> (m_instanceData.size());
	}

	if (auto lock = IGPUBuffer::CAutoLock<MeshInstanceData>(*m_instanceBuffer))
	{
		MeshInstanceData* ptr = lock;
		memcpy(ptr, m_instanceData.data(), sizeof(MeshInstanceData) * m_instanceData.size());
	}
}

CDynamicArrayBatch::CDynamicArrayBatch(IDevice&, enum EScenePipeline pipeline, const std::vector<ITexture *> *textures)
	: IBatch(pipeline)
{
	if (textures)
	{
		m_textures = *textures;
	}
}

CDynamicArrayBatch::~CDynamicArrayBatch()
{
}

void CDynamicArrayBatch::draw(ICommandBuffer& commandBuffer)
{
    IDevice& device = commandBuffer.getDevice();
    std::unique_ptr<IGPUBuffer> buffer = device.createGPUBuffer(myData.size() * sizeof(float), IGPUBuffer::eConstantVertex);


}

void CDynamicArrayBatch::addMeshData(float *data, uint32_t numFloats)
{
    myData.reserve(myData.size() + numFloats);
    for (uint32_t i = 0; i < numFloats; ++i)
    {
        myData.push_back(data[i]);
    }
}
