#include "X11PathUtils.h"
#include <array>
#include <sys/types.h>
#include <unistd.h>
#include <sys/stat.h>
#include <limits.h>
#include <stdio.h>
#include <cstdio>

std::string X11PathUtils::getShaderPath() const
{
	std::array <char, PATH_MAX> path = {0};
	std::array <char, PATH_MAX> dest = {0};
	pid_t pid = getpid();

	std::snprintf(path.data(), path.size(), "/proc/%d/exe", pid);

	if (readlink(path.data(), dest.data(), path.size()) == -1)
	{
		return "\\";
	}

	std::string pathStr(dest.data());
	size_t pos = pathStr.find_last_of('/');

	// no directory found, just return root
	if (pos == std::string::npos)
	{
		return "\\";
	}

	return pathStr.substr(0, pos) + "/shaders/";
}

std::string X11PathUtils::getModelPath() const
{
	std::array <char, PATH_MAX> path = {0};
	std::array <char, PATH_MAX> dest = {0};
	pid_t pid = getpid();

	std::snprintf(path.data(), path.size(), "/proc/%d/exe", pid);

	if (readlink(path.data(), dest.data(), path.size()) == -1)
	{
		return "\\";
	}

	std::string pathStr(dest.data());
	size_t pos = pathStr.find_last_of('/');

	// no directory found, just return root
	if (pos == std::string::npos)
	{
		return "\\";
	}

	return pathStr.substr(0, pos) + "/shaders/";
}
