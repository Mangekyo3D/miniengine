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
}

ResourceManager::~ResourceManager()
{
	m_device->finishJobs();
}

SMDModel* ResourceManager::loadModel(std::string modelName)
{
	auto iter = m_models.find(modelName);
	if (iter != m_models.end())
		return iter->second.get();

	auto& utils =  OSUtils::get();
	std::string finalFilename = utils.getModelPath() + modelName;

	auto model = SMDModel::openFromFile(this, finalFilename.c_str());

	if (!model)
		return nullptr;

	auto result = model.get();
	m_models[modelName] = std::move(model);
	return result;
}

ITexture* ResourceManager::loadTexture(std::string textureName)
{
	auto iter = m_textures.find(textureName);
	if (iter != m_textures.end())
		return iter->second.get();

	auto& utils =  OSUtils::get();
	std::string finalFileName = utils.getTexturePath() + textureName;

	BmpReader reader;
	if (!reader.openFromFile(finalFileName.data(), true))
		return nullptr;

	auto texture = m_device->createTexture(ITexture::EFormat::eSRGB8, ITexture::EUsage::eSampled, reader.getWidth(), reader.getHeight(), true);
	ITexture* result = texture.get();
	TextureStreamRequest req(result, finalFileName.data());
	m_device->addTextureStreamRequest(req);
	m_textures[textureName] = std::move(texture);
	return result;
}

IAudioResource *ResourceManager::loadAudio(std::string audioName)
{
	auto iter = m_audio.find(audioName);
	if (iter != m_audio.end())
		return iter->second.get();

	auto& utils =  OSUtils::get();
	std::string finalFileName = utils.getAudioPath() + audioName;

	auto audio = IAudioDevice::get().createAudioResource(finalFileName);
	IAudioResource* result = audio.get();
	m_audio[audioName] = std::move(audio);
	return result;
}
