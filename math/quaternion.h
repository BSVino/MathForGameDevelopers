#pragma once

#include "vector.h"

class Quaternion
{
public:
	Quaternion(const Vector& n, float a);

public:
	float w, x, y, z;
};
