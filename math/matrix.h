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

#include <vector.h>
#include <vector4d.h>

// The red pill

// This Matrix4x4 class is for use in right-handed coordinate spaces with Y up.
// A column is in sequential memory positions (m[0][0], m[0][1], m[0][2], m[0][3])
// A row is in strided positions (m[0][0], m[1][0], m[2][0], m[3][0])
// Its values are stored with Forward/Up/Right vectors residing in the columns 0, 1, 2
// Its translations are stored in the fourth column
// Transformations are done on column vectors on the right

// The notation of each array location
// [00 10 20 30]
// [01 11 21 31]
// [02 12 22 32]
// [03 13 23 33]

// The layout in that notation of the base vectors (f u r t = forward up right translation)
// [fx ux rx tx]
// [fy uy ry ty]
// [fz uz rz tz]
// [fw uw rw tw]

Vector CrossProduct(const Vector& a, const Vector& b);

class Matrix4x4
{
public:
	            Matrix4x4() { Identity(); }
	explicit    Matrix4x4(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33);
	            Matrix4x4(const Matrix4x4& m);
	explicit    Matrix4x4(float* aflValues);
	explicit    Matrix4x4(const Vector& vecForward, const Vector& vecUp, const Vector& vecRight, const Vector& vecPosition = Vector(0,0,0));

public:
	void		Identity();
	void		Init(const Matrix4x4& m);
	void		Init(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33);

	bool		IsIdentity() const;

	Matrix4x4	Transposed() const;

	// Simple matrix operations
	Matrix4x4	operator*(float f) const;
	Matrix4x4	operator+(const Matrix4x4& m) const;
	Matrix4x4	operator-(const Matrix4x4& m) const;

	// Set a transformation
	void		SetTranslation(const Vector& vecPos);
	void		SetRotation(float flAngle, const Vector& vecAxis);		// Assumes the axis is a normalized vector.
	void		SetScale(const Vector& vecScale);
	void		SetReflection(const Vector& vecPlaneNormal);			// Reflection around a plane with this normal which passes through the center of the local space.
																		// Assumes the plane normal passed in is normalized.

	static Matrix4x4	ProjectPerspective(float flFOV, float flAspectRatio, float flNear, float flFar);							// Just like gluPerspectives
	static Matrix4x4	ProjectFrustum(float flLeft, float flRight, float flBottom, float flTop, float flNear, float flFar);		// Just like glFrustum
	static Matrix4x4	ProjectOrthographic(float flLeft, float flRight, float flBottom, float flTop, float flNear, float flFar);	// Just like glOrtho
	static Matrix4x4	ConstructCameraView(const Vector& vecPosition, const Vector& vecDirection, const Vector& vecUp);			// Like gluLookAt but a direction parameter instead of target

	// Add a translation
	Matrix4x4	operator+=(const Vector& v);
	Matrix4x4   operator-(const Vector& v) const;

	// Add a transformation
	Matrix4x4	operator*(const Matrix4x4& t) const;
	Matrix4x4	operator*=(const Matrix4x4& t);

	bool		operator==(const Matrix4x4& t) const;
	bool		Equals(const Matrix4x4& t, float flEp = 0.000001f) const;

	// Add a transformation
	Matrix4x4	AddTranslation(const Vector& v);
	Matrix4x4	AddScale(const Vector& vecScale);
	Matrix4x4	AddReflection(const Vector& vecPlaneNormal);

	Vector		GetTranslation() const;
	Vector      GetScale() const;

	// Transform a vector
	Vector		operator*(const Vector& v) const;
	Vector		TransformVector(const Vector& v) const;		// Same as homogenous vector with w=0 transform, no translation.
															// You want to use this for directional vectors such as normals and velocities because translations will change their length.
															// It's not immune to scaling though! A matrix with scaling will output a vector of different length than the input.

	Vector4D	operator*(const Vector4D& v) const;

	// Try not to use these in case the underlying format changes.
	Vector4D	GetRow(int i);
	Vector4D	GetColumn(int i) const;
	void		SetColumn(int i, const Vector4D& vecColumn);
	void		SetColumn(int i, const Vector& vecColumn);

	void		SetForwardVector(const Vector& vecForward);
	void		SetUpVector(const Vector& vecUp);
	void		SetRightVector(const Vector& vecRight);
	Vector		GetForwardVector() const { return Vector(m[0][0], m[0][1], m[0][2]); }
	Vector		GetUpVector() const { return Vector(m[1][0], m[1][1], m[1][2]); }
	Vector		GetRightVector() const { return Vector(m[2][0], m[2][1], m[2][2]); }

	void		InvertRT();
	Matrix4x4	InvertedRT() const;

	float		Trace() const;

	operator float*()
	{
		return(&m[0][0]);
	}

	operator const float*() const
	{
		return(&m[0][0]);
	}

	struct MVector4D
	{
		float x, y, z, w;
	};

	union {
		float       m[4][4];
		MVector4D   v[4];
	};
};
