#include "Win32PathUtils.h"
#include <Windows.h>
#include <array>

std::string Win32PathUtils::getRootPath() const
{
	HMODULE module = GetModuleHandle(nullptr);

	std::array <char, MAX_PATH> path;
	GetModuleFileName(module, path.data(), static_cast <DWORD> (path.size()));

	std::string pathStr(path.data());
	size_t pos = pathStr.rfind('\\');
	// no directory found, just return root
	if (pos == std::string::npos || pos == 0)
		return "\\";
	pos = pathStr.rfind('\\', pos - 1);
	if (pos == std::string::npos)
		return "\\";

	return pathStr.substr(0, pos);
}
