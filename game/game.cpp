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

#include <math/collision.h>
#include <maths.h>

#include <renderer/renderer.h>
#include <renderer/renderingcontext.h>

#include "character.h"

// This is the player character
CCharacter box;

// These are some other boxes
CCharacter target1;
CCharacter target2;
CCharacter target3;

CCharacter prop1;
CCharacter prop2;
CCharacter prop3;
CCharacter prop4;

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
		box.vecMovementGoal.x = box.flSpeed;
		return true;
	}
	else if (c == 'A')
	{
		box.vecMovementGoal.z = box.flSpeed;
		return true;
	}
	else if (c == 'S')
	{
		box.vecMovementGoal.x = -box.flSpeed;
		return true;
	}
	else if (c == 'D')
	{
		box.vecMovementGoal.z = -box.flSpeed;
		return true;
	}
	else if (c == ' ')
	{
		box.vecVelocity.y = 7;
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
		box.vecMovementGoal.x = 0;
	}
	else if (c == 'A')
	{
		box.vecMovementGoal.z = 0;
	}
	else if (c == 'S')
	{
		box.vecMovementGoal.x = 0;
	}
	else if (c == 'D')
	{
		box.vecMovementGoal.z = 0;
	}
	else
		CApplication::KeyPress(c);
}

// This method is called every time the player moves the mouse
void CGame::MouseMotion(int x, int y)
{
	int iMouseMovedX = x - m_iLastMouseX;
	int iMouseMovedY = m_iLastMouseY - y; // The data comes in backwards. negative y means the mouse moved up.

	float flSensitivity = 0.3f;

	box.angView.p += iMouseMovedY*flSensitivity;
	box.angView.y += iMouseMovedX*flSensitivity;

	box.angView.Normalize();

	m_iLastMouseX = x;
	m_iLastMouseY = y;
}

