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

#define _USE_MATH_DEFINES

#include <vector>
#include <math.h>

#include <common.h>

#include <maths.h>
#include <euler.h>
#include <aabb.h>
#include <collision.h>

#include "renderer/application.h"
#include "renderer/renderer.h"
#include "renderer/renderingcontext.h"

using std::vector;

// CGame is the "application" class. It creates the window and handles user input.
// It extends CApplication, which does all of the dirty work. All we have to do
// is override functions like KeyPress and KeyRelease, and CApplication will call
// those functions when the player presses a key on the keyboard.
class CGame : public CApplication
{
	DECLARE_CLASS(CGame, CApplication);

	// A puff of "smoke" as if from a bullet hitting something
	class CPuff
	{
	public:
		Point vecOrigin;
		float flTimeCreated;
	};

	// A bullet tracer so we can see where the bullet goes
	class CBulletTracer
	{
	public:
		Point vecStart;
		Point vecEnd;
		float flTimeCreated;
	};

public:
	CGame(int argc, char** argv)
		: CApplication(argc, argv)
	{
		m_iLastMouseX = m_iLastMouseY = 0;
	}

public:
	void Load();

	virtual bool KeyPress(int c);
	virtual void KeyRelease(int c);
	virtual void MouseMotion(int x, int y);
	virtual bool MouseInput(int iButton, tinker_mouse_state_t iState);

	bool TraceLine(const Vector& v0, const Vector& v1, Vector& vecIntersection, class CCharacter*& pHit);

	void MakePuff(const Point& vecPuff);
	const vector<CPuff>& GetPuffs() const { return m_aPuffs; }

	void MakeBulletTracer(const Point& vecStart, const Point& vecEnd);
	const vector<CBulletTracer>& GetTracers() const { return m_aTracers; }

	void Update(float dt);
	void Draw();
	void GameLoop();

private:
	int m_iLastMouseX;
	int m_iLastMouseY;

	vector<CPuff> m_aPuffs;
	vector<CBulletTracer> m_aTracers;

	size_t m_iMonsterTexture;
};

CGame* Game()
{
	return static_cast<CGame*>(Application());
}

// This class holds information for a single character - eg the position and velocity of the player
class CCharacter
{
public:
	CCharacter()
	{
		flShotTime = -1;
	}

public:
	void ShotEffect(CRenderingContext* c)
	{
		// flShotTime gets set to the time when the character was last shot.
		// So, when the character is shot, it will ramp up from 0 to 2pi, or 360 degrees.
		// (We need to use radians because our system sin/cos functions use radians.)
		float flTime = (Game()->GetTime() - flShotTime) * 10;
		if (flShotTime < 0 || flTime > 2*M_PI)
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

public:
	Point  vecPosition;
	Vector vecMovement;
	Vector vecMovementGoal;
	Vector vecVelocity;
	Vector vecGravity;
	EAngle angView;
	float  flSpeed;
	AABB   aabbSize;

	float  flShotTime;
};

// This is the player character
CCharacter box;

// These are some other boxes
CCharacter target1;
CCharacter target2;
CCharacter target3;

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
		Vector v0 = box.vecPosition + Vector(0, 1, 0);
		Vector v1 = box.vecPosition + Vector(0, 1, 0) + box.angView.ToVector() * 100;

