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

#include "renderingcontext.h"

#include <string.h>
#include <GL3/gl3w.h>

#if defined(__APPLE__)
#include <OpenGL/glu.h>
#else
#include <GL/glu.h>
#endif

#include <common.h>

#include <renderer/shaders.h>
#include <renderer/application.h>
#include <renderer/renderer.h>

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

using namespace std;

vector<float> CRenderingContext::s_dynamic_verts;

vector<CRenderingContext::CRenderContext> CRenderingContext::s_aContexts;

CRenderingContext::CRenderingContext(CRenderer* pRenderer, bool bInherit)
{
	m_pRenderer = pRenderer;

	m_clrRender = ::Color(255, 255, 255, 255);

	s_aContexts.push_back(CRenderContext());

	if (bInherit && s_aContexts.size() > 1)
	{
		CRenderContext& oLastContext = s_aContexts[s_aContexts.size()-2];
		CRenderContext& oThisContext = GetContext();

		oThisContext.m_mProjection = oLastContext.m_mProjection;
		oThisContext.m_mView = oLastContext.m_mView;
		oThisContext.m_mTransformations = oLastContext.m_mTransformations;

		strncpy(oThisContext.m_szProgram, oLastContext.m_szProgram, PROGRAM_LEN);
		oThisContext.m_pShader = oLastContext.m_pShader;

		oThisContext.m_iViewportX = oLastContext.m_iViewportX;
		oThisContext.m_iViewportY = oLastContext.m_iViewportY;
		oThisContext.m_iViewportWidth = oLastContext.m_iViewportWidth;
		oThisContext.m_iViewportHeight = oLastContext.m_iViewportHeight;
		oThisContext.m_eBlend = oLastContext.m_eBlend;
		oThisContext.m_flAlpha = oLastContext.m_flAlpha;
		oThisContext.m_bDepthMask = oLastContext.m_bDepthMask;
		oThisContext.m_bDepthTest = oLastContext.m_bDepthTest;
		oThisContext.m_eDepthFunction = oLastContext.m_eDepthFunction;
		oThisContext.m_bCull = oLastContext.m_bCull;
		oThisContext.m_bWinding = oLastContext.m_bWinding;

		m_pShader = oThisContext.m_pShader;

		if (m_pShader)
			m_iProgram = m_pShader->m_iProgram;
		else
			m_iProgram = 0;
	}
	else
	{
		m_pShader = NULL;

		BindTexture(0);
		UseProgram("");

		SetViewport(0, 0, Application()->GetWindowWidth(), Application()->GetWindowHeight());
		SetBlend(BLEND_NONE);
		SetAlpha(1);
		SetDepthMask(true);
		SetDepthTest(true);
		SetDepthFunction(DF_LESS);
		SetBackCulling(true);
		SetWinding(true);
	}
}

CRenderingContext::~CRenderingContext()
{
	s_aContexts.pop_back();

	if (s_aContexts.size())
	{
		CRenderContext& oContext = GetContext();

		UseProgram(oContext.m_pShader);

		if (*oContext.m_szProgram)
		{
			oContext.m_bProjectionUpdated = false;
			oContext.m_bViewUpdated = false;
			oContext.m_bTransformUpdated = false;
		}

		SetViewport(oContext.m_iViewportX, oContext.m_iViewportY, oContext.m_iViewportWidth, oContext.m_iViewportHeight);
		SetBlend(oContext.m_eBlend);
		SetAlpha(oContext.m_flAlpha);
		SetDepthMask(oContext.m_bDepthMask);
		SetDepthTest(oContext.m_bDepthTest);
		SetDepthFunction(oContext.m_eDepthFunction);
		SetBackCulling(oContext.m_bCull);
		SetWinding(oContext.m_bWinding);
	}
	else
	{
		GLCall(glActiveTexture(GL_TEXTURE0));
		GLCall(glBindTexture(GL_TEXTURE_2D, 0));

		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		if (m_pRenderer)
			GLCall(glViewport(0, 0, (GLsizei)m_pRenderer->m_iWidth, (GLsizei)m_pRenderer->m_iHeight));
		else
			GLCall(glViewport(0, 0, (GLsizei)Application()->GetWindowWidth(), (GLsizei)Application()->GetWindowHeight()));

		GLCall(glUseProgram(0));

		GLCall(glDisablei(GL_BLEND, 0));

		GLCall(glDepthMask(true));
		GLCall(glEnable(GL_DEPTH_TEST));
		GLCall(glEnable(GL_CULL_FACE));
		GLCall(glDepthFunc(GL_LESS));

		GLCall(glFrontFace(GL_CCW));
	}
}

void CRenderingContext::SetProjection(const Matrix4x4& m)
{
	CRenderContext& oContext = GetContext();

	oContext.m_mProjection = m;
	GetContext().m_bProjectionUpdated = false;
}

void CRenderingContext::SetView(const Matrix4x4& m)
{
	CRenderContext& oContext = GetContext();

	oContext.m_mView = m;
	oContext.m_bViewUpdated = false;
}

