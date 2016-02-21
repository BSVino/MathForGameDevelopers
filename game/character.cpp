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

#include "character.h"

#include <mtrand.h>

#include <renderer/renderingcontext.h>

CCharacter::CCharacter()
{
	m_flShotTime = -1;
	m_bHitByTraces = true;
	m_clrRender = Color(255, 255, 255, 255);
	m_iTexture = 0;
	m_iBillboardTexture = 0;
	m_bEnemyAI = false;
	m_flRotationTheta = 0;
	m_vecRotationAxis = Vector(0, 1, 0);
	m_vecScaling = Vector(1, 1, 1);
	m_bTakesDamage = false;
	m_bDrawTransparent = false;
	m_iHealth = 3;
	m_flDistanceTraveled = 0;
}

void CCharacter::SetTransform(const Vector& vecScaling, float flTheta, const Vector& vecRotationAxis, const Vector& vecTranslation)
{
	m_vecScaling = vecScaling;
	m_vecRotationAxis = vecRotationAxis;
	m_flRotationTheta = flTheta;

	SetTranslation(vecTranslation);
}

void CCharacter::SetTranslation(const Vector& vecTranslation)
{
	m_vecTranslation = vecTranslation;

	BuildTransform();
}

void CCharacter::SetRotation(const EAngle& angRotation)
{
	angRotation.ToAxisAngle(m_vecRotationAxis, m_flRotationTheta);

	BuildTransform();
}

void CCharacter::SetRotation(const Quaternion& qRotation)
{
	qRotation.ToAxisAngle(m_vecRotationAxis, m_flRotationTheta);

	BuildTransform();
}

void CCharacter::BuildTransform()
{
	// Produce a transformation matrix from our three TRS matrices.
	// Order matters! http://youtu.be/7pe1xYzFCvA
	Matrix4x4 mScaling, mRotation, mTranslation;
	//mScaling.SetScale(m_vecScaling);
	mRotation.SetRotation(m_flRotationTheta, m_vecRotationAxis);
	mTranslation.SetTranslation(m_vecTranslation);
	SetGlobalTransform(mTranslation * mRotation * mScaling);
}

void CCharacter::ShotEffect(CRenderingContext* c)
{
	// flShotTime gets set to the time when the character was last shot.
	// So, when the character is shot, it will ramp up from 0 to 2pi, or 360 degrees.
	// (We need to use radians because our system sin/cos functions use radians.)
	float flTime = (Game()->GetTime() - m_flShotTime) * 10;
	if (m_flShotTime < 0 || flTime > 2*M_PI)
		return;

	// Create three rotated basis vectors. The X and Z vectors spin around in a circle,
	// but the Y vector remains facing straight up.
	// http://youtu.be/6HaDoXWPICQ
	Vector vecRotateX(cos(flTime), 0, sin(flTime));
	Vector vecRotateY(0, 1, 0);
	Vector vecRotateZ(-sin(flTime), 0, cos(flTime));

	// Load the three basis vectors into a matrix and transform our character with them.
	Matrix4x4 mRotation(vecRotateX, vecRotateY, vecRotateZ);
	c->Transform(mRotation);
}

void CCharacter::TakeDamage(int iDamage)
{
	if (!m_bTakesDamage)
		return;

	m_iHealth -= iDamage;

	if (m_iHealth <= 0)
	{
		// Spawn another baddy to take this guy's place.
		CCharacter* pNew = Game()->CreateCharacter();

		// Position the new monster in a random spot near the player.
		pNew->SetTransform(Vector(1, 1, 1), 0, Vector(0, 1, 0), Vector((float)(mtrand()%20)-10, 0, (float)(mtrand()%20)-10));

		pNew->m_aabbSize.vecMin = Vector(-1, 0, -1);
		pNew->m_aabbSize.vecMax = Vector(1, 2, 1);
		pNew->m_iBillboardTexture = Game()->GetMonsterTexture();
		pNew->m_bEnemyAI = true;
		pNew->m_bTakesDamage = true;

		// We're at zero health, time to die.
		Game()->RemoveCharacter(this);
	}
}

// Make this character move along with the parent character http://youtu.be/KI0KZzqC6sA
void CCharacter::SetMoveParent(CCharacter* pParent)
{
	if (m_hMoveParent.Get() == pParent)
		return;

	if (m_hMoveParent.Get())
	{
		// We're either setting to a different parent or setting to null.
		// If we already have a move parent then we need to calculate the global transform.
		// Good thing we have a handy function for that.
		m_mGlobalTransform = GetGlobalTransform();

		// The view should be transformed into global as well.
		m_angView = GetGlobalView();
	}

	m_hMoveParent = pParent;

	if (!pParent)
		return;

	Matrix4x4 mParentInverse = m_hMoveParent->GetGlobalTransform().InvertedTR();

	// The local transform can be calculated by moving the
	// global coordinates into the move parent local space.
	m_mLocalTransform = mParentInverse * m_mGlobalTransform;

	// The view should be transformed into local as well. We use a different
	// function to make sure we don't get the translation part in our direction
	// vector. http://youtu.be/B6d97neDPBk
	m_angView = mParentInverse.TransformDirection(m_angView.ToVector());
}

const Matrix4x4 CCharacter::GetGlobalTransform() const
{
	if (m_hMoveParent.Get())
		// Calculate the global transform on the fly.
		// In many video games this value is calculated only once every time
		// the move parent is changed, then cached. Here we don't worry about
		// performance so much.
		return m_hMoveParent->GetGlobalTransform() * m_mLocalTransform;

	return m_mGlobalTransform;
}

void CCharacter::SetGlobalTransform(const Matrix4x4& mGlobal)
{
	if (m_hMoveParent.Get())
	{
		// We're not using global coordinates so we have to transfer the global
		// coordinates to local.
		m_mLocalTransform = m_hMoveParent->GetGlobalTransform().InvertedTR() * mGlobal;
		return;
	}

	m_mGlobalTransform = mGlobal;
}

const Vector CCharacter::GetGlobalOrigin() const
{
	return GetGlobalTransform().GetTranslation();
}

void CCharacter::SetGlobalOrigin(const Vector& vecOrigin)
{
	if (m_hMoveParent.Get())
	{
		m_mLocalTransform.SetTranslation(m_hMoveParent->GetGlobalTransform().InvertedTR() * vecOrigin);
		return;
	}

	m_mGlobalTransform.SetTranslation(vecOrigin);
}

const EAngle CCharacter::GetLocalView() const
{
	return m_angView;
}

void CCharacter::SetLocalView(const EAngle& angView)
{
	m_angView = angView;
}

const Vector CCharacter::GetGlobalView() const
{
	if (m_hMoveParent.Get())
		// Calculate the global view on the fly. We use a different transform
		// function because this is a direction vector, not a position vector.
		// http://youtu.be/B6d97neDPBk
		return m_hMoveParent->GetGlobalTransform().TransformDirection(m_angView.ToVector());

	return m_angView.ToVector();
}

const AABB CCharacter::GetGlobalAABB() const
{
	return m_aabbSize * m_vecScaling + GetGlobalOrigin();
}
