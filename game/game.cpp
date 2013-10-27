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

#include <cstring>

#include <math/collision.h>
#include <math/frustum.h>
#include <maths.h>
#include <math/quaternion.h>

#include <renderer/renderer.h>
#include <renderer/renderingcontext.h>

#include "character.h"

CGame::CGame(int argc, char** argv)
	: CApplication(argc, argv)
{
	m_hPlayer = nullptr;

	m_iLastMouseX = m_iLastMouseY = -1;

	memset(m_apEntityList, 0, sizeof(m_apEntityList));
}

void CGame::Load()
{
	m_iMonsterTexture = GetRenderer()->LoadTextureIntoGL("monster.png");
}

void CGame::MakePuff(const Point& p)
{
	m_aPuffs.push_back(CPuff());

	m_aPuffs.back().vecOrigin = p;
	m_aPuffs.back().flTimeCreated = GetTime();
}

void CGame::MakeBulletTracer(const Point& s, const Point& e)
{
	m_aTracers.push_back(CBulletTracer());

	m_aTracers.back().vecStart = s;
	m_aTracers.back().vecEnd = e;
	m_aTracers.back().flTimeCreated = GetTime();
}

// This method gets called when the user presses a key
bool CGame::KeyPress(int c)
{
	if (c == 'W')
	{
		m_hPlayer->m_vecMovementGoal.x = m_hPlayer->m_flSpeed;
		return true;
	}
	else if (c == 'A')
	{
		m_hPlayer->m_vecMovementGoal.z = m_hPlayer->m_flSpeed;
		return true;
	}
	else if (c == 'S')
	{
		m_hPlayer->m_vecMovementGoal.x = -m_hPlayer->m_flSpeed;
		return true;
	}
	else if (c == 'D')
	{
		m_hPlayer->m_vecMovementGoal.z = -m_hPlayer->m_flSpeed;
		return true;
	}
	else if (c == ' ')
	{
		m_hPlayer->m_vecVelocity.y = 7;
		return true;
	}
	else
		return CApplication::KeyPress(c);
}

// This method gets called when the player releases a key.
void CGame::KeyRelease(int c)
{
	if (c == 'W')
	{
		m_hPlayer->m_vecMovementGoal.x = 0;
	}
	else if (c == 'A')
	{
		m_hPlayer->m_vecMovementGoal.z = 0;
	}
	else if (c == 'S')
	{
		m_hPlayer->m_vecMovementGoal.x = 0;
	}
	else if (c == 'D')
	{
		m_hPlayer->m_vecMovementGoal.z = 0;
	}
	else
		CApplication::KeyPress(c);
}

// This method is called every time the player moves the mouse
void CGame::MouseMotion(int x, int y)
{
	if (!HasFocus())
	{
		// Swallow the input while the window isn't in focus so the player
		// isn't facing off in a strange direction when they tab back in.
		m_iLastMouseX = x;
		m_iLastMouseY = y;
		return;
	}

	if (m_iLastMouseX == -1 && m_iLastMouseY == -1)
	{
		m_iLastMouseX = x;
		m_iLastMouseY = y;
	}

	int iMouseMovedX = x - m_iLastMouseX;
	int iMouseMovedY = m_iLastMouseY - y; // The data comes in backwards. negative y means the mouse moved up.

	if (!m_hPlayer)
		return;

	float flSensitivity = 0.3f;

	EAngle angView = m_hPlayer->GetLocalView();

	angView.p += iMouseMovedY*flSensitivity;
	angView.y += iMouseMovedX*flSensitivity;
	angView.Normalize();

	m_hPlayer->SetLocalView(angView);

	m_iLastMouseX = x;
	m_iLastMouseY = y;
}

bool CGame::MouseInput(int iButton, tinker_mouse_state_t iState)
{
	if (iButton == TINKER_KEY_MOUSE_LEFT && iState == TINKER_MOUSE_PRESSED)
	{
		Vector v0 = m_hPlayer->GetGlobalOrigin() + Vector(0, 1, 0);
		Vector v1 = m_hPlayer->GetGlobalOrigin() + Vector(0, 1, 0) + m_hPlayer->GetGlobalView() * 100;

		Vector vecIntersection;
		CCharacter* pHit = nullptr;
		if (TraceLine(v0, v1, vecIntersection, pHit))
		{
			MakePuff(vecIntersection);
			MakeBulletTracer(v0, vecIntersection);

			if (pHit)
			{
				pHit->m_flShotTime = Game()->GetTime();
				pHit->TakeDamage(1);
			}
		}
		else
			MakeBulletTracer(v0, v1);

		return true;
	}

	return false;
}

