#include <vector>

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
	virtual bool KeyPress(int c);
	virtual void KeyRelease(int c);
	virtual void MouseMotion(int x, int y);
	virtual bool MouseInput(int iButton, tinker_mouse_state_t iState);

	bool TraceLine(const Vector& v0, const Vector& v1, Vector& vecIntersection);

	void MakePuff(const Point& vecPuff);
	const vector<CPuff>& GetPuffs() const { return m_aPuffs; }

	void MakeBulletTracer(const Point& vecStart, const Point& vecEnd);
	const vector<CBulletTracer>& GetTracers() const { return m_aTracers; }

private:
	int m_iLastMouseX;
	int m_iLastMouseY;

	vector<CPuff> m_aPuffs;
	vector<CBulletTracer> m_aTracers;
};

CGame* Game()
{
	return static_cast<CGame*>(Application());
}

// This class holds information for a single character - eg the position and velocity of the player
class CCharacter
{
public:
	Point  vecPosition;
	Vector vecMovement;
	Vector vecMovementGoal;
	Vector vecVelocity;
	Vector vecGravity;
	EAngle angView;
	float  flSpeed;
	AABB   aabbSize;
};

// This is the player character
CCharacter box;

// These are some other boxes
CCharacter target1;
CCharacter target2;
CCharacter target3;

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
	int iMouseMovedY = y - m_iLastMouseY;

	float flSensitivity = 0.01f;

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
		if (TraceLine(v0, v1, vecIntersection))
		{
			MakePuff(vecIntersection);
			MakeBulletTracer(v0, vecIntersection);
		}
		else
			MakeBulletTracer(v0, v1);

		return true;
	}

	return false;
}

// Trace a line through the world to simulate, eg, a bullet http://www.youtube.com/watch?v=USjbg5QXk3g
bool CGame::TraceLine(const Vector& v0, const Vector& v1, Vector& vecIntersection)
{
	float flLowestFraction = 1;

	Vector vecTestIntersection;
	float flTestFraction;

	if (LineAABBIntersection(target1.aabbSize + target1.vecPosition, v0, v1, vecTestIntersection, flTestFraction) && flTestFraction < flLowestFraction)
	{
		vecIntersection = vecTestIntersection;
		flLowestFraction = flTestFraction;
	}

	if (LineAABBIntersection(target2.aabbSize + target2.vecPosition, v0, v1, vecTestIntersection, flTestFraction) && flTestFraction < flLowestFraction)
	{
		vecIntersection = vecTestIntersection;
		flLowestFraction = flTestFraction;
	}

	if (LineAABBIntersection(target3.aabbSize + target3.vecPosition, v0, v1, vecTestIntersection, flTestFraction) && flTestFraction < flLowestFraction)
	{
		vecIntersection = vecTestIntersection;
		flLowestFraction = flTestFraction;
	}

	if (flLowestFraction < 1)
		return true;

	return false;
}

// In this Update() function we need to update all of our characters. Move them around or whatever we want to do.
// http://www.youtube.com/watch?v=c4b9lCfSDQM
void Update(float dt)
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

void Draw(CRenderer* pRenderer)
{
	Vector vecForward = box.angView.ToVector();
	Vector vecUp(0, 1, 0);

	// Cross-product http://www.youtube.com/watch?v=FT7MShdqK6w
	Vector vecRight = vecUp.Cross(vecForward).Normalized();

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

	// Render the player-box
	r.SetUniform("vecColor", Vector4D(0.8f, 0.4f, 0.2f, 1));
	r.RenderBox(box.vecPosition - Vector(0.5f, 0, 0.5f), box.vecPosition + Vector(0.5f, 2, 0.5f));

	// Render some other boxes, so that we can tell when we're moving.
	r.SetUniform("vecColor", Vector4D(0.3f, 0.9f, 0.5f, 1));

	Vector vecBoxPosition = Vector(6, 0, 4);
	r.RenderBox(target1.vecPosition + target1.aabbSize.vecMin, target1.vecPosition + target1.aabbSize.vecMax);

	vecBoxPosition = Vector(3, 0, -2);
	r.RenderBox(target2.vecPosition + target2.aabbSize.vecMin, target2.vecPosition + target2.aabbSize.vecMax);

	vecBoxPosition = Vector(-5, 0, 8);
	r.RenderBox(target3.vecPosition + target3.aabbSize.vecMin, target3.vecPosition + target3.aabbSize.vecMax);

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

			int iOrange = Remap(Game()->GetTime(), flTimeCreated, flTimeOver, 0, 255);
			r.SetUniform("vecColor", Color(255, iOrange, 0, 255));
			r.RenderBox(vecOrigin - Vector(1, 1, 1)*flSize, vecOrigin + Vector(1, 1, 1)*flSize);
		}
	}

	pRenderer->FinishRendering(&r);

	// Call this last. Your rendered stuff won't appear on the screen until you call this.
	Application()->SwapBuffers();
}

// The Game Loop http://www.youtube.com/watch?v=c4b9lCfSDQM
void GameLoop(CRenderer* pRenderer)
{
	// Initialize the box's position etc
	box.vecPosition = Point(0, 0, 0);
	box.vecMovement = Vector(0, 0, 0);
	box.vecMovementGoal = Vector(0, 0, 0);
	box.vecVelocity = Vector(0, 0, 0);
	box.vecGravity = Vector(0, -10, 0);
	box.flSpeed = 15;

	target1.vecPosition = Point(6, 0, 4);
	target1.aabbSize.vecMin = Vector(-1, 0, -1);
	target1.aabbSize.vecMax = Vector(1, 2, 1);

	target2.vecPosition = Point(3, 0, -2);
	target2.aabbSize.vecMin = Vector(-1, 0, -1);
	target2.aabbSize.vecMax = Vector(1, 2, 1);

	target3.vecPosition = Point(-5, 0, 8);
	target3.aabbSize.vecMin = Vector(-1, 0, -1);
	target3.aabbSize.vecMax = Vector(1, 2, 1);

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

		Draw(pRenderer);
	}
}

int main(int argc, char* argv[])
{
	// Create a game
	CGame game(argc, argv);

	// Open the game's window
	game.OpenWindow(640, 480, false, false);
	game.SetMouseCursorEnabled(false);

	// Run the game loop!
	GameLoop(game.GetRenderer());

	return 0;
}

