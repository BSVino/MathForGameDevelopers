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

#pragma once

#include <cmath>

// https://www.youtube.com/watch?v=qJq7I2DLGzI
float Approach(float flGoal, float flCurrent, float dt)
{
	float flDifference = flGoal - flCurrent;

	if (flDifference > dt)
		return flCurrent + dt;
	if (flDifference < -dt)
		return flCurrent - dt;

	return flGoal;
}

// http://youtu.be/K3TLNQOxU0o
float Remap(float x, float t1, float t2, float s1, float s2)
{
	// "Yellow" is a "normalized" value between 0 and 1
	float yellow = (x - t1)/(t2 - t1);

	// "Green" is the result!
	float green = yellow*(s2 - s1) + s1;

	return green;
}

float RemapClamp(float x, float t1, float t2, float s1, float s2)
{
	if (x < t1)
		return s1;
	if (x > t2)
		return s2;

	return Remap(x, t1, t2, s1, s2);
}

// Returns a triangle wave on the interval [0, 1]
inline float TriangleWave(float flTime, float flLength)
{
	// flTime is a value in the interval [0, infinity].

	// flMod is in the interval [0, flLength]
	float flMod = (float)fmod(flTime, flLength);

	// flRemapped is in the interval [-1, 1]
	float flRemapped = Remap(flMod, 0, flLength, -1, 1);

	// The negative values are flipped to positive, so that you have a triangle wave on [0, 1]: /\/\/\/\/
	return fabs(flRemapped);
}

inline float CubicInterpolation(float t)
{
	return -2*t*t*t + 3*t*t;
}