// Trace a line through the world to simulate, eg, a bullet http://www.youtube.com/watch?v=USjbg5QXk3g
bool CGame::TraceLine(const Vector& v0, const Vector& v1, Vector& vecIntersection, CCharacter*& pHit)
{
	float flLowestFraction = 1;

	Vector vecTestIntersection;
	float flTestFraction;
	pHit = nullptr;

	for (size_t i = 0; i < MAX_CHARACTERS; i++)
	{
		CCharacter* pCharacter = GetCharacterIndex(i);
		if (!pCharacter)
			continue;

		// Only monsters and boxes get hit by traces. The player doesn't, he's immune to his own attacks.
		if (!pCharacter->m_bHitByTraces)
			continue;

		Matrix4x4 mInverse = pCharacter->GetGlobalTransform().InvertedTR();

		// The v0 and v1 are in the global coordinate system and we need to transform it to the target's
		// local coordinate system to use axis-aligned intersection. We do so using the inverse transform matrix.
		// http://youtu.be/-Fn4atv2NsQ
		if (LineAABBIntersection(pCharacter->m_aabbSize, mInverse*v0, mInverse*v1, vecTestIntersection, flTestFraction) && flTestFraction < flLowestFraction)
		{
			// Once we have the result we can use the regular transform matrix to get it back in
			// global coordinates. http://youtu.be/-Fn4atv2NsQ
			vecIntersection = pCharacter->GetGlobalTransform()*vecTestIntersection;
			flLowestFraction = flTestFraction;
			pHit = pCharacter;
		}
	}

	// Intersect with the floor.
	// Line-Plane Intersection algorithm: http://youtu.be/fIu_8b2n8ZM
	if (LinePlaneIntersection(Vector(0, 1, 0), Vector(0, 0, 0), v0, v1, vecTestIntersection, flTestFraction) && flTestFraction < flLowestFraction)
	{
		vecIntersection = vecTestIntersection;
		flLowestFraction = flTestFraction;
		pHit = nullptr;
	}

	if (flLowestFraction < 1)
		return true;

	return false;
}

