#ifdef WIN32
#include "Win32Factory.h"
#include "Win32PathUtils.h"
#else
#include "X11Factory.h"
#include "X11PathUtils.h"
#endif
#include <memory>

std::unique_ptr <OSFactory> OSFactory::s_factory;

const OSFactory& OSFactory::get(){
	if (!s_factory){
		#ifdef WIN32
		auto newfactory = std::make_unique<Win32Factory> ();
		#else
		auto newfactory = std::make_unique<X11Factory> ();
		#endif

		s_factory = std::move(newfactory);
	}

	return *s_factory;
}


std::unique_ptr <OSUtils> OSUtils::s_utils;

const OSUtils& OSUtils::get()
{
	if (!s_utils){
		#ifdef WIN32
		auto newutils = std::make_unique<Win32PathUtils> ();
		#else
		auto newutils = std::make_unique<X11PathUtils> ();
		#endif
		newutils->initialize();
		s_utils = std::move(newutils);
	}

	return *s_utils;
}
