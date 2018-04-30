#include "scopedtimer.h"
#include <iostream>

ScopedTimer::ScopedTimer(const char* title)
	: m_title(title)
	, m_startTime(std::chrono::high_resolution_clock::now())
{
}

ScopedTimer::~ScopedTimer()
{
	std::chrono::high_resolution_clock::time_point endTime = std::chrono::high_resolution_clock::now();
	std::chrono::duration <double> timePassed = std::chrono::duration_cast <std::chrono::duration <double>> (endTime - m_startTime);
	std::cout << "Seconds elapsed for " << m_title << ": " << timePassed.count() << std::endl;
}