void CRenderingContext::SetPosition(const Vector& vecPosition)
{
	CRenderContext& oContext = GetContext();

	oContext.m_mTransformations.SetTranslation(vecPosition);
	oContext.m_bTransformUpdated = false;
}

void CRenderingContext::Transform(const Matrix4x4& m)
{
	CRenderContext& oContext = GetContext();

	oContext.m_mTransformations *= m;
	oContext.m_bTransformUpdated = false;
}

void CRenderingContext::Translate(const Vector& vecTranslate)
{
	CRenderContext& oContext = GetContext();

	oContext.m_mTransformations.AddTranslation(vecTranslate);
	oContext.m_bTransformUpdated = false;
}

void CRenderingContext::Rotate(float flAngle, Vector vecAxis)
{
	Matrix4x4 mRotation;
	mRotation.SetRotation(flAngle, vecAxis);

	CRenderContext& oContext = GetContext();

	oContext.m_mTransformations *= mRotation;
	oContext.m_bTransformUpdated = false;
}

void CRenderingContext::Scale(float flX, float flY, float flZ)
{
	CRenderContext& oContext = GetContext();

	oContext.m_mTransformations.AddScale(Vector(flX, flY, flZ));
	oContext.m_bTransformUpdated = false;
}

void CRenderingContext::ResetTransformations()
{
	CRenderContext& oContext = GetContext();

	oContext.m_mTransformations.Identity();
	oContext.m_bTransformUpdated = false;
}

void CRenderingContext::LoadTransform(const Matrix4x4& m)
{
	CRenderContext& oContext = GetContext();

	oContext.m_mTransformations = m;
	oContext.m_bTransformUpdated = false;
}

void CRenderingContext::SetViewport(int x, int y, int w, int h)
{
	CRenderContext& oContext = GetContext();

	oContext.m_iViewportX = x;
	oContext.m_iViewportY = y;
	oContext.m_iViewportWidth = w;
	oContext.m_iViewportHeight = h;
}

void CRenderingContext::SetBlend(blendtype_t eBlend)
{
	if (eBlend)
	{
		GLCall(glEnablei(GL_BLEND, 0));

		if (eBlend == BLEND_ALPHA)
			GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		else if (eBlend == BLEND_ADDITIVE)
			GLCall(glBlendFunc(GL_SRC_ALPHA, GL_ONE));
		else if (eBlend == BLEND_BOTH)
			GLCall(glBlendFunc(GL_ONE, GL_ONE));
	}
	else
		GLCall(glDisablei(GL_BLEND, 0));

	GetContext().m_eBlend = eBlend;
}

void CRenderingContext::SetDepthMask(bool bDepthMask)
{
	GLCall(glDepthMask(bDepthMask));
	GetContext().m_bDepthMask = bDepthMask;
}

void CRenderingContext::SetDepthTest(bool bDepthTest)
{
	if (bDepthTest)
		GLCall(glEnable(GL_DEPTH_TEST));
	else
		GLCall(glDisable(GL_DEPTH_TEST));
	GetContext().m_bDepthTest = bDepthTest;
}

void CRenderingContext::SetDepthFunction(depth_function_t eDepthFunction)
{
	if (eDepthFunction == DF_LEQUAL)
		GLCall(glDepthFunc(GL_LEQUAL));
	else if (eDepthFunction == DF_LESS)
		GLCall(glDepthFunc(GL_LESS));

	GetContext().m_eDepthFunction = eDepthFunction;
}

void CRenderingContext::SetBackCulling(bool bCull)
{
	if (bCull)
		GLCall(glEnable(GL_CULL_FACE));
	else
		GLCall(glDisable(GL_CULL_FACE));
	GetContext().m_bCull = bCull;
}

void CRenderingContext::SetWinding(bool bWinding)
{
	GetContext().m_bWinding = bWinding;
	GLCall(glFrontFace(bWinding?GL_CCW:GL_CW));
}

void CRenderingContext::ClearColor(const ::Color& clrClear)
{
	GLCall(glClearColor((float)(clrClear.r())/255, (float)(clrClear.g())/255, (float)(clrClear.b())/255, (float)(clrClear.a())/255));
	GLCall(glClear(GL_COLOR_BUFFER_BIT));
}

void CRenderingContext::ClearDepth()
{
	GLCall(glClear(GL_DEPTH_BUFFER_BIT));
}

