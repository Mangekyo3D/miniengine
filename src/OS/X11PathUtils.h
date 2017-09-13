#include "OSFactory.h"

class X11PathUtils : public OSUtils
{
public:
	virtual std::string getShaderPath() const override;
	virtual std::string getModelPath() const override;
};
