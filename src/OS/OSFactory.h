#pragma once
#include "GameWindow.h"
#include <memory>
#include <string>

class OSFactory
{
	public:
		virtual std::unique_ptr <GameWindow> createGameWindow(bool bDebugContext) const = 0;
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
	std::string getShaderPath() const
	{
		return getRootPath() + "/shaders/";
	}

	std::string getModelPath() const
	{
		return getRootPath() + "/models/";
	}

	std::string getTexturePath() const
	{
		return getRootPath() + "/textures/";
	}

	static const OSUtils& get();

	protected:
		virtual std::string getRootPath() const = 0;
		OSUtils() {}

	private:
		static std::unique_ptr <OSUtils> s_utils;
};