void CRenderingContext::RenderWireBox(const Vector& vecMins, const Vector& vecMaxs)
{
	BeginRenderLineLoop();
		Vertex(vecMaxs);
		Vertex(Vector(vecMins.x, vecMaxs.y, vecMaxs.z));
		Vertex(Vector(vecMins.x, vecMaxs.y, vecMins.z));
		Vertex(Vector(vecMaxs.x, vecMaxs.y, vecMins.z));
		Vertex(vecMaxs);

		Vertex(Vector(vecMaxs.x, vecMins.y, vecMaxs.z));
		Vertex(Vector(vecMins.x, vecMins.y, vecMaxs.z));
		Vertex(Vector(vecMins.x, vecMins.y, vecMins.z));
		Vertex(Vector(vecMaxs.x, vecMins.y, vecMins.z));
		Vertex(Vector(vecMaxs.x, vecMins.y, vecMaxs.z));
	EndRender();

	BeginRenderLines();
		Vertex(Vector(vecMins.x, vecMaxs.y, vecMaxs.z));
		Vertex(Vector(vecMins.x, vecMins.y, vecMaxs.z));
	EndRender();

	BeginRenderLines();
		Vertex(Vector(vecMins.x, vecMaxs.y, vecMins.z));
		Vertex(Vector(vecMins.x, vecMins.y, vecMins.z));
	EndRender();

	BeginRenderLines();
		Vertex(Vector(vecMaxs.x, vecMaxs.y, vecMins.z));
		Vertex(Vector(vecMaxs.x, vecMins.y, vecMins.z));
	EndRender();
}

void CRenderingContext::RenderBox(const Vector& vecMins, const Vector& vecMaxs)
{
	Vector vecForward(vecMaxs.x - vecMins.x, 0, 0);
	Vector vecUp(0, vecMaxs.y - vecMins.y, 0);
	Vector vecRight(0, 0, vecMaxs.z - vecMins.z);

	// The back face
	BeginRenderTris();
		Normal(Vector(-1, 0, 0));
		TexCoord(0.0f, 0.0f);
		Vertex(vecMins);
		TexCoord(1.0f, 0.0f);
		Vertex(vecMins + vecRight);
		TexCoord(1.0f, 1.0f);
		Vertex(vecMins + vecRight + vecUp);
		TexCoord(0.0f, 0.0f);
		Vertex(vecMins);
		TexCoord(1.0f, 1.0f);
		Vertex(vecMins + vecRight + vecUp);
		TexCoord(0.0f, 1.0f);
		Vertex(vecMins + vecUp);

		// The left face
		Normal(Vector(0, 0, -1));
		TexCoord(0.0f, 0.0f);
		Vertex(vecMins);
		TexCoord(0.0f, 1.0f);
		Vertex(vecMins + vecUp);
		TexCoord(1.0f, 1.0f);
		Vertex(vecMins + vecUp + vecForward);
		TexCoord(0.0f, 0.0f);
		Vertex(vecMins);
		TexCoord(1.0f, 1.0f);
		Vertex(vecMins + vecUp + vecForward);
		TexCoord(1.0f, 0.0f);
		Vertex(vecMins + vecForward);

		// The bottom face
		Normal(Vector(0, -1, 0));
		TexCoord(0.0f, 0.0f);
		Vertex(vecMins);
		TexCoord(1.0f, 0.0f);
		Vertex(vecMins + vecForward);
		TexCoord(1.0f, 1.0f);
		Vertex(vecMins + vecForward + vecRight);
		TexCoord(0.0f, 0.0f);
		Vertex(vecMins);
		TexCoord(1.0f, 1.0f);
		Vertex(vecMins + vecForward + vecRight);
		TexCoord(0.0f, 1.0f);
		Vertex(vecMins + vecRight);

		// The top face
		Normal(Vector(0, 1, 0));
		TexCoord(0.0f, 1.0f);
		Vertex(vecMins + vecRight + vecUp + vecForward);
		TexCoord(0.0f, 0.0f);
		Vertex(vecMins + vecUp + vecForward);
		TexCoord(1.0f, 0.0f);
		Vertex(vecMins + vecUp);
		TexCoord(0.0f, 1.0f);
		Vertex(vecMins + vecRight + vecUp + vecForward);
		TexCoord(1.0f, 0.0f);
		Vertex(vecMins + vecUp);
		TexCoord(1.0f, 1.0f);
		Vertex(vecMins + vecRight + vecUp);

		// The right face
		Normal(Vector(0, 0, 1));
		TexCoord(0.0f, 1.0f);
		Vertex(vecMins + vecRight + vecUp + vecForward);
		TexCoord(1.0f, 1.0f);
		Vertex(vecMins + vecRight + vecUp);
		TexCoord(1.0f, 0.0f);
		Vertex(vecMins + vecRight);
		TexCoord(0.0f, 1.0f);
		Vertex(vecMins + vecRight + vecUp + vecForward);
		TexCoord(1.0f, 0.0f);
		Vertex(vecMins + vecRight);
		TexCoord(0.0f, 0.0f);
		Vertex(vecMins + vecForward + vecRight);

		// The front face
		Normal(Vector(1, 0, 0));
		TexCoord(0.0f, 1.0f);
		Vertex(vecMins + vecRight + vecUp + vecForward);
		TexCoord(0.0f, 0.0f);
		Vertex(vecMins + vecForward + vecRight);
		TexCoord(1.0f, 0.0f);
		Vertex(vecMins + vecForward);
		TexCoord(0.0f, 1.0f);
		Vertex(vecMins + vecRight + vecUp + vecForward);
		TexCoord(1.0f, 0.0f);
		Vertex(vecMins + vecForward);
		TexCoord(1.0f, 1.0f);
		Vertex(vecMins + vecForward + vecUp);
	EndRender();
}

