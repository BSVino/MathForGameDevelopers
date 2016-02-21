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

#include "renderer.h"

#include <vector>
#include <assert.h>

#include <GL3/gl3w.h>
#include <GL/glfw.h>

#if defined(__APPLE__)
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#include <files.h>

#include <renderer/image_read.h>
#include <renderer/shaders.h>

#include "application.h"
#include "renderingcontext.h"

using namespace std;

CRenderer* CRenderer::s_pRenderer = nullptr;

CRenderer::CRenderer(size_t iWidth, size_t iHeight)
{
	if (!HardwareSupported())
		exit(1);

	m_bRenderOrthographic = false;

	m_bUseMultisampleTextures = !!glTexImage2DMultisample;

	GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	GLCall(glGetIntegerv(GL_SAMPLES, &m_iScreenSamples));

	GLCall(glGenVertexArrays(1, &m_default_vao));
	GLCall(glBindVertexArray(m_default_vao));

	GLCall(glGenBuffers(1, &m_dynamic_mesh_vbo));

	SetSize(iWidth, iHeight);

	m_bDrawBackground = true;

	InitBunny();
}

void CRenderer::Initialize()
{
	LoadShaders();
	CShaderLibrary::CompileShaders(m_iScreenSamples);

	WindowResize(m_iWidth, m_iHeight);

	if (!CShaderLibrary::IsCompiled())
		exit(1);
}

void CRenderer::LoadShaders()
{
	vector<string> asShaders = ListDirectory("shaders", false);

	for (size_t i = 0; i < asShaders.size(); i++)
	{
		string sShader = asShaders[i];
		if (!str_endswith(sShader, ".txt"))
			continue;

		CShaderLibrary::AddShader("shaders/" + sShader);
	}
}

void CRenderer::WindowResize(int w, int h)
{
	m_iWidth = w;
	m_iHeight = h;
}

void CRenderer::StartRendering(class CRenderingContext* pContext)
{
	m_iWidth = Application()->GetWindowWidth();
	m_iHeight = Application()->GetWindowHeight();

	float flAspectRatio = (float)m_iWidth/(float)m_iHeight;

	if (ShouldRenderOrthographic())
		pContext->SetProjection(Matrix4x4::ProjectOrthographic(
				-flAspectRatio*m_flCameraOrthoHeight, flAspectRatio*m_flCameraOrthoHeight,
				-m_flCameraOrthoHeight, m_flCameraOrthoHeight,
				-100, 100
			));
	else
		pContext->SetProjection(Matrix4x4::ProjectPerspective(
				m_flCameraFOV,
				flAspectRatio,
				m_flCameraNear,
				m_flCameraFar
			));

	pContext->SetView(Matrix4x4::ConstructCameraView(m_vecCameraPosition, m_vecCameraDirection, m_vecCameraUp));

	for (size_t i = 0; i < 16; i++)
	{
		m_aflModelView[i] = ((float*)pContext->GetView())[i];
		m_aflProjection[i] = ((float*)pContext->GetProjection())[i];
	}

	GLCall(glViewport(0, 0, (GLsizei)m_iWidth, (GLsizei)m_iHeight));

	if (m_iScreenSamples)
		GLCall(glEnable(GL_MULTISAMPLE));
}

void CRenderer::FinishRendering(class CRenderingContext* pContext)
{
	if (m_iScreenSamples)
		GLCall(glDisable(GL_MULTISAMPLE));
}

void CRenderer::RenderOffscreenBuffers(class CRenderingContext* pContext)
{
}

void CRenderer::RenderFullscreenBuffers(class CRenderingContext* pContext)
{
}

void CRenderer::RenderMapFullscreen(size_t iMap, bool bMapIsMultisample)
{
	CRenderingContext c(this, true);

	c.SetWinding(true);
	c.SetDepthTest(false);

	if (!c.GetActiveProgram())
	{
		c.UseProgram("quad");
		c.SetUniform("vecColor", Vector4D(1, 1, 1, 1));
	}

	c.SetUniform("iDiffuse", 0);
	c.SetUniform("bDiffuse", true);

	c.BeginRenderVertexArray();

	c.SetTexCoordBuffer(&m_vecFullscreenTexCoords[0].x);
	c.SetPositionBuffer(&m_vecFullscreenVertices[0].x);

	c.BindTexture(iMap, 0, bMapIsMultisample);

	c.EndRenderVertexArray(6);
}