// In this Update() function we need to update all of our characters. Move them around or whatever we want to do.
// http://www.youtube.com/watch?v=c4b9lCfSDQM
void CGame::Update(float dt)
{
	// The approach function http://www.youtube.com/watch?v=qJq7I2DLGzI
	m_hPlayer->m_vecMovement.x = Approach(m_hPlayer->m_vecMovementGoal.x, m_hPlayer->m_vecMovement.x, dt * 65);
	m_hPlayer->m_vecMovement.z = Approach(m_hPlayer->m_vecMovementGoal.z, m_hPlayer->m_vecMovement.z, dt * 65);

	Vector vecForward = m_hPlayer->GetGlobalView();
	vecForward.y = 0;
	vecForward.Normalize();

	Vector vecUp(0, 1, 0);

	// Cross product http://www.youtube.com/watch?v=FT7MShdqK6w
	Vector vecRight = vecUp.Cross(vecForward);

	float flSaveY = m_hPlayer->m_vecVelocity.y;
	m_hPlayer->m_vecVelocity = vecForward * m_hPlayer->m_vecMovement.x + vecRight * m_hPlayer->m_vecMovement.z;
	m_hPlayer->m_vecVelocity.y = flSaveY;

	// Update position and vecMovement. http://www.youtube.com/watch?v=c4b9lCfSDQM
	m_hPlayer->SetTranslation(m_hPlayer->GetGlobalOrigin() + m_hPlayer->m_vecVelocity * dt);
	m_hPlayer->m_vecVelocity = m_hPlayer->m_vecVelocity + m_hPlayer->m_vecGravity * dt;

	// Make sure the player doesn't fall through the floor. The y dimension is up/down, and the floor is at 0.
	Vector vecTranslation = m_hPlayer->GetGlobalOrigin();
	if (vecTranslation.y < 0)
		m_hPlayer->SetTranslation(Vector(vecTranslation.x, 0, vecTranslation.z));

	// Grab the player's translation and make a translation only matrix. http://www.youtube.com/watch?v=iCazI3nKBf0
	Vector vecPosition = m_hPlayer->GetGlobalOrigin();
	Matrix4x4 mPlayerTranslation;
	mPlayerTranslation.SetTranslation(vecPosition);

	// Create a set of basis vectors that do what we need.
	vecForward = m_hPlayer->GetGlobalView();
	vecForward.y = 0;       // Flatten the angles so that the box doesn't rotate up and down as the player does.
	vecForward.Normalize(); // Re-normalize, we need all of our basis vectors to be normal vectors (unit-length)
	vecUp = Vector(0, 1, 0);  // The global up vector
	vecRight = -vecUp.Cross(vecForward).Normalized(); // Cross-product: https://www.youtube.com/watch?v=FT7MShdqK6w

	// Use these basis vectors to make a matrix that will transform the player-box the way we want it.
	// http://youtu.be/8sqv11x10lc
	Matrix4x4 mPlayerRotation(vecForward, vecUp, vecRight);

	Matrix4x4 mPlayerScaling = Matrix4x4();

	// Produce a transformation matrix from our three TRS matrices.
	// Order matters! http://youtu.be/7pe1xYzFCvA
	m_hPlayer->SetGlobalTransform(mPlayerTranslation * mPlayerRotation * mPlayerScaling);

	float flMonsterSpeed = 0.5f;
	for (size_t i = 0; i < MAX_CHARACTERS; i++)
	{
		CCharacter* pCharacter = GetCharacterIndex(i);
		if (!pCharacter)
			continue;

		if (!pCharacter->m_bEnemyAI)
			continue;

		// Update position and movement. http://www.youtube.com/watch?v=c4b9lCfSDQM
		pCharacter->m_vecVelocity = (m_hPlayer->GetGlobalOrigin() - pCharacter->GetGlobalOrigin()).Normalized() * flMonsterSpeed;

		pCharacter->SetTranslation(pCharacter->GetGlobalOrigin() + pCharacter->m_vecVelocity * dt);
	}
}

