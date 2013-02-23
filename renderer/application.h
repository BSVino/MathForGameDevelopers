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

#include <vector>
#include <string>

#include <vector.h>

#include "keys.h"

class CApplication
{
public:
								CApplication(int argc, char** argv);
	virtual 					~CApplication();

public:
	bool						HasCommandLineSwitch(const char* pszSwitch);
	const char*					GetCommandLineSwitchValue(const char* pszSwitch);

	void						SetMultisampling(bool bMultisampling) { m_bMultisampling = bMultisampling; }

	void						OpenWindow(size_t iWidth, size_t iHeight, bool bFullscreen, bool bResizeable);

	virtual std::string         WindowTitle() { return "Math for Game Developers"; }
	virtual std::string         AppDirectory() { return "MFGD"; }

	void						SwapBuffers();
	float                       GetTime();

	bool						IsOpen();
	void						Close();

	bool						HasFocus();

	static void					RenderCallback() { Get()->Render(); };
	virtual void				Render();

	static int					WindowCloseCallback() { return Get()->WindowClose(); };
	virtual int					WindowClose();

	static void					WindowResizeCallback(int x, int y) { Get()->WindowResize(x, y); };
	virtual void				WindowResize(int x, int y);

	static void					MouseMotionCallback(int x, int y) { Get()->MouseMotion(x, y); };
	virtual void				MouseMotion(int x, int y);

	static void					MouseInputCallback(int iButton, int iState);
	void						MouseInputCallback(int iButton, tinker_mouse_state_t iState);
	virtual bool				MouseInput(int iButton, tinker_mouse_state_t iState);

	static void					MouseWheelCallback(int x, int y);
	virtual void				MouseWheel(int x, int y) {};

	static void					KeyEventCallback(int c, int e) { Get()->KeyEvent(c, e); };
	void						KeyEvent(int c, int e);
	virtual bool				KeyPress(int c);
	virtual void				KeyRelease(int c);

	static void					CharEventCallback(int c, int e) { Get()->CharEvent(c, e); };
	void						CharEvent(int c, int e);

	virtual bool				DoKeyPress(int c) { return false; };
	virtual void				DoKeyRelease(int c) {};

	virtual bool				DoCharPress(int c) { return false; };

	bool						IsCtrlDown();
	bool						IsAltDown();
	bool						IsShiftDown();
	bool						IsMouseLeftDown();
	bool						IsMouseRightDown();
	bool						IsMouseMiddleDown();
	void						GetMousePosition(int& x, int& y);

	void						SetMouseCursorEnabled(bool bEnabled);
	bool						IsMouseCursorEnabled();

	int							GetWindowWidth() { return (int)m_iWindowWidth; };
	int							GetWindowHeight() { return (int)m_iWindowHeight; };

	bool						IsFullscreen() { return m_bFullscreen; };

	virtual void				OnClientDisconnect(int iClient) {};

	virtual class CRenderer*	CreateRenderer();
	class CRenderer*			GetRenderer() { return m_pRenderer; }

	static CApplication*		Get() { return s_pApplication; };

protected:
	size_t						m_pWindow;
	size_t						m_iWindowWidth;
	size_t						m_iWindowHeight;
	bool						m_bFullscreen;
	bool						m_bIsOpen;

	bool						m_bMultisampling;

	bool						m_bMouseEnabled;
	double						m_flLastMousePress;

	std::vector<const char*>    m_apszCommandLine;

	class CRenderer*			m_pRenderer;

	class CConsole*				m_pConsole;

	static CApplication*		s_pApplication;
};

inline CApplication* Application()
{
	return CApplication::Get();
}
