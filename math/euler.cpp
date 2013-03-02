#include "euler.h"

#include "vector.h"

#include <cmath>

Vector EAngle::ToVector() const
{
	Vector result;

	result.x = cos(y)*cos(p);
	result.y = sin(p);
	result.z = sin(y)*cos(p);

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
