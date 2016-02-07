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

#include "matrix.h"

#include <cstring>

#include "common.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

Matrix4x4::Matrix4x4(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33)
{
	Init(m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33);
}

Matrix4x4::Matrix4x4(const Matrix4x4& i)
{
	m[0][0] = i.m[0][0]; m[0][1] = i.m[0][1]; m[0][2] = i.m[0][2]; m[0][3] = i.m[0][3];
	m[1][0] = i.m[1][0]; m[1][1] = i.m[1][1]; m[1][2] = i.m[1][2]; m[1][3] = i.m[1][3];
	m[2][0] = i.m[2][0]; m[2][1] = i.m[2][1]; m[2][2] = i.m[2][2]; m[2][3] = i.m[2][3];
	m[3][0] = i.m[3][0]; m[3][1] = i.m[3][1]; m[3][2] = i.m[3][2]; m[3][3] = i.m[3][3];
}

Matrix4x4::Matrix4x4(float* aflValues)
{
	memcpy(&m[0][0], aflValues, sizeof(float)*16);
}

Matrix4x4::Matrix4x4(const Vector& vecForward, const Vector& vecUp, const Vector& vecRight, const Vector& vecPosition)
{
	SetForwardVector(vecForward);
	SetUpVector(vecUp);
	SetRightVector(vecRight);
	SetTranslation(vecPosition);

	m[0][3] = 0;
	m[1][3] = 0;
	m[2][3] = 0;
	m[3][3] = 1;
}

Matrix4x4::Matrix4x4(const Vector4D& vecForward, const Vector4D& vecUp, const Vector4D& vecRight, const Vector4D& vecPosition)
{
	SetForwardVector(vecForward);
	SetUpVector(vecUp);
	SetRightVector(vecRight);
	SetTranslation(vecPosition);
}

void Matrix4x4::Identity()
{
	memset(this, 0, sizeof(Matrix4x4));

	m[0][0] = m[1][1] = m[2][2] = m[3][3] = 1.0f;
}

void Matrix4x4::Init(const Matrix4x4& i)
{
	memcpy(&m[0][0], &i.m[0][0], sizeof(float)*16);
}

void Matrix4x4::Init(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13, float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33)
{
	m[0][0] = m00; m[0][1] = m01; m[0][2] = m02; m[0][3] = m03;
	m[1][0] = m10; m[1][1] = m11; m[1][2] = m12; m[1][3] = m13;
	m[2][0] = m20; m[2][1] = m21; m[2][2] = m22; m[2][3] = m23;
	m[3][0] = m30; m[3][1] = m31; m[3][2] = m32; m[3][3] = m33;
}

bool Matrix4x4::IsIdentity() const
{
	if (!(m[0][0] == 1 && m[1][1] == 1 && m[2][2] == 1 && m[3][3] == 1))
		return false;

	if (!(m[0][1] == 0 && m[0][2] == 0 && m[0][3] == 0))
		return false;

	if (!(m[1][0] == 0 && m[1][2] == 0 && m[1][3] == 0))
		return false;

	if (!(m[2][0] == 0 && m[2][1] == 0 && m[2][3] == 0))
		return false;

	if (!(m[3][0] == 0 && m[3][1] == 0 && m[3][2] == 0))
		return false;

	return true;
}

Matrix4x4 Matrix4x4::Transposed() const
{
	Matrix4x4 r;
	r.m[0][0] = m[0][0]; r.m[1][0] = m[0][1]; r.m[2][0] = m[0][2]; r.m[3][0] = m[0][3];
	r.m[0][1] = m[1][0]; r.m[1][1] = m[1][1]; r.m[2][1] = m[1][2]; r.m[3][1] = m[1][3];
	r.m[0][2] = m[2][0]; r.m[1][2] = m[2][1]; r.m[2][2] = m[2][2]; r.m[3][2] = m[2][3];
	r.m[0][3] = m[3][0]; r.m[1][3] = m[3][1]; r.m[2][3] = m[3][2]; r.m[3][3] = m[3][3];
	return r;
}

