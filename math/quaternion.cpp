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

	x = n.x*sin(a/2);
	y = n.y*sin(a/2);
	z = n.z*sin(a/2);
}

// http://youtu.be/A6A0rpV9ElA
const Quaternion Quaternion::Inverted() const
{
	Quaternion q;
	q.w = w;
	q.x = -x;
	q.y = -y;
	q.z = -z;
	return q;
}