void CRenderingContext::RenderBillboard(size_t iTexture, float flRadius, Vector vecUp, Vector vecRight)
{
	vecUp = vecUp * flRadius;
	vecRight = vecRight * flRadius;

	BindTexture(iTexture);

	// The up and right vectors form an orthogonal basis for the plane that the billboarded sprite is in.
	// That means we can use combinations of these two vectors to find the four vertices of the sprite.
	// http://youtu.be/puOTwCrEm7Q

	// Pass the vertices of the billboarded sprite into the renderer in counter-clockwise order.
	BeginRenderTriFan();
		TexCoord(0.0f, 0.0f);
		Vertex(-vecUp - vecRight);

		TexCoord(1.0f, 0.0f);
		Vertex(-vecUp + vecRight);

		TexCoord(1.0f, 1.0f);
		Vertex(vecUp + vecRight);

		TexCoord(0.0f, 1.0f);
		Vertex(vecUp - vecRight);
	EndRender();
}

void CRenderingContext::UseProgram(const char* pszProgram)
{
	CRenderContext& oContext = GetContext();

	strncpy(oContext.m_szProgram, pszProgram, PROGRAM_LEN);

	oContext.m_pShader = m_pShader = CShaderLibrary::GetShader(pszProgram);

	UseProgram(m_pShader);
}

void CRenderingContext::UseProgram(class CShader* pShader)
{
	CRenderContext& oContext = GetContext();

	oContext.m_pShader = m_pShader = pShader;

	if (!m_pShader)
	{
		oContext.m_szProgram[0] = '\0';
		m_iProgram = 0;
		GLCall(glUseProgram(0));
		return;
	}

	strncpy(oContext.m_szProgram, pShader->m_sName.c_str(), PROGRAM_LEN);

	m_iProgram = m_pShader->m_iProgram;
	GLCall(glUseProgram((GLuint)m_pShader->m_iProgram));

	oContext.m_bProjectionUpdated = false;
	oContext.m_bViewUpdated = false;
}

void CRenderingContext::SetUniform(const char* pszName, int iValue)
{
	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	TAssert(iUniform > 0);
	GLCall(glUniform1i(iUniform, iValue));
}

void CRenderingContext::SetUniform(const char* pszName, float flValue)
{
	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	TAssert(iUniform > 0);
	GLCall(glUniform1f(iUniform, flValue));
}

void CRenderingContext::SetUniform(const char* pszName, const Vector& vecValue)
{
	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	TAssert(iUniform > 0);
	GLCall(glUniform3fv(iUniform, 1, &vecValue.x));
}

void CRenderingContext::SetUniform(const char* pszName, const Vector4D& vecValue)
{
	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	TAssert(iUniform > 0);
	GLCall(glUniform4fv(iUniform, 1, vecValue));
}

void CRenderingContext::SetUniform(const char* pszName, const ::Color& clrValue)
{
	Vector4D vecValue(clrValue);  // Convert it to a vector so it's a 0-1 value and not a 0-255 value
	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	TAssert(iUniform > 0);
	GLCall(glUniform4fv(iUniform, 1, &vecValue.x));
}

void CRenderingContext::SetUniform(const char* pszName, const Matrix4x4& mValue)
{
	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	TAssert(iUniform > 0);
	GLCall(glUniformMatrix4fv(iUniform, 1, false, mValue));
}

void CRenderingContext::SetUniform(const char* pszName, size_t iSize, const float* aflValues)
{
	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	TAssert(iUniform > 0);
	GLCall(glUniform1fv(iUniform, iSize, aflValues));
}

void CRenderingContext::BindTexture(size_t iTexture, int iChannel, bool bMultisample)
{
	GLCall(glActiveTexture(GL_TEXTURE0+iChannel));

	if (bMultisample)
		GLCall(glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, (GLuint)iTexture));
	else
		GLCall(glBindTexture(GL_TEXTURE_2D, (GLuint)iTexture));
}

void CRenderingContext::SetColor(const ::Color& c)
{
	m_clrRender = c;
}

void CRenderingContext::BeginRenderTris()
{
	s_dynamic_verts.clear();
	m_num_verts = 0;

	m_bTexCoord = false;
	m_bNormal = false;
	m_bTangents = false;

	m_iDrawMode = GL_TRIANGLES;
}

void CRenderingContext::BeginRenderTriFan()
{
	s_dynamic_verts.clear();
	m_num_verts = 0;

	m_bTexCoord = false;
	m_bNormal = false;
	m_bTangents = false;

	m_iDrawMode = GL_TRIANGLE_FAN;
}

void CRenderingContext::BeginRenderTriStrip()
{
	s_dynamic_verts.clear();
	m_num_verts = 0;

	m_bTexCoord = false;
	m_bNormal = false;
	m_bTangents = false;

	m_iDrawMode = GL_TRIANGLE_STRIP;
}

void CRenderingContext::BeginRenderLines()
{
	s_dynamic_verts.clear();
	m_num_verts = 0;

	m_bTexCoord = false;
	m_bNormal = false;
	m_bTangents = false;

	m_iDrawMode = GL_LINES;
}

