#pragma once

#include "vector.h"

// Rotation of a vector around an axis by theta degrees.
// https://www.youtube.com/watch?v=dttFiVn0rvc
const Vector RotateVectorAroundAxisAngle(const Vector& n, float a, const Vector& v)
{
	// a will be our theta.
	// We must convert degrees to radians. (This step wasn't shown in the video.)
	// 360 degrees == 2pi radians
	a = a /360 * (float)M_PI * 2;

	return v*cos(a) + (v.Dot(n)*n*(1-cos(a))) + (n.Cross(v)*sin(a));
}
