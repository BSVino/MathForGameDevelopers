#pragma once

#include "vector.h"

class Quaternion
{
public:
	Quaternion() {};
	Quaternion(const Vector& n, float a);

public:
	const Quaternion Inverted() const;

public:
	float w, x, y, z;
};
