#pragma once
#include "OSFactory.h"

class Win32PathUtils : public OSUtils
{
public:
	std::string getRootPath() const override;
};