Matrix4x4 Matrix4x4::operator*(float f) const
{
	Matrix4x4 r;

	r.m[0][0] = m[0][0]*f;
	r.m[0][1] = m[0][1]*f;
	r.m[0][2] = m[0][2]*f;
	r.m[0][3] = m[0][3]*f;

	r.m[1][0] = m[1][0]*f;
	r.m[1][1] = m[1][1]*f;
	r.m[1][2] = m[1][2]*f;
	r.m[1][3] = m[1][3]*f;

	r.m[2][0] = m[2][0]*f;
	r.m[2][1] = m[2][1]*f;
	r.m[2][2] = m[2][2]*f;
	r.m[2][3] = m[2][3]*f;

	r.m[3][0] = m[3][0]*f;
	r.m[3][1] = m[3][1]*f;
	r.m[3][2] = m[3][2]*f;
	r.m[3][3] = m[3][3]*f;

	return r;
}

Matrix4x4 Matrix4x4::operator+(const Matrix4x4& t) const
{
	Matrix4x4 r;

	r.m[0][0] = m[0][0]+t.m[0][0];
	r.m[0][1] = m[0][1]+t.m[0][1];
	r.m[0][2] = m[0][2]+t.m[0][2];
	r.m[0][3] = m[0][3]+t.m[0][3];

	r.m[1][0] = m[1][0]+t.m[1][0];
	r.m[1][1] = m[1][1]+t.m[1][1];
	r.m[1][2] = m[1][2]+t.m[1][2];
	r.m[1][3] = m[1][3]+t.m[1][3];

	r.m[2][0] = m[2][0]+t.m[2][0];
	r.m[2][1] = m[2][1]+t.m[2][1];
	r.m[2][2] = m[2][2]+t.m[2][2];
	r.m[2][3] = m[2][3]+t.m[2][3];

	r.m[3][0] = m[3][0]+t.m[3][0];
	r.m[3][1] = m[3][1]+t.m[3][1];
	r.m[3][2] = m[3][2]+t.m[3][2];
	r.m[3][3] = m[3][3]+t.m[3][3];

	return r;
}

Matrix4x4 Matrix4x4::operator-(const Matrix4x4& t) const
{
	Matrix4x4 r;

	r.m[0][0] = m[0][0]-t.m[0][0];
	r.m[0][1] = m[0][1]-t.m[0][1];
	r.m[0][2] = m[0][2]-t.m[0][2];
	r.m[0][3] = m[0][3]-t.m[0][3];

	r.m[1][0] = m[1][0]-t.m[1][0];
	r.m[1][1] = m[1][1]-t.m[1][1];
	r.m[1][2] = m[1][2]-t.m[1][2];
	r.m[1][3] = m[1][3]-t.m[1][3];

	r.m[2][0] = m[2][0]-t.m[2][0];
	r.m[2][1] = m[2][1]-t.m[2][1];
	r.m[2][2] = m[2][2]-t.m[2][2];
	r.m[2][3] = m[2][3]-t.m[2][3];

	r.m[3][0] = m[3][0]-t.m[3][0];
	r.m[3][1] = m[3][1]-t.m[3][1];
	r.m[3][2] = m[3][2]-t.m[3][2];
	r.m[3][3] = m[3][3]-t.m[3][3];

	return r;
}

void Matrix4x4::SetTranslation(const Vector& vecPos)
{
	m[3][0] = vecPos.x;
	m[3][1] = vecPos.y;
	m[3][2] = vecPos.z;
}

void Matrix4x4::SetTranslation(const Vector4D& vecPos)
{
	m[3][0] = vecPos.x;
	m[3][1] = vecPos.y;
	m[3][2] = vecPos.z;
	m[3][3] = vecPos.w;
}

void Matrix4x4::SetRotation(float flAngle, const Vector& v)
{
	// Normalize beforehand
	TAssert(fabs(v.LengthSqr() - 1) < 0.000001f);

	// c = cos(angle), s = sin(angle), t = (1-c)
	// [ xxt+c   xyt-zs  xzt+ys ]
	// [ yxt+zs  yyt+c   yzt-xs ]
	// [ zxt-ys  zyt+xs  zzt+c  ]

	float x = v.x;
	float y = v.y;
	float z = v.z;

	float c = cos(flAngle*(float)M_PI/180);
	float s = sin(flAngle*(float)M_PI/180);
	float t = 1-c;

	m[0][0] = x*x*t + c;
	m[1][0] = x*y*t - z*s;
	m[2][0] = x*z*t + y*s;

	m[0][1] = y*x*t + z*s;
	m[1][1] = y*y*t + c;
	m[2][1] = y*z*t - x*s;

	m[0][2] = z*x*t - y*s;
	m[1][2] = z*y*t + x*s;
	m[2][2] = z*z*t + c;
}

