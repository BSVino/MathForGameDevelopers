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
#include <color.h>

class Vector4D
{
public:
				Vector4D();
				Vector4D(const Vector& v);
				Vector4D(const Vector& v, float w);
				Vector4D(const Color& c);
				Vector4D(float x, float y, float z, float w);
				Vector4D(const float* xyzw);

public:
	const Vector4D	operator+(const Vector4D& v) const;
	const Vector4D	operator-(const Vector4D& v) const;

	bool	operator==(const Vector4D& v) const
	{
		float flEp = 0.000001f;
		return std::fabs(v.x - x) < flEp && std::fabs(v.y - y) < flEp && std::fabs(v.z - z) < flEp && std::fabs(v.w - w) < flEp;
	}

	operator float*()
	{
		return(&x);
	}

	operator const float*() const
	{
		return(&x);
	}

	float	x, y, z, w;
};

inline Vector4D::Vector4D()
	: x(0), y(0), z(0), w(0)
{
}

inline Vector4D::Vector4D(const Vector& v)
	: x(v.x), y(v.y), z(v.z), w(0)
{
}

inline Vector4D::Vector4D(const Vector& v, float W)
	: x(v.x), y(v.y), z(v.z), w(W)
{
}

inline Vector4D::Vector4D(const Color& c)
	: x(((float)c.r())/255), y(((float)c.g())/255), z(((float)c.b())/255), w(((float)c.a())/255)
{
}

inline Vector4D::Vector4D(float X, float Y, float Z, float W)
	: x(X), y(Y), z(Z), w(W)
{
}

inline Vector4D::Vector4D(const float* xyzw)
	: x(*xyzw), y(*(xyzw+1)), z(*(xyzw+2)), w(*(xyzw+3))
{
}

inline const Vector4D Vector4D::operator+(const Vector4D& v) const
{
	return Vector4D(x+v.x, y+v.y, z+v.z, w+v.w);
}

inline const Vector4D Vector4D::operator-(const Vector4D& v) const
{
	return Vector4D(x-v.x, y-v.y, z-v.z, w-v.w);
}
