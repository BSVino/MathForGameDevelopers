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

#include "game.h"

#include <vector>

#include <vector.h>
#include <matrix.h>
#include <euler.h>
#include <aabb.h>
#include <quaternion.h>

#include "handle.h"

using std::vector;

// This class holds information for a single character - eg the position and velocity of the player
class CCharacter
{
public:
	CCharacter();

public:
	void SetTransform(const Vector& vecScaling, float flTheta, const Vector& vecRotationAxis, const Vector& vecTranslation);
	void SetTranslation(const Vector& vecTranslation);
	void SetRotation(const EAngle& angRotation);
	void SetRotation(const Quaternion& qRotation);

	void ShotEffect(class CRenderingContext* c);

	void TakeDamage(int iDamage);

	void SetMoveParent(CCharacter* pParent);

	const Matrix4x4 GetGlobalTransform() const;
	void            SetGlobalTransform(const Matrix4x4& mGlobal);

	const Vector GetGlobalOrigin() const;
	void         SetGlobalOrigin(const Vector& vecOrigin);

	const EAngle GetLocalView() const;
	void         SetLocalView(const EAngle& angView);
	const Vector GetGlobalView() const;

	const AABB   GetGlobalAABB() const;

private:
	void BuildTransform();

public:
	int       m_iIndex;
	int       m_iParity;

	Vector    m_vecTranslation;
	Vector    m_vecScaling;
	Vector    m_vecRotationAxis;
	float     m_flRotationTheta;

	Vector    m_vecMovement;
	Vector    m_vecMovementGoal;
	Vector    m_vecVelocity;
	Vector    m_vecGravity;
	float     m_flSpeed;
	AABB      m_aabbSize;
	Color     m_clrRender;
	size_t    m_iTexture;
	size_t    m_iBillboardTexture;
	bool      m_bHitByTraces;
	bool      m_bEnemyAI;
	bool      m_bTakesDamage;
	bool      m_bDrawTransparent;
	int       m_iHealth;

	float     m_flShotTime;

	float     m_flDistanceTraveled;

private:
	// If we have a move parent then we only use the local coordinates.
	// Otherwise we'll only use the global coordinates. Use the functions
	// provided to access.
	Matrix4x4 m_mGlobalTransform;
	Matrix4x4 m_mLocalTransform;
	CHandle   m_hMoveParent;
	EAngle    m_angView;
};

