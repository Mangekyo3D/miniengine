#include "resourcemanager.h"
#include "SMDmodel.h"
#include "render/batch.h"
#include "render/itexture.h"
#include "OS/OSFactory.h"
#include "audiointerface.h"
#include "render/idevice.h"
#include "render/ipipeline.h"
#include "bitmap.h"

ResourceManager::ResourceManager(IDevice* device)
	: m_device(device)
{
	loadDefaultPipelines();
}

ResourceManager::~ResourceManager()
{
}

SMDModel* ResourceManager::loadModel(std::string modelName)
{
	auto iter = m_models.find(modelName);
	if (iter != m_models.end())
	{
		return iter->second.get();
	}
	else
	{
		auto& utils =  OSUtils::get();
		std::string finalFilename = utils.getModelPath() + modelName;

		auto model = std::make_unique <SMDModel> ();

		if (model->openFromFile(this, finalFilename.c_str()))
		{
			SMDModel* result = model.get();
			m_models[modelName] = std::move(model);
			return result;
		}
		else return nullptr;
	}
}

IPipeline* ResourceManager::loadPipeline(EPipelines pipelineName)
{
	return m_pipelines[pipelineName].get();
}

ITexture* ResourceManager::loadTexture(std::string textureName)
{
	auto iter = m_textures.find(textureName);
	if (iter != m_textures.end())
	{
		return iter->second.get();
	}
	else
	{
		auto& utils =  OSUtils::get();
		std::string finalFileName = utils.getTexturePath() + textureName;

		BmpReader reader;
		if (reader.openFromFile(finalFileName.data(), true))
		{
			auto texture = m_device->createTexture(ITexture::EFormat::eSRGB8, reader.getWidth(), reader.getHeight(), true);
			ITexture* result = texture.get();
			TextureStreamRequest req(result, finalFileName.data());
			m_device->addTextureStreamRequest(req);
			m_textures[textureName] = std::move(texture);
			return result;
		}
		else
		{
			return nullptr;
		}
	}
}

IAudioResource *ResourceManager::loadAudio(std::string audioName)
{
	auto iter = m_audio.find(audioName);
	if (iter != m_audio.end())
	{
		return iter->second.get();
	}
	else
	{
		auto& utils =  OSUtils::get();
		std::string finalFileName = utils.getAudioPath() + audioName;

		auto audio = IAudioDevice::get().createAudioResource(finalFileName);
		IAudioResource* result = audio.get();
		m_audio[audioName] = std::move(audio);
		return result;
	}
}

void ResourceManager::loadDefaultPipelines()
{
	SPipelineParams params;
	params.m_flags = eReverseDepth | eCullBackFace;

	const char* pipelineName = "generic";

	{
		SVertexBinding vertBinding(sizeof(VertexFormatVN));
		vertBinding.addAttribute(offsetof(VertexFormatVN, vertex), eFloat, 3);
		vertBinding.addAttribute(offsetof(VertexFormatVN, normal), e1010102int, 4);

		auto pipeline = m_device->createPipeline(params, &vertBinding, nullptr, pipelineName);
		m_pipelines[eDiffuse] = std::move(pipeline);
	}

	{
		SVertexBinding vertBinding(sizeof(VertexFormatVNT));
		vertBinding.addAttribute(offsetof(VertexFormatVNT, vertex), eFloat, 3);
		vertBinding.addAttribute(offsetof(VertexFormatVNT, normal), e1010102int, 4);
		vertBinding.addAttribute(offsetof(VertexFormatVNT, texCoord), eFloat, 2);

		pipelineName = "genericTextured";
		auto pipeline = m_device->createPipeline(params, &vertBinding, nullptr, pipelineName);
		m_pipelines[eDiffuseTextured] = std::move(pipeline);

		params.m_flags |= ePrimitiveRestart;
		pipeline = m_device->createPipeline(params, &vertBinding, nullptr, pipelineName);
		m_pipelines[eDiffuseTexturedPrimRestart] = std::move(pipeline);
	}

	{
		SVertexBinding vertBinding(sizeof(VertexFormatV));
		vertBinding.addAttribute(offsetof(VertexFormatV, vertex), eFloat, 3);
		params.m_flags = 0;
		pipelineName = "toneMapping";
		auto pipeline = m_device->createPipeline(params, &vertBinding, nullptr, pipelineName);
		m_pipelines[eToneMapping] = std::move(pipeline);
	}
}
