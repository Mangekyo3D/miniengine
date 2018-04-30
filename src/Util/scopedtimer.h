#pragma once
#include <chrono>


class ScopedTimer
{
public:
	ScopedTimer(const char* title);
	ScopedTimer(const ScopedTimer&) = delete;
	ScopedTimer& operator = (const ScopedTimer&) = delete;
	~ScopedTimer();

private:
	const char* m_title;
	std::chrono::high_resolution_clock::time_point m_startTime;
};