const double* CRenderer::GetModelView() const
{
	return &m_aflModelView[0];
}

const double* CRenderer::GetProjection() const
{
	return &m_aflProjection[0];
}

const int* CRenderer::GetViewport() const
{
	return &m_aiViewport[0];
}

Vector CRenderer::GetCameraVector()
{
	return m_vecCameraDirection;
}

void CRenderer::SetSize(int w, int h)
{
	m_iWidth = w;
	m_iHeight = h;

	m_vecFullscreenTexCoords[0] = Vector2D(0, 1);
	m_vecFullscreenTexCoords[1] = Vector2D(1, 0);
	m_vecFullscreenTexCoords[2] = Vector2D(0, 0);
	m_vecFullscreenTexCoords[3] = Vector2D(0, 1);
	m_vecFullscreenTexCoords[4] = Vector2D(1, 1);
	m_vecFullscreenTexCoords[5] = Vector2D(1, 0);

	m_vecFullscreenVertices[0] = Vector(-1, -1, 0);
	m_vecFullscreenVertices[1] = Vector(1, 1, 0);
	m_vecFullscreenVertices[2] = Vector(-1, 1, 0);
	m_vecFullscreenVertices[3] = Vector(-1, -1, 0);
	m_vecFullscreenVertices[4] = Vector(1, -1, 0);
	m_vecFullscreenVertices[5] = Vector(1, 1, 0);
}

bool CRenderer::HardwareSupported()
{
	if (!gl3wIsSupported(3, 2))
		return false;

	// Compile a test shader. If it fails we don't support shaders.
	const char* pszVertexShader =
		"#version 150\n"
		"void main()"
		"{"
		"	gl_Position = vec4(0.0, 0.0, 0.0, 0.0);"
		"}";

	const char* pszFragmentShader =
		"#version 150\n"
		"out vec4 vecFragColor;"
		"void main()"
		"{"
		"	vecFragColor = vec4(1.0, 1.0, 1.0, 1.0);"
		"}";

	GLuint iVShader = GLCallReturn(glCreateShader(GL_VERTEX_SHADER));
	GLuint iFShader = GLCallReturn(glCreateShader(GL_FRAGMENT_SHADER));
	GLuint iProgram = GLCallReturn(glCreateProgram());

	GLCall(glShaderSource(iVShader, 1, &pszVertexShader, NULL));
	GLCall(glCompileShader(iVShader));
	GLchar info_log[512];
	GLCall(glGetShaderInfoLog(iVShader, sizeof(info_log), NULL, info_log));

	int iVertexCompiled;
	GLCall(glGetShaderiv(iVShader, GL_COMPILE_STATUS, &iVertexCompiled));

	GLCall(glShaderSource(iFShader, 1, &pszFragmentShader, NULL));
	GLCall(glCompileShader(iFShader));

	int iFragmentCompiled;
	GLCall(glGetShaderiv(iFShader, GL_COMPILE_STATUS, &iFragmentCompiled));

	GLCall(glAttachShader(iProgram, iVShader));
	GLCall(glAttachShader(iProgram, iFShader));
	GLCall(glLinkProgram(iProgram));

	int iProgramLinked;
	GLCall(glGetProgramiv(iProgram, GL_LINK_STATUS, &iProgramLinked));

	GLCall(glDetachShader(iProgram, iVShader));
	GLCall(glDetachShader(iProgram, iFShader));
	GLCall(glDeleteShader(iVShader));
	GLCall(glDeleteShader(iFShader));
	GLCall(glDeleteProgram(iProgram));

	return iVertexCompiled == GL_TRUE && iFragmentCompiled == GL_TRUE && iProgramLinked == GL_TRUE;
}

