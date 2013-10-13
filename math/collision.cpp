/*
Copyright (c) 2012, Lunar Workshop, Inc.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
3. All advertising materials mentioning features or use of this software must display the following acknowledgement:
   This product includes software developed by Lunar Workshop, Inc.
4. Neither the name of the Lunar Workshop nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY LUNAR WORKSHOP INC ''AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL LUNAR WORKSHOP BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "collision.h"

#include <utility>
#include <algorithm>

#include "aabb.h"
#include "vector.h"

using std::swap;
using std::min;
using std::max;

bool ClipLine(int d, const AABB& aabbBox, const Vector& v0, const Vector& v1, float& f_low, float& f_high)
{
	// f_low and f_high are the results from all clipping so far. We'll write our results back out to those parameters.

	// f_dim_low and f_dim_high are the results we're calculating for this current dimension.
	float f_dim_low, f_dim_high;

	// Find the point of intersection in this dimension only as a fraction of the total vector http://youtu.be/USjbg5QXk3g?t=3m12s
	f_dim_low = (aabbBox.vecMin.v[d] - v0.v[d])/(v1.v[d] - v0.v[d]);
	f_dim_high = (aabbBox.vecMax.v[d] - v0.v[d])/(v1.v[d] - v0.v[d]);

	// Make sure low is less than high
	if (f_dim_high < f_dim_low)
		swap(f_dim_high, f_dim_low);

	// If this dimension's high is less than the low we got then we definitely missed. http://youtu.be/USjbg5QXk3g?t=7m16s
	if (f_dim_high < f_low)
		return false;

	// Likewise if the low is less than the high.
	if (f_dim_low > f_high)
		return false;

	// Add the clip from this dimension to the previous results http://youtu.be/USjbg5QXk3g?t=5m32s
	f_low = max(f_dim_low, f_low);
	f_high = min(f_dim_high, f_high);

	if (f_low > f_high)
		return false;

	return true;
}

// Find the intersection of a line from v0 to v1 and an axis-aligned bounding box http://www.youtube.com/watch?v=USjbg5QXk3g
bool LineAABBIntersection(const AABB& aabbBox, const Vector& v0, const Vector& v1, Vector& vecIntersection, float& flFraction)
{
	float f_low = 0;
	float f_high = 1;

	if (!ClipLine(0, aabbBox, v0, v1, f_low, f_high))
		return false;

	if (!ClipLine(1, aabbBox, v0, v1, f_low, f_high))
		return false;

	if (!ClipLine(2, aabbBox, v0, v1, f_low, f_high))
		return false;

	// The formula for I: http://youtu.be/USjbg5QXk3g?t=6m24s
	Vector b = v1 - v0;
	vecIntersection = v0 + b * f_low;

	flFraction = f_low;

	return true;
}

// Find the intersection of two axis-aligned bounding boxes http://youtu.be/ENuk9HgeTiI
bool AABBIntersection(const AABB& a, const AABB& b)
{
	for (int i = 0; i < 3; i++)
	{
		if (a.vecMin.v[i] > b.vecMax.v[i])
			return false;
		if (a.vecMax.v[i] < b.vecMin.v[i])
			return false;
	}

	return true;
}

// Line-Plane Intersection: http://youtu.be/fIu_8b2n8ZM
bool LinePlaneIntersection(const Vector& n, const Vector& c, const Vector& x0, const Vector& x1, Vector& vecIntersection, float& flFraction)
{
	// n - plane normal
	// c - any point in the plane
	// x0 - the beginning of our line
	// x1 - the end of our line

	Vector v = x1 - x0;
	Vector w = c - x0;

	float k = w.Dot(n)/v.Dot(n);

	vecIntersection = x0 + k * v;

	flFraction = k;

	return k >= 0 && k <= 1;
}

const Vector NearestPointOnSphere(const Vector& a, float r, const Vector& m)
{
	Vector am = m - a;
	Vector v = am.Normalized();

	return a + v * r;
}