void CGame::Draw()
{
	Vector vecForward = m_hPlayer->GetGlobalView();
	Vector vecUp(0, 1, 0);

	// Cross-product http://www.youtube.com/watch?v=FT7MShdqK6w
	Vector vecRight = vecUp.Cross(vecForward).Normalized();

	CRenderer* pRenderer = GetRenderer();

	// Tell the renderer how to set up the camera.
	pRenderer->SetCameraPosition(m_hPlayer->GetGlobalOrigin() - vecForward * 6 + vecUp * 3 - vecRight * 0.5f);
	pRenderer->SetCameraDirection(vecForward);
	pRenderer->SetCameraUp(Vector(0, 1, 0));
	pRenderer->SetCameraFOV(90);
	pRenderer->SetCameraNear(0.1f);
	pRenderer->SetCameraFar(1000);

	// This rendering context is a tool for rendering things to the screen.
	// All of our drawing commands are part of it.
	CRenderingContext r(pRenderer);

	// Clear the depth buffer and set a background color.
	r.ClearDepth();
	r.ClearColor(Color(210, 230, 255));

	// CRenderer::StartRendering() - This function sets up OpenGL with the
	// camera information that we passed it before.
	pRenderer->StartRendering(&r);

	m_oFrameFrustum = CFrustum(r.GetProjection() * r.GetView());

	// First tell OpenGL what "shader" or "program" to use.
	r.UseProgram("model");

	// Set the sunlight direction. The y component is -1 so the light is pointing down.
	r.SetUniform("vecSunlight", Vector(-1, -1, 0.5f).Normalized());

	r.SetUniform("bDiffuse", false);

	// Render the ground.
	r.SetUniform("vecColor", Vector4D(0.6f, 0.7f, 0.9f, 1));
	r.BeginRenderTriFan();
		r.Normal(Vector(0, 1, 0));
		r.Vertex(Vector(-30, 0, -30));
		r.Vertex(Vector(-30, 0, 30));
		r.Vertex(Vector(30, 0, 30));
		r.Vertex(Vector(30, 0, -30));
	r.EndRender();

	// Create an array of vectors to store our triangles.
	Vector avecPoints[6];

	// First triangle.
	avecPoints[0] = Vector(1, 1, 0);
	avecPoints[1] = Vector(0, 1, 0);
	avecPoints[2] = Vector(1, 0, 0);

	// Second triangle.
	avecPoints[3] = Vector(1, 0, 0);
	avecPoints[4] = Vector(0, 1, 0);
	avecPoints[5] = Vector(0, 0, 0);

	r.SetUniform("vecColor", Vector4D(0.9f, 0.4f, 0.2f, 1));

	// Render the triangles.
	r.BeginRenderVertexArray();
	r.SetPositionBuffer(&avecPoints[0].x);
	r.EndRenderVertexArray(6);

	// Prepare a list of entities to render.
	m_apRenderOpaqueList.clear();
	m_apRenderTransparentList.clear();

	for (size_t i = 0; i < MAX_CHARACTERS; i++)
	{
		CCharacter* pCharacter = GetCharacterIndex(i);
		if (!pCharacter)
			continue;

		// We need to scale the AABB using the character's scale values before we can use it to calculate our center/radius.
		AABB aabbSizeWithScaling = pCharacter->m_aabbSize * pCharacter->m_vecScaling;
		Vector vecCharacterCenter = pCharacter->GetGlobalOrigin() + aabbSizeWithScaling.GetCenter();
		float flCharacterRadius = aabbSizeWithScaling.GetRadius();

		// If the entity is outside the viewing frustum then the player can't see it - don't draw it.
		// http://youtu.be/4p-E_31XOPM
		if (!m_oFrameFrustum.SphereIntersection(vecCharacterCenter, flCharacterRadius))
			continue;

		if (pCharacter->m_bDrawTransparent)
			m_apRenderTransparentList.push_back(pCharacter);
		else
			m_apRenderOpaqueList.push_back(pCharacter);
	}

	// Draw all opaque characters first.
	DrawCharacters(m_apRenderOpaqueList, false);

	for (size_t i = 0; i < MAX_CHARACTERS; i++)
	{
		CCharacter* pCharacter = GetCharacterIndex(i);
		if (!pCharacter)
			continue;

		if (!pCharacter->m_bEnemyAI)
			continue;

		float flRadius = 3.5f;

		Vector vecIndicatorOrigin = NearestPointOnSphere(m_hPlayer->GetGlobalOrigin(), flRadius, pCharacter->GetGlobalOrigin());

		float flBoxSize = 0.1f;

		r.SetUniform("vecColor", Color(255, 0, 0, 255));
		r.RenderBox(vecIndicatorOrigin - Vector(1, 1, 1)*flBoxSize, vecIndicatorOrigin + Vector(1, 1, 1)*flBoxSize);
	}

	// Sort the transparent render list so that we paint the items farther from the camera first. http://youtu.be/fEjZrwDKdi8
	MergeSortTransparentRenderList();

	// Now draw all transparent characters, sorted by distance from the camera.
	DrawCharacters(m_apRenderTransparentList, true);

	r.SetUniform("bDiffuse", false);

	// Render any bullet tracers that may have been created.
	float flBulletTracerTime = 0.1f;
	for (size_t i = 0; i < Game()->GetTracers().size(); i++)
	{
		if (Game()->GetTime() < Game()->GetTracers()[i].flTimeCreated + flBulletTracerTime)
		{
			Vector vecStart = Game()->GetTracers()[i].vecStart;
			Vector vecEnd = Game()->GetTracers()[i].vecEnd;

			r.SetUniform("vecColor", Vector4D(1, 0.9f, 0, 1));
			r.BeginRenderLines();
				r.Normal(Vector(0, 1, 0));
				r.Vertex(vecStart);
				r.Vertex(vecEnd);
			r.EndRender();
		}
	}

	// Render any puffs that may have been created.
	float flPuffTime = 0.3f;
	for (size_t i = 0; i < Game()->GetPuffs().size(); i++)
	{
		if (Game()->GetTime() < Game()->GetPuffs()[i].flTimeCreated + flPuffTime)
		{
			float flTimeCreated = Game()->GetPuffs()[i].flTimeCreated;
			float flTimeOver = Game()->GetPuffs()[i].flTimeCreated + flPuffTime;
			float flStartSize = 0.2f;
			float flEndSize = 0.5f;

			float flSize = Remap(Game()->GetTime(), flTimeCreated, flTimeOver, flStartSize, flEndSize);

			Vector vecOrigin = Game()->GetPuffs()[i].vecOrigin;

			int iOrange = (int)Remap(Game()->GetTime(), flTimeCreated, flTimeOver, 0, 255);
			r.SetUniform("vecColor", Color(255, iOrange, 0, 255));
			r.RenderBox(vecOrigin - Vector(1, 1, 1)*flSize, vecOrigin + Vector(1, 1, 1)*flSize);
		}
	}

	pRenderer->FinishRendering(&r);

	// Call this last. Your rendered stuff won't appear on the screen until you call this.
	Application()->SwapBuffers();
}