size_t CRenderer::LoadVertexDataIntoGL(size_t iSizeInBytes, float* aflVertices)
{
	// If it's only floats doubles and the occasional int then it should always be a multiple of four bytes.
	assert(iSizeInBytes%4 == 0);

	GLuint iVBO;
	GLCall(glGenBuffers(1, &iVBO));
	GLCall(glBindBuffer(GL_ARRAY_BUFFER, iVBO));

	GLCall(glBufferData(GL_ARRAY_BUFFER, iSizeInBytes, 0, GL_STATIC_DRAW));

	GLCall(glBufferSubData(GL_ARRAY_BUFFER, 0, iSizeInBytes, aflVertices));

	int iSize = 0;
	GLCall(glGetBufferParameteriv(GL_ARRAY_BUFFER, GL_BUFFER_SIZE, &iSize));
	if(iSizeInBytes != iSize)
	{
		GLCall(glDeleteBuffers(1, &iVBO));
		assert(false);
		// Data size is mismatch with input array
		return 0;
	}

	GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));

	return iVBO;
}

size_t CRenderer::LoadIndexDataIntoGL(size_t iSizeInBytes, unsigned int* aiIndices)
{
	GLuint iVBO;
	GLCall(glGenBuffers(1, &iVBO));
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iVBO));

	GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, iSizeInBytes, 0, GL_STATIC_DRAW));

	GLCall(glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, iSizeInBytes, aiIndices));

	int iSize = 0;
	GLCall(glGetBufferParameteriv(GL_ELEMENT_ARRAY_BUFFER, GL_BUFFER_SIZE, &iSize));
	if (iSizeInBytes != iSize)
	{
		GLCall(glDeleteBuffers(1, &iVBO));
		assert(false);
		// Data size is mismatch with input array
		return 0;
	}

	return iVBO;
}

void CRenderer::UnloadVertexDataFromGL(size_t iBuffer)
{
	GLCall(glDeleteBuffers(1, (GLuint*)&iBuffer));
}

size_t CRenderer::LoadTextureIntoGL(string sFilename, int iClamp)
{
	if (!sFilename.length())
		return 0;

	int x, y, n;
    unsigned char *pData = stbi_load(sFilename.c_str(), &x, &y, &n, 4);

	if (!pData)
		return 0;

	if (x & (x-1))
	{
		assert(false);
		// Image width is not power of 2.
		stbi_image_free(pData);
		return 0;
	}

	if (y & (y-1))
	{
		assert(false);
		// Image height is not power of 2.
		stbi_image_free(pData);
		return 0;
	}

	size_t iGLId = LoadTextureIntoGL((unsigned char*)pData, x, y, iClamp);

	stbi_image_free(pData);

	return iGLId;
}

size_t CRenderer::LoadTextureIntoGL(unsigned char* pclrData, int x, int y, int iClamp, bool bNearestFiltering)
{
	GLuint iGLId;
	GLCall(glGenTextures(1, &iGLId));
	GLCall(glBindTexture(GL_TEXTURE_2D, iGLId));

	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, bNearestFiltering?GL_NEAREST:GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, bNearestFiltering?GL_NEAREST:GL_LINEAR));

	if (iClamp == 1)
	{
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	}

	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, pclrData));
	GLCall(glGenerateMipmap(GL_TEXTURE_2D));

	GLCall(glBindTexture(GL_TEXTURE_2D, 0));

	s_iTexturesLoaded++;

	return iGLId;
}

size_t CRenderer::LoadTextureIntoGL(Vector* pvecData, int x, int y, int iClamp, bool bMipMaps)
{
	GLuint iGLId;
	GLCall(glGenTextures(1, &iGLId));
	GLCall(glBindTexture(GL_TEXTURE_2D, iGLId));

	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));

	if (iClamp == 1)
	{
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE));
		GLCall(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE));
	}

	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, pvecData));

	if (bMipMaps)
		GLCall(glGenerateMipmap(GL_TEXTURE_2D));

	GLCall(glBindTexture(GL_TEXTURE_2D, 0));

	s_iTexturesLoaded++;

	return iGLId;
}

void CRenderer::UnloadTextureFromGL(size_t iGLId)
{
	GLCall(glDeleteTextures(1, (GLuint*)&iGLId));
	s_iTexturesLoaded--;
}

size_t CRenderer::s_iTexturesLoaded = 0;

