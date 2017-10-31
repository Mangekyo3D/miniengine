#include "mathutils.h"
#define _USE_MATH_DEFINES
#include <cmath>

float degreesToRads(float angle)
{
	return static_cast <float> (angle * M_PI / 180.0);
}
