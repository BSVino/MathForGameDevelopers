#include "vector.h"

// https://www.youtube.com/watch?v=rqhAOc9gvC4
Vector PredictProjectileAtTime(float t, Vector v0, Vector x0, Vector g)
{
	return g * (0.5f * t * t) + v0 * t + x0;
}

float PredictProjectileMaximumHeightTime(Vector v0, Vector g)
{
	return -v0.y / g.y;
}