void CRenderingContext::BeginRenderLineLoop()
{
	s_dynamic_verts.clear();
	m_num_verts = 0;

	m_bTexCoord = false;
	m_bNormal = false;
	m_bTangents = false;

	m_iDrawMode = GL_LINE_LOOP;
}

void CRenderingContext::BeginRenderLineStrip()
{
	s_dynamic_verts.clear();
	m_num_verts = 0;

	m_bTexCoord = false;
	m_bNormal = false;
	m_bTangents = false;

	m_iDrawMode = GL_LINE_STRIP;
}

void CRenderingContext::BeginRenderDebugLines()
{
	BeginRenderLines();
}

void CRenderingContext::BeginRenderPoints(float flSize)
{
	s_dynamic_verts.clear();
	m_num_verts = 0;

	m_bTexCoord = false;
	m_bNormal = false;
	m_bTangents = false;

	GLCall(glPointSize( flSize ));
	m_iDrawMode = GL_POINTS;
}

void CRenderingContext::TexCoord(float s, float t, int iChannel)
{
	m_texcoords = Vector2D(s, t);

	m_bTexCoord = true;
}

void CRenderingContext::TexCoord(const Vector2D& v, int iChannel)
{
	m_texcoords = v;

	m_bTexCoord = true;
}

void CRenderingContext::TexCoord(const Vector& v, int iChannel)
{
	m_texcoords = v;

	m_bTexCoord = true;
}

void CRenderingContext::Normal(const Vector& v)
{
	m_normal = v;
	m_bNormal = true;
}

void CRenderingContext::Tangent(const Vector& v)
{
	TAssert(m_bNormal);
	m_tangent = v;
	m_bTangents = true;
}

void CRenderingContext::Bitangent(const Vector& v)
{
	TAssert(m_bNormal);
	m_bitangent = v;
	m_bTangents = true;
}

void CRenderingContext::Vertex(const Vector& v)
{
	m_num_verts++;

	s_dynamic_verts.push_back(v.x);
	s_dynamic_verts.push_back(v.y);
	s_dynamic_verts.push_back(v.z);

	if (m_bTexCoord)
	{
		s_dynamic_verts.push_back(m_texcoords.x);
		s_dynamic_verts.push_back(m_texcoords.y);
	}

	if (m_bNormal)
	{
		s_dynamic_verts.push_back(m_normal.x);
		s_dynamic_verts.push_back(m_normal.y);
		s_dynamic_verts.push_back(m_normal.z);
	}

	if (m_bTangents)
	{
		s_dynamic_verts.push_back(m_tangent.x);
		s_dynamic_verts.push_back(m_tangent.y);
		s_dynamic_verts.push_back(m_tangent.z);
		s_dynamic_verts.push_back(m_bitangent.x);
		s_dynamic_verts.push_back(m_bitangent.y);
		s_dynamic_verts.push_back(m_bitangent.z);
	}
}

