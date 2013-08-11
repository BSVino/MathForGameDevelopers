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

#include "euler.h"

#include "vector.h"
#include "quaternion.h"

#include <cmath>

// Converts a direction vector into an euler angle, assuming up (0, 1, 0)
EAngle::EAngle(const Vector& vecDirection)
{
	p = atan2(vecDirection.y, sqrt(vecDirection.x*vecDirection.x + vecDirection.z*vecDirection.z)) * (float)(180/M_PI);
	y = (float)atan2(vecDirection.z, vecDirection.x) * (float)(180/M_PI);
	r = 0;
}

// Convert euler angles to a vector: https://www.youtube.com/watch?v=zZM2uUkEoFw
Vector EAngle::ToVector() const
{
	Vector result;

	// Convert from degrees to radians.
	float y2 = y/360*2*(float)M_PI;
	float p2 = p/360*2*(float)M_PI;

	result.x = cos(y2)*cos(p2);
	result.y = sin(p2);
	result.z = sin(y2)*cos(p2);

	return result;
}

void EAngle::ToAxisAngle(Vector& vecAxis, float& flAngle) const
{
	// Use Quaternions to do our dirty work.
	Quaternion qRoll(Vector(1, 0, 0), r);
	Quaternion qPitch(Vector(0, 0, 1), p);
	Quaternion qYaw(Vector(0, 1, 0), y);

	Quaternion qEuler = qYaw * qPitch * qRoll;

	qEuler.ToAxisAngle(vecAxis, flAngle);
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