bool CGame::MouseInput(int iButton, tinker_mouse_state_t iState)
{
	if (iButton == TINKER_KEY_MOUSE_LEFT && iState == TINKER_MOUSE_PRESSED)
	{
		Vector v0 = box.mTransform.GetTranslation() + Vector(0, 1, 0);
		Vector v1 = box.mTransform.GetTranslation() + Vector(0, 1, 0) + box.angView.ToVector() * 100;

		Vector vecIntersection;
		CCharacter* pHit = nullptr;
		if (TraceLine(v0, v1, vecIntersection, pHit))
		{
			MakePuff(vecIntersection);
			MakeBulletTracer(v0, vecIntersection);

			if (pHit)
				pHit->flShotTime = Game()->GetTime();
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

	// The v0 and v1 are in the global coordinate system and we need to transform it to the target's
	// local coordinate system to use axis-aligned intersection. We do so using the inverse transform matrix.
	// http://youtu.be/-Fn4atv2NsQ
	if (LineAABBIntersection(target1.aabbSize, target1.mTransformInverse*v0, target1.mTransformInverse*v1, vecTestIntersection, flTestFraction) && flTestFraction < flLowestFraction)
	{
		// Once we have the result we can use the regular transform matrix to get it back in
		// global coordinates. http://youtu.be/-Fn4atv2NsQ
		vecIntersection = target1.mTransform*vecTestIntersection;
		flLowestFraction = flTestFraction;
		pHit = &target1;
	}

	if (LineAABBIntersection(target2.aabbSize, target2.mTransformInverse*v0, target2.mTransformInverse*v1, vecTestIntersection, flTestFraction) && flTestFraction < flLowestFraction)
	{
		vecIntersection = target2.mTransform*vecTestIntersection;
		flLowestFraction = flTestFraction;
		pHit = &target2;
	}

	if (LineAABBIntersection(target3.aabbSize, target3.mTransformInverse*v0, target3.mTransformInverse*v1, vecTestIntersection, flTestFraction) && flTestFraction < flLowestFraction)
	{
		vecIntersection = target3.mTransform*vecTestIntersection;
		flLowestFraction = flTestFraction;
		pHit = &target3;
	}

	if (LineAABBIntersection(prop1.aabbSize, prop1.mTransformInverse*v0, prop1.mTransformInverse*v1, vecTestIntersection, flTestFraction) && flTestFraction < flLowestFraction)
	{
		vecIntersection = prop1.mTransform*vecTestIntersection;
		flLowestFraction = flTestFraction;
		pHit = &prop1;
	}

	if (LineAABBIntersection(prop2.aabbSize, prop2.mTransformInverse*v0, prop2.mTransformInverse*v1, vecTestIntersection, flTestFraction) && flTestFraction < flLowestFraction)
	{
		vecIntersection = prop2.mTransform*vecTestIntersection;
		flLowestFraction = flTestFraction;
		pHit = &prop2;
	}

	if (LineAABBIntersection(prop3.aabbSize, prop3.mTransformInverse*v0, prop3.mTransformInverse*v1, vecTestIntersection, flTestFraction) && flTestFraction < flLowestFraction)
	{
		vecIntersection = prop3.mTransform*vecTestIntersection;
		flLowestFraction = flTestFraction;
		pHit = &prop3;
	}

	if (LineAABBIntersection(prop4.aabbSize, prop4.mTransformInverse*v0, prop4.mTransformInverse*v1, vecTestIntersection, flTestFraction) && flTestFraction < flLowestFraction)
	{
		vecIntersection = prop4.mTransform*vecTestIntersection;
		flLowestFraction = flTestFraction;
		pHit = &prop4;
	}

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
	box.vecMovement.x = Approach(box.vecMovementGoal.x, box.vecMovement.x, dt * 65);
	box.vecMovement.z = Approach(box.vecMovementGoal.z, box.vecMovement.z, dt * 65);

	Vector vecForward = box.angView.ToVector();
	vecForward.y = 0;
	vecForward.Normalize();

	Vector vecUp(0, 1, 0);

	// Cross product http://www.youtube.com/watch?v=FT7MShdqK6w
	Vector vecRight = vecUp.Cross(vecForward);

	float flSaveY = box.vecVelocity.y;
	box.vecVelocity = vecForward * box.vecMovement.x + vecRight * box.vecMovement.z;
	box.vecVelocity.y = flSaveY;

	// Update position and vecMovement. http://www.youtube.com/watch?v=c4b9lCfSDQM
	box.mTransform += box.vecVelocity * dt;
	box.vecVelocity = box.vecVelocity + box.vecGravity * dt;

	// Make sure the player doesn't fall through the floor. The y dimension is up/down, and the floor is at 0.
	if (box.mTransform.GetTranslation().y < 0)
		box.mTransform.v[3].y = 0;

	// Grab the player's translation and make a translation only matrix. http://www.youtube.com/watch?v=iCazI3nKBf0
	Vector vecPosition = box.mTransform.GetTranslation();
	Matrix4x4 mPlayerTranslation;
	mPlayerTranslation.SetTranslation(vecPosition);

	// Create a set of basis vectors that do what we need.
	vecForward = box.angView.ToVector(); // Euler angles: https://www.youtube.com/watch?v=zZM2uUkEoFw
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
	box.mTransform = mPlayerTranslation * mPlayerRotation * mPlayerScaling;
}

void CGame::Draw()
{
	Vector vecForward = box.angView.ToVector();
	Vector vecUp(0, 1, 0);

	// Cross-product http://www.youtube.com/watch?v=FT7MShdqK6w
	Vector vecRight = vecUp.Cross(vecForward).Normalized();

	CRenderer* pRenderer = GetRenderer();

	// Tell the renderer how to set up the camera.
	pRenderer->SetCameraPosition(box.mTransform.GetTranslation() - vecForward * 3 + vecUp * 3 - vecRight * 1.5f);
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

	// First tell OpenGL what "shader" or "program" to use.
	r.UseProgram("model");

	// Set the sunlight direction. The y component is -1 so the light is pointing down.
	r.SetUniform("vecSunlight", Vector(-1, -1, 0.5f).Normalized());

	{
		CRenderingContext c(pRenderer, true);

		// Render the player-box
		c.SetUniform("vecColor", Vector4D(0.8f, 0.4f, 0.2f, 1));

		// The transform matrix holds all transformations for the player. Just pass it through to the renderer.
		// http://youtu.be/7pe1xYzFCvA
		c.Transform(box.mTransform);

		// Render the player-box
		c.RenderBox(-Vector(0.5f, 0, 0.5f), Vector(0.5f, 2, 0.5f));
	}

	{
		CRenderingContext c(pRenderer, true);

		c.SetBackCulling(false);

		// Render the enemies.
		c.SetUniform("vecColor", Vector4D(1, 1, 1, 1));

		Vector vecForward, vecRight, vecUp;
		vecForward = target1.mTransform.GetTranslation() - pRenderer->GetCameraPosition();
		vecRight = -Vector(0, 1, 0).Cross(vecForward).Normalized();
		vecUp = vecForward.Cross(-vecRight).Normalized();

		c.LoadTransform(target1.mTransform);
		c.Translate(Vector(0, target1.aabbSize.GetHeight()/2, 0)); // Move the monster up so his feet don't stick in the ground.
		target1.ShotEffect(&c);
		c.SetUniform("bDiffuse", true);
		c.RenderBillboard(m_iMonsterTexture, target1.aabbSize.vecMax.x, vecUp, vecRight);

		vecForward = target2.mTransform.GetTranslation() - pRenderer->GetCameraPosition();
		vecRight = -Vector(0, 1, 0).Cross(vecForward).Normalized();
		vecUp = vecForward.Cross(-vecRight).Normalized();

		c.LoadTransform(target2.mTransform);
		c.Translate(Vector(0, target2.aabbSize.GetHeight()/2, 0)); // Move the monster up so his feet don't stick in the ground.
		target2.ShotEffect(&c);
		c.RenderBillboard(m_iMonsterTexture, target2.aabbSize.vecMax.x, vecUp, vecRight);

		vecForward = target3.mTransform.GetTranslation() - pRenderer->GetCameraPosition();
		vecRight = -Vector(0, 1, 0).Cross(vecForward).Normalized();
		vecUp = vecForward.Cross(-vecRight).Normalized();

		c.LoadTransform(target3.mTransform);
		c.Translate(Vector(0, target3.aabbSize.GetHeight()/2, 0)); // Move the monster up so his feet don't stick in the ground.
		target3.ShotEffect(&c);
		c.RenderBillboard(m_iMonsterTexture, target3.aabbSize.vecMax.x, vecUp, vecRight);

		c.SetUniform("bDiffuse", false);
	}

	{
		CRenderingContext c(pRenderer, true);

		c.SetBackCulling(false);

		// Render the props.
		c.SetUniform("vecColor", Vector4D(0.4f, 0.8f, 0.2f, 1));

		c.LoadTransform(prop1.mTransform);
		c.RenderBox(prop1.aabbSize.vecMin, prop1.aabbSize.vecMax);

		c.LoadTransform(prop2.mTransform);
		c.RenderBox(prop2.aabbSize.vecMin, prop2.aabbSize.vecMax);

		c.LoadTransform(prop3.mTransform);
		c.RenderBox(prop3.aabbSize.vecMin, prop3.aabbSize.vecMax);

		c.LoadTransform(prop4.mTransform);
		c.RenderBox(prop4.aabbSize.vecMin, prop4.aabbSize.vecMax);
	}

	// Render the ground.
	r.SetUniform("vecColor", Vector4D(0.6f, 0.7f, 0.9f, 1));
	r.BeginRenderTriFan();
		r.Normal(Vector(0, 1, 0));
		r.Vertex(Vector(-30, 0, -30));
		r.Vertex(Vector(-30, 0, 30));
		r.Vertex(Vector(30, 0, 30));
		r.Vertex(Vector(30, 0, -30));
	r.EndRender();

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

// The Game Loop http://www.youtube.com/watch?v=c4b9lCfSDQM
void CGame::GameLoop()
{
	// Initialize the box's position etc
	box.mTransform.SetTranslation(Point(0, 0, 0));
	box.vecMovement = Vector(0, 0, 0);
	box.vecMovementGoal = Vector(0, 0, 0);
	box.vecVelocity = Vector(0, 0, 0);
	box.vecGravity = Vector(0, -10, 0);
	box.flSpeed = 15;

	Vector vecMonsterMin = Vector(-1, 0, -1);
	Vector vecMonsterMax = Vector(1, 2, 1);

	target1.SetTransform(Vector(1, 1, 1),00, Vector(0, 1, 0), Vector(6, 0, 4));
	target1.aabbSize.vecMin = vecMonsterMin;
	target1.aabbSize.vecMax = vecMonsterMax;

	target2.SetTransform(Vector(1, 1, 1), 0, Vector(0, 1, 0), Vector(3, 0, -2));
	target2.aabbSize.vecMin = vecMonsterMin;
	target2.aabbSize.vecMax = vecMonsterMax;

	target3.SetTransform(Vector(3, 3, 3), 0, Vector(0, 1, 0), Vector(-5, 0, 8));
	target3.aabbSize.vecMin = vecMonsterMin;
	target3.aabbSize.vecMax = vecMonsterMax;

	Vector vecPropMin = Vector(-1, 0, -1);
	Vector vecPropMax = Vector(1, 2, 1);

	prop1.SetTransform(Vector(2, 1, 4), 20, Vector(0, 1, 0), Vector(18, 0, 10));
	prop1.aabbSize.vecMin = vecPropMin;
	prop1.aabbSize.vecMax = vecPropMax;

	prop2.SetTransform(Vector(1, 2, 3), 30, Vector(0, 1, 0), Vector(10, 0, 15));
	prop2.aabbSize.vecMin = vecPropMin;
	prop2.aabbSize.vecMax = vecPropMax;

	prop3.SetTransform(Vector(1, 1, 1), -30, Vector(0, 1, 0), Vector(11, 0, 8));
	prop3.aabbSize.vecMin = vecPropMin;
	prop3.aabbSize.vecMax = vecPropMax;

	prop4.SetTransform(Vector(2, 2, 2), 40, Vector(0, 1, 0), Vector(-2, 0, 14));
	prop4.aabbSize.vecMin = vecPropMin;
	prop4.aabbSize.vecMax = vecPropMax;

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