void Matrix4x4::SetScale(const Vector& vecScale)
{
	m[0][0] = vecScale.x;
	m[1][1] = vecScale.y;
	m[2][2] = vecScale.z;
}

void Matrix4x4::SetReflection(const Vector& vecPlane)
{
	// Normalize beforehand or use ::SetReflection()
	TAssert(fabs(vecPlane.LengthSqr() - 1) < 0.000001f);

	m[0][0] = 1 - 2 * vecPlane.x * vecPlane.x;
	m[1][1] = 1 - 2 * vecPlane.y * vecPlane.y;
	m[2][2] = 1 - 2 * vecPlane.z * vecPlane.z;
	m[1][0] = m[0][1] = -2 * vecPlane.x * vecPlane.y;
	m[2][0] = m[0][2] = -2 * vecPlane.x * vecPlane.z;
	m[1][2] = m[2][1] = -2 * vecPlane.y * vecPlane.z;
}

// Create a perspective projection matrix.
// http://youtu.be/dul0mui292Q
Matrix4x4 Matrix4x4::ProjectPerspective(float flFOV, float flAspectRatio, float flNear, float flFar)
{
	float flTanThetaOver2 = tan(flFOV * (float)M_PI / 360);

	Matrix4x4 m;

	m.Identity();

	// X and Y scaling
	m.m[0][0] = 1/flTanThetaOver2;
	m.m[1][1] = flAspectRatio/flTanThetaOver2;

	// Z coordinate makes z -1 when we're on the near plane and +1 on the far plane
	m.m[2][2] = (flNear + flFar)/(flNear - flFar);
	m.m[3][2] = 2 * flNear * flFar / (flNear - flFar);

	// W = -1 so that we have [x y z -z], a homogenous vector that becomes [-x/z -y/z -1] after division by w.
	m.m[2][3] = -1;

	// Must zero this out, the identity has it as 1.
	m.m[3][3] = 0;

	return m;
}

Matrix4x4 Matrix4x4::ProjectFrustum(float flFOV, float flAspectRatio, float flNear, float flFar)
{
	float flRight = flNear * tan(flFOV * (float)M_PI / 360);
	float flLeft = -flRight;

	float flBottom = flLeft / flAspectRatio;
	float flTop = flRight / flAspectRatio;

	return ProjectFrustum(flLeft, flRight, flBottom, flTop, flNear, flFar);
}

Matrix4x4 Matrix4x4::ProjectFrustum(float flLeft, float flRight, float flBottom, float flTop, float flNear, float flFar)
{
	Matrix4x4 m;
	
	m.Identity();

	float flXD = flRight - flLeft;
	float flYD = flTop - flBottom;
	float flZD = flFar - flNear;

	m.m[0][0] = (2 * flNear) / flXD;
	m.m[1][1] = (2 * flNear) / flYD;

	m.m[2][0] = (flRight + flLeft) / flXD;
	m.m[2][1] = (flTop + flBottom) / flYD;
	m.m[2][2] = -(flFar + flNear) / flZD;
	m.m[2][3] = -1;

	m.m[3][2] = -(2 * flFar * flNear) / flZD;

	m.m[3][3] = 0;

	return m;
}

Matrix4x4 Matrix4x4::ProjectOrthographic(float flLeft, float flRight, float flBottom, float flTop, float flNear, float flFar)
{
	Matrix4x4 m;
	
	m.Identity();

	float flXD = flRight - flLeft;
	float flYD = flTop - flBottom;
	float flZD = flFar - flNear;

	m.m[0][0] = 2.0f / flXD;
	m.m[1][1] = 2.0f / flYD;
	m.m[2][2] = -2.0f / flZD;

	m.m[3][0] = -(flRight + flLeft) / flXD;
	m.m[3][1] = -(flTop + flBottom) / flYD;
	m.m[3][2] = -(flFar + flNear) / flZD;

	return m;
}

// In order to do rendering we have to transform our global coordinates to be
// in the local space of the camera, with the specific requirement that the
// camera looks down the -Z axis. In this function we construct a matrix for
// this transformation. http://youtu.be/3ZmqJb7J5wE
Matrix4x4 Matrix4x4::ConstructCameraView(const Vector& vecPosition, const Vector& vecDirection, const Vector& vecUp)
{
	TAssert(fabs(vecDirection.LengthSqr()-1) < 0.0001f);

	Vector vecCamRight = vecDirection.Cross(vecUp).Normalized();
	Vector vecCamUp = vecCamRight.Cross(vecDirection);

	// OpenGL wants to be looking down the -Z axis. So, pass -vecDirection into the Z position.
	// Then invert the matrix because we're going from global space into local camera space.
	// Easy!
	return Matrix4x4(vecCamRight, vecCamUp, -vecDirection, vecPosition).InvertedTR();
}