void CGame::DrawCharacters(const std::vector<CCharacter*>& apRenderList, bool bTransparent)
{
	CRenderer* pRenderer = GetRenderer();

	// Loop through all characters, render them one at a time.
	// Start at the back of the list so that transparent entities use the painter's algorithm.
	for (size_t i = apRenderList.size()-1; i < apRenderList.size(); i--)
	{
		CCharacter* pCharacter = apRenderList[i];

		CRenderingContext c(pRenderer, true);

		c.SetBlend(BLEND_NONE);
		c.SetAlpha(1);

		// Set the color of the box to be rendered.
		c.SetUniform("vecColor", pCharacter->m_clrRender);

		if (pCharacter->m_iBillboardTexture)
		{
			c.SetBackCulling(false);
			c.SetUniform("bDiffuse", true);

			// Create a billboard by creating basis vectors. https://www.youtube.com/watch?v=puOTwCrEm7Q
			Vector vecForward, vecRight, vecUp;
			vecForward = pCharacter->GetGlobalOrigin() - pRenderer->GetCameraPosition();
			vecRight = -Vector(0, 1, 0).Cross(vecForward).Normalized();
			vecUp = vecForward.Cross(-vecRight).Normalized();

			if (pCharacter->m_bDrawTransparent)
			{
				c.SetAlpha(0.6f);
				c.SetBlend(BLEND_ALPHA);
			}

			c.LoadTransform(pCharacter->GetGlobalTransform());
			c.Translate(Vector(0, pCharacter->m_aabbSize.GetHeight()/2, 0)); // Move the character up so his feet don't stick in the ground.
			pCharacter->ShotEffect(&c);
			c.RenderBillboard(pCharacter->m_iBillboardTexture, pCharacter->m_aabbSize.vecMax.x, vecUp, vecRight);
		}
		else
		{
			c.SetUniform("bDiffuse", false);

			// The transform matrix holds all transformations for the player. Just pass it through to the renderer.
			// http://youtu.be/7pe1xYzFCvA
			c.Transform(pCharacter->GetGlobalTransform());

			if (pCharacter->m_bDrawTransparent)
			{
				c.SetAlpha(0.6f);
				c.SetBlend(BLEND_ALPHA);
			}

			// Render the player-box
			c.RenderBox(pCharacter->m_aabbSize.vecMin, pCharacter->m_aabbSize.vecMax);
		}
	}
}

