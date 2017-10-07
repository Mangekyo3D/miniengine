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
		virtual std::string getShaderPath() const = 0;
		virtual std::string getModelPath() const = 0;
		virtual std::string getTexturePath() const = 0;

		static const OSUtils& get();

	protected:
		OSUtils() {}
	private:
		static std::unique_ptr <OSUtils> s_utils;
};
