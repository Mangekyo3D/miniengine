#pragma once
#include <cmath>

inline float colorComponentSRGBToLinear(float value)
{
	return (value <= 0.04045f) ? value / 12.92f : pow((value + 0.055f) / 1.055f, 2.4f);
}

inline float colorComponentLinearToSRGB(float value)
{
	return (value <= 0.0031308f) ? 12.92f * value : (1.055f) * pow(value, 1.0f / 2.4f) - 0.055f;
}