void CRenderingContext::EndRender()
{
	if (!m_pShader)
	{
		UseProgram("model");
		if (!m_pShader)
			return;
	}

	CRenderContext& oContext = GetContext();

	if (!oContext.m_bProjectionUpdated)
		SetUniform("mProjection", oContext.m_mProjection);

	if (!oContext.m_bViewUpdated)
		SetUniform("mView", oContext.m_mView);

	if (!oContext.m_bTransformUpdated)
		SetUniform("mGlobal", oContext.m_mTransformations);

	oContext.m_bProjectionUpdated = oContext.m_bViewUpdated = oContext.m_bTransformUpdated = true;

	GLCall(glBindVertexArray(m_pRenderer->m_default_vao));

	GLCall(glBindBuffer(GL_ARRAY_BUFFER, m_pRenderer->m_dynamic_mesh_vbo));
	GLCall(glBufferData(GL_ARRAY_BUFFER, GLsizeiptr((size_t)s_dynamic_verts.size() * sizeof(float)), s_dynamic_verts.data(), GL_STATIC_DRAW));

	int stride = 3 * sizeof(float);
	int position_offset = 0;
	int texcoord_offset = stride;
	int normal_offset = stride;
	int tangent_offset = stride;
	int bitangent_offset = stride;

	if (m_bTexCoord)
	{
		int uv_size = 2 * sizeof(float);
		stride += uv_size;
		normal_offset += uv_size;
		tangent_offset += uv_size;
		bitangent_offset += uv_size;
	}

	if (m_bNormal)
	{
		int normal_size = 3 * sizeof(float);
		stride += normal_size;
		tangent_offset += normal_size;
		bitangent_offset += normal_size;
	}

	if (m_bTangents)
	{
		int tangents_size = 6 * sizeof(float);
		stride += tangents_size;
		bitangent_offset += 3 * sizeof(float);
	}

	if (m_bTexCoord)
	{
		for (size_t i = 0; i < MAX_TEXTURE_CHANNELS; i++)
		{
			if (m_pShader->m_aiTexCoordAttributes[i] != ~0)
			{
				GLCall(glEnableVertexAttribArray(m_pShader->m_aiTexCoordAttributes[i]));
				GLCall(glVertexAttribPointer(m_pShader->m_aiTexCoordAttributes[i], 2, GL_FLOAT, false, stride, BUFFER_OFFSET(texcoord_offset)));
			}
		}
	}

	if (m_bNormal && m_pShader->m_iNormalAttribute != ~0)
	{
		GLCall(glEnableVertexAttribArray(m_pShader->m_iNormalAttribute));
		GLCall(glVertexAttribPointer(m_pShader->m_iNormalAttribute, 3, GL_FLOAT, false, stride, BUFFER_OFFSET(normal_offset)));
	}

	if (m_bTangents && m_pShader->m_iTangentAttribute != ~0 && m_pShader->m_iBitangentAttribute != ~0)
	{
		GLCall(glEnableVertexAttribArray(m_pShader->m_iTangentAttribute));
		GLCall(glVertexAttribPointer(m_pShader->m_iTangentAttribute, 3, GL_FLOAT, false, stride, BUFFER_OFFSET(tangent_offset)));

		GLCall(glEnableVertexAttribArray(m_pShader->m_iBitangentAttribute));
		GLCall(glVertexAttribPointer(m_pShader->m_iBitangentAttribute, 3, GL_FLOAT, false, stride, BUFFER_OFFSET(bitangent_offset)));
	}

	GLCall(glEnableVertexAttribArray(m_pShader->m_iPositionAttribute));
	GLCall(glVertexAttribPointer(m_pShader->m_iPositionAttribute, 3, GL_FLOAT, false, stride, BUFFER_OFFSET(0)));

	GLCall(glDrawArrays(m_iDrawMode, 0, m_num_verts));

	GLCall(glDisableVertexAttribArray(m_pShader->m_iPositionAttribute));
	for (size_t i = 0; i < MAX_TEXTURE_CHANNELS; i++)
	{
		if (m_pShader->m_aiTexCoordAttributes[i] != ~0)
			GLCall(glDisableVertexAttribArray(m_pShader->m_aiTexCoordAttributes[i]));
	}
	if (m_pShader->m_iNormalAttribute != ~0)
		GLCall(glDisableVertexAttribArray(m_pShader->m_iNormalAttribute));
	if (m_pShader->m_iTangentAttribute != ~0)
		GLCall(glDisableVertexAttribArray(m_pShader->m_iTangentAttribute));
	if (m_pShader->m_iBitangentAttribute != ~0)
		GLCall(glDisableVertexAttribArray(m_pShader->m_iBitangentAttribute));
	if (m_pShader->m_iColorAttribute != ~0)
		GLCall(glDisableVertexAttribArray(m_pShader->m_iColorAttribute));
}

void CRenderingContext::CreateVBO(size_t& iVBO, size_t& iVBOSize)
{
	TAssert(m_pRenderer);
	if (!m_pRenderer)
		return;

	TAssert(s_dynamic_verts.size());
	if (!s_dynamic_verts.size())
		return;

	iVBOSize = (size_t)s_dynamic_verts.size() * sizeof(s_dynamic_verts[0]);
	iVBO = m_pRenderer->LoadVertexDataIntoGL(iVBOSize, s_dynamic_verts.data());
}

void CRenderingContext::BeginRenderVertexArray(size_t iBuffer)
{
	if (iBuffer)
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, iBuffer));
}

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

void CRenderingContext::SetPositionBuffer(float* pflBuffer, size_t iStride)
{
	TAssert(m_pShader->m_iPositionAttribute != ~0);
	GLCall(glEnableVertexAttribArray(m_pShader->m_iPositionAttribute));
	GLCall(glVertexAttribPointer(m_pShader->m_iPositionAttribute, 3, GL_FLOAT, false, iStride, pflBuffer));
}

void CRenderingContext::SetPositionBuffer(size_t iOffset, size_t iStride)
{
	TAssert(iOffset%4 == 0);	// Should be multiples of four because it's offsets in bytes and we're always working with floats or doubles
	TAssert(m_pShader->m_iPositionAttribute != ~0);
	GLCall(glEnableVertexAttribArray(m_pShader->m_iPositionAttribute));
	GLCall(glVertexAttribPointer(m_pShader->m_iPositionAttribute, 3, GL_FLOAT, false, iStride, BUFFER_OFFSET(iOffset)));
}

void CRenderingContext::SetNormalsBuffer(float* pflBuffer, size_t iStride)
{
	if (m_pShader->m_iNormalAttribute == ~0)
		return;

	GLCall(glEnableVertexAttribArray(m_pShader->m_iNormalAttribute));
	GLCall(glVertexAttribPointer(m_pShader->m_iNormalAttribute, 3, GL_FLOAT, false, iStride, pflBuffer));
}

void CRenderingContext::SetNormalsBuffer(size_t iOffset, size_t iStride)
{
	if (m_pShader->m_iNormalAttribute == ~0)
		return;

	TAssert(iOffset%4 == 0);	// Should be multiples of four because it's offsets in bytes and we're always working with floats or doubles
	GLCall(glEnableVertexAttribArray(m_pShader->m_iNormalAttribute));
	GLCall(glVertexAttribPointer(m_pShader->m_iNormalAttribute, 3, GL_FLOAT, false, iStride, BUFFER_OFFSET(iOffset)));
}