// Sort our render list using the divide and conquer technique knows as Merge Sort.
// http://youtu.be/fEjZrwDKdi8
void MergeSortRenderSubList(std::vector<CCharacter*>& apRenderList, size_t iStart, size_t iEnd)
{
	// iStart is the index of the first index that we are to sort. iEnd is the index+1 of the last index we are to sort.
	size_t iLength = iEnd - iStart;

	if (iLength <= 1)
	{
		// We are in a base case of one item. We're sorted! Return.
		return;
	}
	else if (iLength == 2)
	{
		// We are in a base case of two items. If the first one is bigger than the second, swap them.
		float flLeftDistanceSqr = (apRenderList[iStart]->GetGlobalOrigin() - Game()->GetRenderer()->GetCameraPosition()).LengthSqr();
		float flRightDistanceSqr = (apRenderList[iStart+1]->GetGlobalOrigin() - Game()->GetRenderer()->GetCameraPosition()).LengthSqr();

		// We can compare square distances just like regular distances, and they're faster to calculate. http://www.youtube.com/watch?v=DxmGxkhhluU
		if (flLeftDistanceSqr > flRightDistanceSqr)
			std::swap(apRenderList[iStart], apRenderList[iStart+1]);

		// Now we're sorted!
		return;
	}

	// We aren't in a base case yet. Split the list in two.
	size_t iMiddle = (iStart + iEnd)/2;

	// Sort the two sub-lists by calling this function recursively.
	MergeSortRenderSubList(apRenderList, iStart, iMiddle);
	MergeSortRenderSubList(apRenderList, iMiddle, iEnd);

	// Merge the two sub-lists together by plucking off the lowest element.
	// First make a copy of the list.
	std::vector<CCharacter*> apRenderListCopy = apRenderList;

	size_t iLeft = iStart;
	size_t iRight = iMiddle;
	size_t iOutput = iStart;
	while (true)
	{
		float flLeftDistanceSqr = (apRenderListCopy[iLeft]->GetGlobalOrigin() - Game()->GetRenderer()->GetCameraPosition()).LengthSqr();
		float flRightDistanceSqr = 0;
		if (iRight != iEnd)
			flRightDistanceSqr = (apRenderListCopy[iRight]->GetGlobalOrigin() - Game()->GetRenderer()->GetCameraPosition()).LengthSqr();

		// We can compare square distances just like regular distances, and they're faster to calculate. http://www.youtube.com/watch?v=DxmGxkhhluU
		bool bUseLeft = flLeftDistanceSqr < flRightDistanceSqr;
		if (iRight == iEnd)
			bUseLeft = true;
		else if (iLeft == iMiddle)
			bUseLeft = false;

		if (bUseLeft)
			apRenderList[iOutput++] = apRenderListCopy[iLeft++];
		else
			apRenderList[iOutput++] = apRenderListCopy[iRight++];

		// If we're reached the end of both sub-lists, break from the loop.
		if (iLeft == iMiddle && iRight == iEnd)
			break;
	}

	// Our sub-list is sorted! Return.
}

void CGame::MergeSortTransparentRenderList()
{
	MergeSortRenderSubList(m_apRenderTransparentList, 0, m_apRenderTransparentList.size());
}

