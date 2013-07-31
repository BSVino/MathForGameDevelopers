#pragma once

#include "vector.h"

class Quaternion
{
public:
	Quaternion() {};
	Quaternion(const Vector& n, float a);

public:
	const Quaternion Inverted() const;
	const Quaternion operator*(const Quaternion& q) const;
	const Vector operator*(const Vector& p) const;
	const Quaternion operator^(float t) const;
	const Quaternion Slerp(const Quaternion& r, float t) const;

	void ToAxisAngle(Vector& vecAxis, float& flAngle) const;

public:
	float w;
	Vector v; // x, y, z
};
