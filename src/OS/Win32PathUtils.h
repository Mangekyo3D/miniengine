#pragma once
#include "OSFactory.h"

class Win32PathUtils : public OSUtils
{
public:
	virtual std::string getRootPath() const override;
};
