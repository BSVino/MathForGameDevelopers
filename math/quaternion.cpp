#include "quaternion.h"

#include <cmath>

// Building a quaternion from an axis-angle rotation.
// http://youtu.be/SCbpxiCN0U0
Quaternion::Quaternion(const Vector& n, float a)
{
	// a will be our theta.
	// We must convert degrees to radians. (This step wasn't shown in the video.)
	// 360 degrees == 2pi radians
	a = a /360 * (float)M_PI * 2;

	w = cos(a/2);

	v = n * sin(a/2);
}

// http://youtu.be/A6A0rpV9ElA
const Quaternion Quaternion::Inverted() const
{
	Quaternion q;
	q.w = w;
	q.v = -v;
	return q;
}

// Multiplying two quaternions together combines the rotations.
// http://youtu.be/CRiR2eY5R_s
const Quaternion Quaternion::operator*(const Quaternion& q) const
{
	Quaternion r;

	r.w = w*q.w + v.Dot(q.v);
	r.v = v*q.w + q.v*w + v.Cross(q.v);

	return r;
}

// Rotate a vector with this quaternion.
// http://youtu.be/Ne3RNhEVSIE
// The basic equation is qpq* (the * means inverse) but we use a simplified version of that equation.
const Vector Quaternion::operator*(const Vector& V) const
{
	Quaternion p;
	p.w = 0;
	p.v = V;

	// Could do it this way:
	/*
	const Quaternion& q = (*this);
	return (q * p * q.Inverted()).v;
	*/

	// But let's optimize it a bit instead.
	Vector vcV = v.Cross(V);
	return V + vcV*(2*w) + v.Cross(vcV)*2;
}
