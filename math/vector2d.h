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

#include <vector.h>

class Vector2D
{
public:
				Vector2D();
				Vector2D(float x, float y);
				Vector2D(const Vector& v);

public:
	float	Length() const;
	float	LengthSqr() const;

	const Vector2D	operator+(const Vector2D& v) const;
	const Vector2D	operator-(const Vector2D& v) const;
	const Vector2D	operator*(float s) const;
	const Vector2D	operator/(float s) const;

	void	operator+=(const Vector2D &v);
	void	operator-=(const Vector2D &v);

	float	x, y;
};

inline Vector2D::Vector2D()
	: x(0), y(0)
{
}

inline Vector2D::Vector2D(float X, float Y)
	: x(X), y(Y)
{
}

inline Vector2D::Vector2D(const Vector& v)
	: x(v.x), y(v.y)
{
}

inline float Vector2D::Length() const
{
	return sqrt(x*x + y*y);
}

inline float Vector2D::LengthSqr() const
{
	return x*x + y*y;
}

inline const Vector2D Vector2D::operator+(const Vector2D& v) const
{
	return Vector2D(x+v.x, y+v.y);
}

inline const Vector2D Vector2D::operator-(const Vector2D& v) const
{
	return Vector2D(x-v.x, y-v.y);
}

inline const Vector2D Vector2D::operator*(float s) const
{
	return Vector2D(x*s, y*s);
}

inline const Vector2D Vector2D::operator/(float s) const
{
	return Vector2D(x/s, y/s);
}

inline void Vector2D::operator+=(const Vector2D& v)
{
	x += v.x;
	y += v.y;
}

inline void Vector2D::operator-=(const Vector2D& v)
{
	x -= v.x;
	y -= v.y;
}
