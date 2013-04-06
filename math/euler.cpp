#include "euler.h"

#include "vector.h"

#define _USE_MATH_DEFINES
#include <cmath>

Vector EAngle::ToVector() const
{
	Vector result;

	// Convert from degrees to radians.
	float y2 = y/360*2*M_PI;
	float p2 = p/360*2*M_PI;

	result.x = cos(y2)*cos(p2);
	result.y = sin(p2);
	result.z = sin(y2)*cos(p2);

	return result;
}

void EAngle::Normalize()
{
	if (p > 89)
		p = 89;
	if (p < -89)
		p = -89;

	while (y < -180)
		y += 360;
	while (y > 180)
		y -= 360;
}
