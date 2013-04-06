#pragma once

#include "vector.h"

class AABB
{
public:
	AABB operator+(const Point& p) const
	{
		AABB result = (*this);
		result.vecMin = p + vecMin;
		result.vecMax = p + vecMax;
		return result;
	}

	float GetHeight() const
	{
		return vecMax.y-vecMin.y;
	}

public:
	Vector vecMin;
	Vector vecMax;
};