void CRenderingContext::SetTangentsBuffer(float* pflBuffer, size_t iStride)
{
	if (m_pShader->m_iTangentAttribute == ~0)
		return;

	GLCall(glEnableVertexAttribArray(m_pShader->m_iTangentAttribute));
	GLCall(glVertexAttribPointer(m_pShader->m_iTangentAttribute, 3, GL_FLOAT, false, iStride, pflBuffer));
}

void CRenderingContext::SetTangentsBuffer(size_t iOffset, size_t iStride)
{
	if (m_pShader->m_iTangentAttribute == ~0)
		return;

	TAssert(iOffset%4 == 0);	// Should be multiples of four because it's offsets in bytes and we're always working with floats or doubles
	GLCall(glEnableVertexAttribArray(m_pShader->m_iTangentAttribute));
	GLCall(glVertexAttribPointer(m_pShader->m_iTangentAttribute, 3, GL_FLOAT, false, iStride, BUFFER_OFFSET(iOffset)));
}

void CRenderingContext::SetBitangentsBuffer(float* pflBuffer, size_t iStride)
{
	if (m_pShader->m_iBitangentAttribute == ~0)
		return;

	GLCall(glEnableVertexAttribArray(m_pShader->m_iBitangentAttribute));
	GLCall(glVertexAttribPointer(m_pShader->m_iBitangentAttribute, 3, GL_FLOAT, false, iStride, pflBuffer));
}

void CRenderingContext::SetBitangentsBuffer(size_t iOffset, size_t iStride)
{
	if (m_pShader->m_iBitangentAttribute == ~0)
		return;

	TAssert(iOffset%4 == 0);	// Should be multiples of four because it's offsets in bytes and we're always working with floats or doubles
	GLCall(glEnableVertexAttribArray(m_pShader->m_iBitangentAttribute));
	GLCall(glVertexAttribPointer(m_pShader->m_iBitangentAttribute, 3, GL_FLOAT, false, iStride, BUFFER_OFFSET(iOffset)));
}

void CRenderingContext::SetTexCoordBuffer(float* pflBuffer, size_t iStride, size_t iChannel)
{
	TAssert(iChannel >= 0 && iChannel < MAX_TEXTURE_CHANNELS);

	if (m_pShader->m_aiTexCoordAttributes[iChannel] == ~0)
		return;

	GLCall(glEnableVertexAttribArray(m_pShader->m_aiTexCoordAttributes[iChannel]));
	GLCall(glVertexAttribPointer(m_pShader->m_aiTexCoordAttributes[iChannel], 2, GL_FLOAT, false, iStride, pflBuffer));
}

void CRenderingContext::SetTexCoordBuffer(size_t iOffset, size_t iStride, size_t iChannel)
{
	TAssert(iChannel >= 0 && iChannel < MAX_TEXTURE_CHANNELS);

	if (m_pShader->m_aiTexCoordAttributes[iChannel] == ~0)
		return;

	TAssert(iOffset%4 == 0);	// Should be multiples of four because it's offsets in bytes and we're always working with floats or doubles
	GLCall(glEnableVertexAttribArray(m_pShader->m_aiTexCoordAttributes[iChannel]));
	GLCall(glVertexAttribPointer(m_pShader->m_aiTexCoordAttributes[iChannel], 2, GL_FLOAT, false, iStride, BUFFER_OFFSET(iOffset)));
}

void CRenderingContext::SetCustomIntBuffer(const char* pszName, size_t iSize, size_t iOffset, size_t iStride)
{
	int iAttribute = glGetAttribLocation(m_iProgram, pszName);

	TAssert(iAttribute != ~0);
	if (iAttribute == ~0)
		return;

	TAssert(iOffset%4 == 0);	// Should be multiples of four because it's offsets in bytes and we're always working with floats or doubles
	GLCall(glEnableVertexAttribArray(iAttribute));
	GLCall(glVertexAttribIPointer(iAttribute, iSize, GL_INT, iStride, BUFFER_OFFSET(iOffset)));
}

