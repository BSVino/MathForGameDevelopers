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

class Vector
{
public:
	Vector()
	{
		x = y = z = 0;
	}
	Vector(float X, float Y, float Z)
	{
		x = X;
		y = Y;
		z = Z;
	}
	Vector(float XYZ[3])
	{
		x = XYZ[0];
		y = XYZ[1];
		z = XYZ[2];
	}
	Vector(const class Point& p);

public:
	float Length() const;
	float LengthSqr() const;

	Vector operator-() const;

	Vector operator+(const Vector& v) const;
	Vector operator-(const Vector& v) const;

	Vector operator*(float s) const;
	Vector operator/(float s) const;

	Vector Normalized() const;
	void Normalize();

	float  Dot(const Vector& v) const;
	Vector Cross(const Vector& v) const;

public:
	union {
		struct {
			float x, y, z;
		};
		float v[3];
	};
};

inline const Vector operator*(float s, const Vector& v)
{
	return Vector(s*v.x, s*v.y, s*v.z);
}

inline const Vector operator/(float s, const Vector& v)
{
	return Vector(s/v.x, s/v.y, s/v.z);
}

class Point
{
public:
	Point() {}
	Point(float X, float Y, float Z)
	{
		x = X;
		y = Y;
		z = Z;
	}

	Point(const Vector& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;
	}

	Point operator+(const Vector& v) const;
	Point operator-(const Vector& v) const;

	float x, y, z;
};