Matrix4x4 Matrix4x4::operator+=(const Vector& v)
{
	m[3][0] += v.x;
	m[3][1] += v.y;
	m[3][2] += v.z;

	return *this;
}

Matrix4x4 Matrix4x4::operator-(const Vector& v) const
{
	Matrix4x4 r = *this;

	r.m[3][0] -= v.x;
	r.m[3][1] -= v.y;
	r.m[3][2] -= v.z;

	return r;
}

Matrix4x4 Matrix4x4::operator*(const Matrix4x4& t) const
{
	Matrix4x4 r;

	// [a b c d][A B C D]   [aA+bE+cI+dM
	// [e f g h][E F G H] = [eA+fE+gI+hM ...
	// [i j k l][I J K L]
	// [m n o p][M N O P]

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
			r.m[i][j] = m[0][j]*t.m[i][0] + m[1][j]*t.m[i][1] + m[2][j]*t.m[i][2] + m[3][j]*t.m[i][3];
	}

	return r;
}

Matrix4x4 Matrix4x4::operator*=(const Matrix4x4& t)
{
	*this = (*this)*t;

	return *this;
}

bool Matrix4x4::operator==(const Matrix4x4& t) const
{
	float flEp = 0.000001f;
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (fabs(m[i][j] - t.m[i][j]) > flEp)
				return false;
		}
	}

	return true;
}

bool Matrix4x4::Equals(const Matrix4x4& t, float flEp) const
{
	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			if (fabs(m[i][j] - t.m[i][j]) > flEp)
				return false;
		}
	}

	return true;
}

Matrix4x4 Matrix4x4::AddTranslation(const Vector& v)
{
	Matrix4x4 r;
	r.SetTranslation(v);
	(*this) *= r;

	return *this;
}

Matrix4x4 Matrix4x4::AddScale(const Vector& vecScale)
{
	Matrix4x4 r;
	r.SetScale(vecScale);
	(*this) *= r;

	return *this;
}

Matrix4x4 Matrix4x4::AddReflection(const Vector& v)
{
	Matrix4x4 r;
	r.SetReflection(v);
	(*this) *= r;

	return *this;
}

Vector Matrix4x4::GetTranslation() const
{
	return Vector(m[3][0], m[3][1], m[3][2]);
}

Vector Matrix4x4::GetScale() const
{
	Vector vecReturn;
	vecReturn.x = GetForwardVector().Length();
	vecReturn.y = GetUpVector().Length();
	vecReturn.z = GetRightVector().Length();
	return vecReturn;
}

// Used for vectors which represent a position.
Vector Matrix4x4::operator*(const Vector& v) const
{
	// [a b c x][X] 
	// [d e f y][Y] = [aX+bY+cZ+x dX+eY+fZ+y gX+hY+iZ+z]
	// [g h i z][Z]
	//          [1]

	Vector vecResult;
	vecResult.x = m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z + m[3][0];
	vecResult.y = m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z + m[3][1];
	vecResult.z = m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z + m[3][2];
	return vecResult;
}

// Used for vectors which represent a direction. http://youtu.be/B6d97neDPBk
Vector Matrix4x4::TransformDirection(const Vector& v) const
{
	// [a b c][X] 
	// [d e f][Y] = [aX+bY+cZ dX+eY+fZ gX+hY+iZ]
	// [g h i][Z]

	Vector vecResult;
	vecResult.x = m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z;
	vecResult.y = m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z;
	vecResult.z = m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z;
	return vecResult;
}

Vector4D Matrix4x4::operator*(const Vector4D& v) const
{
	// [a b c x][X] 
	// [d e f y][Y] = [aX+bY+cZ+xW dX+eY+fZ+yW gX+hY+iZ+zW jX+kY+lZ+mW]
	// [g h i z][Z]
	// [j k l m][W]

	Vector4D vecResult;
	vecResult.x = m[0][0] * v.x + m[1][0] * v.y + m[2][0] * v.z + m[3][0] * v.w;
	vecResult.y = m[0][1] * v.x + m[1][1] * v.y + m[2][1] * v.z + m[3][1] * v.w;
	vecResult.z = m[0][2] * v.x + m[1][2] * v.y + m[2][2] * v.z + m[3][2] * v.w;
	vecResult.w = m[0][3] * v.x + m[1][3] * v.y + m[2][3] * v.z + m[3][3] * v.w;
	return vecResult;
}