void CRenderingContext::EndRenderVertexArray(size_t iVertices, bool bWireframe)
{
	CRenderContext& oContext = GetContext();

	if (!oContext.m_bProjectionUpdated)
		SetUniform("mProjection", oContext.m_mProjection);

	if (!oContext.m_bViewUpdated)
		SetUniform("mView", oContext.m_mView);

	if (!oContext.m_bTransformUpdated)
		SetUniform("mGlobal", oContext.m_mTransformations);

	oContext.m_bProjectionUpdated = oContext.m_bViewUpdated = oContext.m_bTransformUpdated = true;

	if (bWireframe)
	{
		GLCall(glLineWidth(1));
		GLCall(glDrawArrays(GL_LINES, 0, iVertices));
	}
	else
		GLCall(glDrawArrays(GL_TRIANGLES, 0, iVertices));

	GLCall(glDisableVertexAttribArray(m_pShader->m_iPositionAttribute));
	for (size_t i = 0; i < MAX_TEXTURE_CHANNELS; i++)
	{
		if (m_pShader->m_aiTexCoordAttributes[i] != ~0)
			GLCall(glDisableVertexAttribArray(m_pShader->m_aiTexCoordAttributes[i]));
	}
	if (m_pShader->m_iNormalAttribute != ~0)
		GLCall(glDisableVertexAttribArray(m_pShader->m_iNormalAttribute));
	if (m_pShader->m_iTangentAttribute != ~0)
		GLCall(glDisableVertexAttribArray(m_pShader->m_iTangentAttribute));
	if (m_pShader->m_iBitangentAttribute != ~0)
		GLCall(glDisableVertexAttribArray(m_pShader->m_iBitangentAttribute));
	if (m_pShader->m_iColorAttribute != ~0)
		GLCall(glDisableVertexAttribArray(m_pShader->m_iColorAttribute));

	GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void CRenderingContext::EndRenderVertexArrayTriangles(size_t iTriangles, int* piIndices)
{
	CRenderContext& oContext = GetContext();

	if (!oContext.m_bProjectionUpdated)
		SetUniform("mProjection", oContext.m_mProjection);

	if (!oContext.m_bViewUpdated)
		SetUniform("mView", oContext.m_mView);

	if (!oContext.m_bTransformUpdated)
		SetUniform("mGlobal", oContext.m_mTransformations);

	oContext.m_bProjectionUpdated = oContext.m_bViewUpdated = oContext.m_bTransformUpdated = true;

	GLCall(glDrawElements(GL_TRIANGLES, iTriangles*3, GL_UNSIGNED_INT, piIndices));

	GLCall(glDisableVertexAttribArray(m_pShader->m_iPositionAttribute));
	for (size_t i = 0; i < MAX_TEXTURE_CHANNELS; i++)
	{
		if (m_pShader->m_aiTexCoordAttributes[i] != ~0)
			GLCall(glDisableVertexAttribArray(m_pShader->m_aiTexCoordAttributes[i]));
	}
	if (m_pShader->m_iNormalAttribute != ~0)
		GLCall(glDisableVertexAttribArray(m_pShader->m_iNormalAttribute));
	if (m_pShader->m_iTangentAttribute != ~0)
		GLCall(glDisableVertexAttribArray(m_pShader->m_iTangentAttribute));
	if (m_pShader->m_iBitangentAttribute != ~0)
		GLCall(glDisableVertexAttribArray(m_pShader->m_iBitangentAttribute));
	if (m_pShader->m_iColorAttribute != ~0)
		GLCall(glDisableVertexAttribArray(m_pShader->m_iColorAttribute));

	GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void CRenderingContext::EndRenderVertexArrayIndexed(size_t iBuffer, size_t iVertices)
{
	CRenderContext& oContext = GetContext();

	if (!oContext.m_bProjectionUpdated)
		SetUniform("mProjection", oContext.m_mProjection);

	if (!oContext.m_bViewUpdated)
		SetUniform("mView", oContext.m_mView);

	if (!oContext.m_bTransformUpdated)
		SetUniform("mGlobal", oContext.m_mTransformations);

	oContext.m_bProjectionUpdated = oContext.m_bViewUpdated = oContext.m_bTransformUpdated = true;

	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iBuffer));

	bool bWireframe = false;
	GLCall(glDrawElements(bWireframe?GL_LINES:GL_TRIANGLES, iVertices, GL_UNSIGNED_INT, nullptr));

	GLCall(glDisableVertexAttribArray(m_pShader->m_iPositionAttribute));
	for (size_t i = 0; i < MAX_TEXTURE_CHANNELS; i++)
	{
		if (m_pShader->m_aiTexCoordAttributes[i] != ~0)
			GLCall(glDisableVertexAttribArray(m_pShader->m_aiTexCoordAttributes[i]));
	}
	if (m_pShader->m_iNormalAttribute != ~0)
		GLCall(glDisableVertexAttribArray(m_pShader->m_iNormalAttribute));
	if (m_pShader->m_iTangentAttribute != ~0)
		GLCall(glDisableVertexAttribArray(m_pShader->m_iTangentAttribute));
	if (m_pShader->m_iBitangentAttribute != ~0)
		GLCall(glDisableVertexAttribArray(m_pShader->m_iBitangentAttribute));
	if (m_pShader->m_iColorAttribute != ~0)
		GLCall(glDisableVertexAttribArray(m_pShader->m_iColorAttribute));

	GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
	GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0));
}

void CRenderingContext::ReadPixels(size_t x, size_t y, size_t w, size_t h, Vector4D* pvecPixels)
{
	GLCall(glReadPixels(x, y, w, h, GL_RGBA, GL_FLOAT, pvecPixels));
}

void CRenderingContext::Finish()
{
	GLCall(glFinish());
}

CRenderingContext::CRenderContext::CRenderContext()
{
	m_bProjectionUpdated = false;
	m_bViewUpdated = false;
	m_bTransformUpdated = false;
}
