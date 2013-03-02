#include <common.h>

#include <maths.h>
#include <euler.h>

#include "renderer/application.h"
#include "renderer/renderer.h"
#include "renderer/renderingcontext.h"

// CGame is the "application" class. It creates the window and handles user input.
// It extends CApplication, which does all of the dirty work. All we have to do
// is override functions like KeyPress and KeyRelease, and CApplication will call
// those functions when the player presses a key on the keyboard.
class CGame : public CApplication
{
	DECLARE_CLASS(CGame, CApplication);

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

private:
	int m_iLastMouseX;
	int m_iLastMouseY;
};

// This class holds information for a single character - eg the position and velocity of the player
class CCharacter
{
public:
	Point vecPosition;
	Vector vecVelocity;
	Vector vecVelocityGoal;
	Vector vecGravity;
	EAngle angView;
};

// We'll create a single character named "box"
CCharacter box;

// This method gets called when the user presses a key
bool CGame::KeyPress(int c)
{
	if (c == 'W')
	{
		box.vecVelocityGoal.z = 15;
		return true;
	}
	else if (c == 'A')
	{
		box.vecVelocityGoal.x = 15;
		return true;
	}
	else if (c == 'S')
	{
		box.vecVelocityGoal.z = -15;
		return true;
	}
	else if (c == 'D')
	{
		box.vecVelocityGoal.x = -15;
		return true;
	}
	else if (c == ' ')
	{
		box.vecVelocity.y = 2;
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
		box.vecVelocityGoal.z = 0;
	}
	else if (c == 'A')
	{
		box.vecVelocityGoal.x = 0;
	}
	else if (c == 'S')
	{
		box.vecVelocityGoal.z = 0;
	}
	else if (c == 'D')
	{
		box.vecVelocityGoal.x = 0;
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

// In this Update() function we need to update all of our characters. Move them around or whatever we want to do.
void Update(float dt)
{
	box.vecVelocity.x = Approach(box.vecVelocityGoal.x, box.vecVelocity.x, dt * 65);
	box.vecVelocity.z = Approach(box.vecVelocityGoal.z, box.vecVelocity.z, dt * 65);

	// Update position and vecVelocity.
	box.vecPosition = box.vecPosition + box.vecVelocity * dt;
	box.vecVelocity = box.vecVelocity + box.vecGravity * dt;

	// Make sure the player doesn't fall through the floor. The y dimension is up/down, and the floor is at 0.
	if (box.vecPosition.y < 0)
		box.vecPosition.y = 0;
}

void Draw(CRenderer* pRenderer)
{
	// Tell the renderer how to set up the camera.
	pRenderer->SetCameraPosition(box.vecPosition - box.angView.ToVector() * 5);
	pRenderer->SetCameraDirection(Vector(box.vecPosition - pRenderer->GetCameraPosition()).Normalized()); // Look at the box
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
	r.RenderBox(vecBoxPosition - Vector(0.5f, 0, 0.5f), vecBoxPosition + Vector(0.5f, 1, 0.5f));

	vecBoxPosition = Vector(3, 0, -2);
	r.RenderBox(vecBoxPosition - Vector(0.5f, 0, 0.5f), vecBoxPosition + Vector(0.5f, 1, 0.5f));

	vecBoxPosition = Vector(-5, 0, 8);
	r.RenderBox(vecBoxPosition - Vector(0.5f, 0, 0.5f), vecBoxPosition + Vector(0.5f, 1, 0.5f));

	// Render the ground.
	r.SetUniform("vecColor", Vector4D(0.6f, 0.7f, 0.9f, 1));
	r.BeginRenderTriFan();
		r.Normal(Vector(0, 1, 0));
		r.Vertex(Vector(-30, 0, -30));
		r.Vertex(Vector(-30, 0, 30));
		r.Vertex(Vector(30, 0, 30));
		r.Vertex(Vector(30, 0, -30));
	r.EndRender();

	pRenderer->FinishRendering(&r);

	// Call this last. Your rendered stuff won't appear on the screen until you call this.
	Application()->SwapBuffers();
}

void GameLoop(CRenderer* pRenderer)
{
	// Initialize the box's position etc
	box.vecPosition = Point(0, 0, 0);
	box.vecVelocity = Vector(0, 0, 0);
	box.vecGravity = Vector(0, -4, 0);

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

