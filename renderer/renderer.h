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

#include <string>

#include <vtb.h>

#include <vector.h>
#include <vector2d.h>

#include "render_common.h"

#define CheckGLReturn() \
do { \
	GLenum e = glGetError(); \
	VAssert(e == GL_NO_ERROR); \
	VPRAGMA_WARNING_PUSH \
	VPRAGMA_WARNING_DISABLE(4127) /* conditional expression is constant */ \
} while (0) \
VPRAGMA_WARNING_POP \

#define GLCall(x) \
	do { (x); CheckGLReturn(); } while (0)

#define GLCallReturn(x) \
	(x); do { CheckGLReturn(); } while (0)

typedef enum
{
	FB_DEPTH = (1<<0),
	FB_TEXTURE = (1<<1),
	FB_RENDERBUFFER = (1<<2),
	FB_LINEAR = (1<<3),
	FB_MULTISAMPLE = (1<<4),
	FB_DEPTH_TEXTURE = (1<<5),
	FB_TEXTURE_HALF_FLOAT = (1<<6),
} fb_options_e;

class CRenderer
{
	friend class CRenderingContext;

public:
					CRenderer(size_t iWidth, size_t iHeight);

public:
	virtual void	Initialize();
	void			LoadShaders();

	void InitBunny();
	int  GetBunnyVerts();
	int  GetBunnyNumVerts();
	int  BunnyPositionOffsetBytes();
	int  BunnyNormalOffsetBytes();
	int  BunnyStrideBytes();

	virtual void	WindowResize(int w, int h);

	virtual void	StartRendering(class CRenderingContext* pContext);
	virtual void	FinishRendering(class CRenderingContext* pContext);
	virtual void	RenderOffscreenBuffers(class CRenderingContext* pContext);
	virtual void	RenderFullscreenBuffers(class CRenderingContext* pContext);

	void			RenderMapFullscreen(size_t iMap, bool bMapIsMultisample = false);

	void			SetCameraPosition(Vector vecCameraPosition) { m_vecCameraPosition = vecCameraPosition; };
	void			SetCameraDirection(Vector vecCameraDirection) { m_vecCameraDirection = vecCameraDirection; };
	void			SetCameraUp(Vector vecCameraUp) { m_vecCameraUp = vecCameraUp; };
	void			SetCameraFOV(float flFOV) { m_flCameraFOV = flFOV; };
	void			SetCameraOrthoHeight(float flOrthoHeight) { m_flCameraOrthoHeight = flOrthoHeight; };
	void			SetCameraNear(float flNear) { m_flCameraNear = flNear; };
	void			SetCameraFar(float flFar) { m_flCameraFar = flFar; };
	void			SetRenderOrthographic(bool bRenderOrtho) { m_bRenderOrthographic = bRenderOrtho; }

	const double*   GetModelView() const;
	const double*   GetProjection() const;
	const int*      GetViewport() const;

	Vector			GetCameraPosition() { return m_vecCameraPosition; };
	Vector			GetCameraDirection() { return m_vecCameraDirection; };
	float			GetCameraFOV() { return m_flCameraFOV; };
	float			GetCameraOrthoHeight() { return m_flCameraOrthoHeight; };
	float			GetCameraNear() { return m_flCameraNear; };
	float			GetCameraFar() { return m_flCameraFar; };
	bool			ShouldRenderOrthographic() { return m_bRenderOrthographic; }

	Vector			GetCameraVector();

	void			SetSize(int w, int h);

	bool			HardwareSupported();

	int				ScreenSamples() { return m_iScreenSamples; }

public:
	static size_t	LoadVertexDataIntoGL(size_t iSizeInBytes, float* aflVertices);
	static size_t	LoadIndexDataIntoGL(size_t iSizeInBytes, unsigned int* aiIndices);
	static void		UnloadVertexDataFromGL(size_t iBuffer);
	static size_t	LoadTextureIntoGL(std::string sFilename, int iClamp = 0);
	static void		UnloadTextureFromGL(size_t iGLID);
	static size_t	GetNumTexturesLoaded() { return s_iTexturesLoaded; }

	static size_t   LoadTextureIntoGL(unsigned char* pclrData, int x, int y, int iClamp, bool bNearestFiltering = false);
	static size_t   LoadTextureIntoGL(Vector* pvecData, int x, int y, int iClamp, bool bMipMaps);

public:
	size_t			m_iWidth;
	size_t			m_iHeight;

	Vector			m_vecCameraPosition;
	Vector			m_vecCameraDirection;
	Vector			m_vecCameraUp;
	float			m_flCameraFOV;
	float			m_flCameraOrthoHeight;
	float			m_flCameraNear;
	float			m_flCameraFar;
	bool			m_bRenderOrthographic;

	double			m_aflModelView[16];
	double			m_aflProjection[16];
	int				m_aiViewport[4];

	Vector2D        m_vecFullscreenTexCoords[6];
	Vector          m_vecFullscreenVertices[6];

	uint32_t        m_default_vao;
	uint32_t        m_dynamic_mesh_vbo;

	bool			m_bDrawBackground;

	bool			m_bUseMultisampleTextures;
	int				m_iScreenSamples;

	int m_iBunnyVerts;
	int m_iBunnyNumVerts;

	static CRenderer* s_pRenderer;
	static size_t	s_iTexturesLoaded;
};
