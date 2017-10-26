#include "OSFactory.h"

class X11PathUtils : public OSUtils
{
protected:
	virtual std::string getRootPath() const override;
};