// The Game Loop http://www.youtube.com/watch?v=c4b9lCfSDQM
void CGame::GameLoop()
{
	m_hPlayer = CreateCharacter();

	// Initialize the box's position etc
	m_hPlayer->SetGlobalOrigin(Point(0, 0, 0));
	m_hPlayer->m_vecMovement = Vector(0, 0, 0);
	m_hPlayer->m_vecMovementGoal = Vector(0, 0, 0);
	m_hPlayer->m_vecVelocity = Vector(0, 0, 0);
	m_hPlayer->m_vecGravity = Vector(0, -10, 0);
	m_hPlayer->m_flSpeed = 15;
	m_hPlayer->m_clrRender = Color(0.8f, 0.4f, 0.2f, 1.0f);
	m_hPlayer->m_bHitByTraces = false;
	m_hPlayer->m_aabbSize = AABB(-Vector(0.5f, 0, 0.5f), Vector(0.5f, 2, 0.5f));
	m_hPlayer->m_bTakesDamage = true;

	Vector vecMonsterMin = Vector(-1, 0, -1);
	Vector vecMonsterMax = Vector(1, 2, 1);

	/*CCharacter* pTarget1 = CreateCharacter();
	pTarget1->SetTransform(Vector(2, 2, 2), 0, Vector(0, 1, 0), Vector(6, 0, 6));
	pTarget1->m_aabbSize.vecMin = vecMonsterMin;
	pTarget1->m_aabbSize.vecMax = vecMonsterMax;
	pTarget1->m_iBillboardTexture = m_iMonsterTexture;
	pTarget1->m_bEnemyAI = true;
	pTarget1->m_bTakesDamage = true;

	CCharacter* pTarget2 = CreateCharacter();
	pTarget2->SetTransform(Vector(2, 2, 2), 0, Vector(0, 1, 0), Vector(6, 0, -6));
	pTarget2->m_aabbSize.vecMin = vecMonsterMin;
	pTarget2->m_aabbSize.vecMax = vecMonsterMax;
	pTarget2->m_iBillboardTexture = m_iMonsterTexture;
	pTarget2->m_bEnemyAI = true;
	pTarget2->m_bTakesDamage = true;

	CCharacter* pTarget3 = CreateCharacter();
	pTarget3->SetTransform(Vector(3, 3, 3), 0, Vector(0, 1, 0), Vector(-6, 0, 8));
	pTarget3->m_aabbSize.vecMin = vecMonsterMin;
	pTarget3->m_aabbSize.vecMax = vecMonsterMax;
	pTarget3->m_iBillboardTexture = m_iMonsterTexture;
	pTarget3->m_bEnemyAI = true;
	pTarget3->m_bTakesDamage = true;*/

	Vector vecPropMin = Vector(-1, 0, -1);
	Vector vecPropMax = Vector(1, 2, 1);

	CCharacter* pProp1 = CreateCharacter();
	pProp1->SetTransform(Vector(1, 1, 1), 20, Vector(0, 1, 0), Vector(18, 0, 10));
	pProp1->m_aabbSize.vecMin = vecPropMin;
	pProp1->m_aabbSize.vecMax = vecPropMax;
	pProp1->m_clrRender = Color(0.4f, 0.8f, 0.2f, 1.0f);

	CCharacter* pProp2 = CreateCharacter();
	pProp2->SetTransform(Vector(1, 1, 1), 30, Vector(0, 1, 0), Vector(10, 0, 15));
	pProp2->m_aabbSize.vecMin = vecPropMin;
	pProp2->m_aabbSize.vecMax = vecPropMax;
	pProp2->m_clrRender = Color(0.4f, 0.8f, 0.2f, 1.0f);

	CCharacter* pProp3 = CreateCharacter();
	pProp3->SetTransform(Vector(1, 1, 1), -30, Vector(0, 1, 0), Vector(11, 0, 8));
	pProp3->m_aabbSize.vecMin = vecPropMin;
	pProp3->m_aabbSize.vecMax = vecPropMax;
	pProp3->m_clrRender = Color(0.4f, 0.8f, 0.2f, 1.0f);

	CCharacter* pProp4 = CreateCharacter();
	pProp4->SetTransform(Vector(1, 1, 1), 40, Vector(0, 1, 0), Vector(-2, 0, 14));
	pProp4->m_aabbSize.vecMin = vecPropMin;
	pProp4->m_aabbSize.vecMax = vecPropMax;
	pProp4->m_clrRender = Color(0.4f, 0.8f, 0.2f, 1.0f);

	float flPreviousTime = 0;
	float flCurrentTime = Application()->GetTime();

	while (true)
	{
		// flCurrentTime will be lying around from last frame. It's now the previous time.
		flPreviousTime = flCurrentTime;
		flCurrentTime = Application()->GetTime();

		float dt = flCurrentTime - flPreviousTime;

		// Keep dt from growing too large.
		if (dt > 0.15f)
			dt = 0.15f;

		Update(dt);

		Draw();
	}
}

// Create a character and add him into our entity list.
// Entity list explained here: http://youtu.be/V6vq0PRFKgk
CCharacter* CGame::CreateCharacter()
{
	size_t iSpot = ~0;

	// Find a spot in my entity list that's empty.
	for (size_t i = 0; i < MAX_CHARACTERS; i++)
	{
		if (!m_apEntityList[i])
		{
			iSpot = i;
			break;
		}
	}

	if (iSpot == ~0)
		// Couldn't find a spot for the new guy! Return null instead.
		return nullptr;

	m_apEntityList[iSpot] = new CCharacter();

	static int iParity = 0;
	m_apEntityList[iSpot]->m_iParity = iParity++;
	m_apEntityList[iSpot]->m_iIndex = iSpot;

	return m_apEntityList[iSpot];
}

// Remove a character from the entity list.
void CGame::RemoveCharacter(CCharacter* pCharacter)
{
	size_t iSpot = ~0;

	// Find a spot in my entity list that's empty.
	for (size_t i = 0; i < MAX_CHARACTERS; i++)
	{
		if (m_apEntityList[i] == pCharacter)
		{
			iSpot = i;
			break;
		}
	}

	if (iSpot == ~0)
		// Couldn't find this guy in our entity list! Do nothing.
		return;

	delete m_apEntityList[iSpot];
	m_apEntityList[iSpot] = nullptr;
}

CCharacter* CGame::GetCharacterIndex(size_t i)
{
	return m_apEntityList[i];
}