Vector4D Matrix4x4::GetRow(int i)
{
	return Vector4D(m[i][0], m[i][1], m[i][2], m[i][3]);
}

Vector4D Matrix4x4::GetColumn(int i) const
{
	return Vector4D(m[0][i], m[1][i], m[2][i], m[3][i]);
}

void Matrix4x4::SetColumn(int i, const Vector4D& vecColumn)
{
	m[0][i] = vecColumn.x;
	m[1][i] = vecColumn.y;
	m[2][i] = vecColumn.z;
	m[3][i] = vecColumn.w;
}

void Matrix4x4::SetColumn(int i, const Vector& vecColumn)
{
	m[0][i] = vecColumn.x;
	m[1][i] = vecColumn.y;
	m[2][i] = vecColumn.z;
}

void Matrix4x4::SetForwardVector(const Vector& v)
{
	m[0][0] = v.x;
	m[0][1] = v.y;
	m[0][2] = v.z;
}

void Matrix4x4::SetUpVector(const Vector& v)
{
	m[1][0] = v.x;
	m[1][1] = v.y;
	m[1][2] = v.z;
}

void Matrix4x4::SetRightVector(const Vector& v)
{
	m[2][0] = v.x;
	m[2][1] = v.y;
	m[2][2] = v.z;
}

void Matrix4x4::SetForwardVector(const Vector4D& v)
{
	m[0][0] = v.x;
	m[0][1] = v.y;
	m[0][2] = v.z;
	m[0][3] = v.w;
}

void Matrix4x4::SetUpVector(const Vector4D& v)
{
	m[1][0] = v.x;
	m[1][1] = v.y;
	m[1][2] = v.z;
	m[1][3] = v.w;
}

void Matrix4x4::SetRightVector(const Vector4D& v)
{
	m[2][0] = v.x;
	m[2][1] = v.y;
	m[2][2] = v.z;
	m[2][3] = v.w;
}

// Use the information embedded in a matrix to create its inverse.
// http://youtu.be/7CxKAtWqHC8
Matrix4x4 Matrix4x4::InvertedTR() const
{
	// This method can only be used if the matrix is a translation/rotation matrix.
	// The below asserts will trigger if this is not the case.
	TAssert(fabs(GetForwardVector().LengthSqr() - 1) < 0.00001f);   // Each basis vector should be length 1.
	TAssert(fabs(GetUpVector().LengthSqr() - 1) < 0.00001f);
	TAssert(fabs(GetRightVector().LengthSqr() - 1) < 0.00001f);
	TAssert(fabs(GetForwardVector().Dot(GetUpVector())) < 0.0001f); // All vectors should be orthogonal.
	TAssert(fabs(GetForwardVector().Dot(GetRightVector())) < 0.0001f);
	TAssert(fabs(GetRightVector().Dot(GetUpVector())) < 0.0001f);

	Matrix4x4 M;

	// Create the transposed upper 3x3 matrix
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < 3; j++)
			M.m[i][j] = m[j][i];

	// The new matrix translation = -Rt
	M.SetTranslation(-(M*GetTranslation()));

	return M;
}

float Matrix4x4::Trace() const
{
	return m[0][0] + m[1][1] + m[2][2];
}

// Since we're mucking around with matrices so much, the basis vectors can
// sometimes get slightly longer or shorter than unit length because of
// floating point precision problems. This function re-normalizes the basis
// vectors to make sure we always have a valid matrix where all basis vectors
// are unit length and orthogonal. Only call it if you're using a TR matrix
// and if you're not worried about a performance penalty.
void Matrix4x4::NormalizeTR()
{
	// Keep the forward vector, just make sure it's normal length.
	SetForwardVector(GetForwardVector().Normalized());

	// The up vector is the second most important vector to keep right.
	// Do a cross-product to re-calculate it using the forward and right vectors.
	SetUpVector(GetForwardVector().Cross(-GetRightVector()).Normalized());

	// Finally, re-calculate the right vector.
	SetRightVector(GetForwardVector().Cross(GetUpVector()).Normalized());
}
