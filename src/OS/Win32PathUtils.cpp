#include "Win32PathUtils.h"
#include <windows.h>
#include <array>

std::string Win32PathUtils::getShaderPath() const
{
	HMODULE module = GetModuleHandle(nullptr);

	std::array <char, MAX_PATH> path;
	GetModuleFileName(module, path.data(), static_cast <DWORD> (path.size()));

	std::string pathStr(path.data());
	size_t pos = pathStr.find_last_of('\\');

	// no directory found, just return root
	if (pos == std::string::npos)
	{
		return "\\";
	}

	return pathStr.substr(0, pos) + "\\shaders\\";
}

std::string Win32PathUtils::getModelPath() const
{
	HMODULE module = GetModuleHandle(nullptr);

	std::array <char, MAX_PATH> path;
	GetModuleFileName(module, path.data(), static_cast <DWORD> (path.size()));

	std::string pathStr(path.data());
	size_t pos = pathStr.find_last_of('\\');

	// no directory found, just return root
	if (pos == std::string::npos)
	{
		return "\\";
	}

	return pathStr.substr(0, pos) + "\\models\\";
}

std::string Win32PathUtils::getTexturePath() const
{
	HMODULE module = GetModuleHandle(nullptr);

	std::array <char, MAX_PATH> path;
	GetModuleFileName(module, path.data(), static_cast <DWORD> (path.size()));

	std::string pathStr(path.data());
	size_t pos = pathStr.find_last_of('\\');

	// no directory found, just return root
	if (pos == std::string::npos)
	{
		return "\\";
	}

	return pathStr.substr(0, pos) + "\\textures\\";
}
