#pragma once
#include "GameWindow.h"
#include <memory>
#include <string>

class OSFactory
{
	public:
		virtual std::unique_ptr <GameWindow> createGameWindow() const = 0;
		virtual ~OSFactory(){}
		static const OSFactory& get();
	protected:
		OSFactory(){}
	private:
		static std::unique_ptr <OSFactory> s_factory;
};

class OSUtils
{
	public:
	const std::string& getShaderPath() const { return m_shaderPath; }
	const std::string& getModelPath() const { return m_modelPath; }
	const std::string& getTexturePath() const { return m_texturePath; }
	const std::string& getAudioPath() const { return m_audioPath; }

	static const OSUtils& get();

	virtual ~OSUtils() {};
	protected:
		virtual std::string getRootPath() const = 0;
		OSUtils() {}

		void initialize() {
			auto rootPath = getRootPath();
			m_audioPath = rootPath + "/audio/";
			m_texturePath = rootPath + "/textures/";
			m_modelPath = rootPath + "/models/";
			m_shaderPath = rootPath + "/shaders/";
		}
	private:
		static std::unique_ptr <OSUtils> s_utils;
		std::string m_texturePath;
		std::string m_audioPath;
		std::string m_modelPath;
		std::string m_shaderPath;
};