		Vector vecIntersection;
		CCharacter* pHit = nullptr;
		if (TraceLine(v0, v1, vecIntersection, pHit))
		{
			MakePuff(vecIntersection);
			MakeBulletTracer(v0, vecIntersection);
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

	if (LineAABBIntersection(target1.aabbSize + target1.vecPosition, v0, v1, vecTestIntersection, flTestFraction) && flTestFraction < flLowestFraction)
	{
		vecIntersection = vecTestIntersection;
		flLowestFraction = flTestFraction;
		pHit = &target1;
	}

	if (LineAABBIntersection(target2.aabbSize + target2.vecPosition, v0, v1, vecTestIntersection, flTestFraction) && flTestFraction < flLowestFraction)
	{
		vecIntersection = vecTestIntersection;
		flLowestFraction = flTestFraction;
		pHit = &target2;
	}

	if (LineAABBIntersection(target3.aabbSize + target3.vecPosition, v0, v1, vecTestIntersection, flTestFraction) && flTestFraction < flLowestFraction)
	{
		vecIntersection = vecTestIntersection;
		flLowestFraction = flTestFraction;
		pHit = &target3;
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
	box.vecPosition = box.vecPosition + box.vecVelocity * dt;
	box.vecVelocity = box.vecVelocity + box.vecGravity * dt;

	// Make sure the player doesn't fall through the floor. The y dimension is up/down, and the floor is at 0.
	if (box.vecPosition.y < 0)
		box.vecPosition.y = 0;
}

void CGame::Draw()
{
	Vector vecForward = box.angView.ToVector();
	Vector vecUp(0, 1, 0);

	// Cross-product http://www.youtube.com/watch?v=FT7MShdqK6w
	Vector vecRight = vecUp.Cross(vecForward).Normalized();

	CRenderer* pRenderer = GetRenderer();

	// Tell the renderer how to set up the camera.
	pRenderer->SetCameraPosition(box.vecPosition - vecForward * 3 + vecUp * 3 - vecRight * 1.5f);
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

		// Sets the position that the player-box will be rendered
		c.Translate(box.vecPosition);

		// Create a set of basis vectors that do what we need.
		Vector vecForward = box.angView.ToVector(); // Euler angles: https://www.youtube.com/watch?v=zZM2uUkEoFw
		vecForward.y = 0;       // Flatten the angles so that the box doesn't rotate up and down as the player does.
		vecForward.Normalize(); // Re-normalize, we need all of our basis vectors to be normal vectors (unit-length)
		Vector vecUp(0, 1, 0);  // The global up vector
		Vector vecRight = -vecUp.Cross(vecForward).Normalized(); // Cross-product: https://www.youtube.com/watch?v=FT7MShdqK6w

		// Use these basis vectors to make a matrix that will transform the player-box the way we want it.
		// http://youtu.be/8sqv11x10lc
		Matrix4x4 mPlayer(vecForward, vecUp, vecRight);
		c.Transform(mPlayer);

		// Render the player-box
		c.RenderBox(-Vector(0.5f, 0, 0.5f), Vector(0.5f, 2, 0.5f));
	}

	{
		CRenderingContext c(pRenderer, true);

		c.SetBackCulling(false);

		// Render the enemies.
		c.SetUniform("vecColor", Vector4D(1, 1, 1, 1));

		Vector vecForward, vecRight, vecUp;
		vecForward = target1.vecPosition - pRenderer->GetCameraPosition();
		vecRight = -Vector(0, 1, 0).Cross(vecForward).Normalized();
		vecUp = vecForward.Cross(-vecRight).Normalized();

		target1.ShotEffect(&c);

		c.SetPosition(target1.vecPosition + Vector(0, target1.aabbSize.GetHeight()/2, 0));
		c.SetUniform("bDiffuse", true);
		c.RenderBillboard(m_iMonsterTexture, target1.aabbSize.vecMax.x, vecUp, vecRight);

		c.ResetTransformations();

		vecForward = target2.vecPosition - pRenderer->GetCameraPosition();
		vecRight = -Vector(0, 1, 0).Cross(vecForward).Normalized();
		vecUp = vecForward.Cross(-vecRight).Normalized();

		target2.ShotEffect(&c);

		c.SetPosition(target2.vecPosition + Vector(0, target1.aabbSize.GetHeight()/2, 0));
		c.RenderBillboard(m_iMonsterTexture, target2.aabbSize.vecMax.x, vecUp, vecRight);

		c.ResetTransformations();

		vecForward = target3.vecPosition - pRenderer->GetCameraPosition();
		vecRight = -Vector(0, 1, 0).Cross(vecForward).Normalized();
		vecUp = vecForward.Cross(-vecRight).Normalized();

		target3.ShotEffect(&c);

		c.SetPosition(target3.vecPosition + Vector(0, target3.aabbSize.GetHeight()/2, 0));
		c.RenderBillboard(m_iMonsterTexture, target3.aabbSize.vecMax.x, vecUp, vecRight);

		c.SetUniform("bDiffuse", false);
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
	box.vecPosition = Point(0, 0, 0);
	box.vecMovement = Vector(0, 0, 0);
	box.vecMovementGoal = Vector(0, 0, 0);
	box.vecVelocity = Vector(0, 0, 0);
	box.vecGravity = Vector(0, -10, 0);
	box.flSpeed = 15;

	Vector vecMonsterMin = Vector(-1, 0, -1);
	Vector vecMonsterMax = Vector(1, 2, 1);

	target1.vecPosition = Point(6, 0, 4);
	target1.aabbSize.vecMin = vecMonsterMin;
	target1.aabbSize.vecMax = vecMonsterMax;

	target2.vecPosition = Point(3, 0, -2);
	target2.aabbSize.vecMin = vecMonsterMin;
	target2.aabbSize.vecMax = vecMonsterMax;

	Vector vecXBasis(1, 0, 0);
	Vector vecYBasis(0, 1, 0);
	Vector vecZBasis(0, 0, 1);

	// Make a matrix that can scale the third target so that he's end boss size.
	// Take each of the coordinate vectors above and scale it by 9. Then the
	// resulting matrix will scale vectors by a factor of three in each dimension!
	// http://youtu.be/0QluD4hJp4U
	Matrix4x4 mBossMatrix(vecXBasis*3, vecYBasis*3, vecZBasis*3);

	target3.vecPosition = Point(-5, 0, 8);
	target3.aabbSize.vecMin = mBossMatrix*vecMonsterMin;
	target3.aabbSize.vecMax = mBossMatrix*vecMonsterMax;

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

int main(int argc, char* argv[])
{
	// Create a game
	CGame game(argc, argv);

	// Open the game's window
	game.OpenWindow(640, 480, false, false);
	game.SetMouseCursorEnabled(false);

	game.Load();

	// Run the game loop!
	game.GameLoop();

	return 0;
}

