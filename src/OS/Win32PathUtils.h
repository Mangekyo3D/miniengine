#pragma once
#include "OSFactory.h"

class Win32PathUtils : public OSUtils
{
public:
	virtual std::string getShaderPath() const override;
	virtual std::string getModelPath() const override;
	virtual std::string getTexturePath() const override;
};
