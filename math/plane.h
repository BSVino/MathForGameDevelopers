/*
Copyright (c) 2013, Lunar Workshop, Inc.

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

#include "vector.h"

// Plane class defined by a normal vector which should aways be unit length and a "distance" to the origin.
// Also can be thought of as ax + by + cz + d = 0, the equation of a plane in 3D space, where a b and c are the x, y, z of the normal n.
class CPlane
{
public:
	void Normalize()
	{
		// It helps a ton if our planes are normalized, meaning n is unit length.
		// To normalize the plane, we do this operation:
		// s(ax + by + cz + d) = s(0)
		// We calculate s by using 1/|n|, and it gets us the number we must scale n by to make it unit length.
		// Notice how d needs to be scaled also.

		float flScale = 1/n.Length();
		n.x *= flScale;
		n.y *= flScale;
		n.z *= flScale;
		d *= flScale;
	}

public:
	Vector n; // The normal
	float d;  // The "distance" to the origin.
};
