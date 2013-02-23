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

#include <GL3/gl3w.h>
#include <GL/glu.h>

#include <common.h>

#include <renderer/shaders.h>
#include <renderer/application.h>
#include <renderer/renderer.h>

using namespace std;

vector<Vector2D> CRenderingContext::s_avecTexCoord;
vector<vector<Vector2D> > CRenderingContext::s_aavecTexCoords;
vector<Vector> CRenderingContext::s_avecNormals;
vector<Vector> CRenderingContext::s_avecVertices;

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
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		if (m_pRenderer)
			glViewport(0, 0, (GLsizei)m_pRenderer->m_iWidth, (GLsizei)m_pRenderer->m_iHeight);
		else
			glViewport(0, 0, (GLsizei)Application()->GetWindowWidth(), (GLsizei)Application()->GetWindowHeight());

		glUseProgram(0);

		glDisablei(GL_BLEND, 0);

		glDepthMask(true);
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glDepthFunc(GL_LESS);

		glFrontFace(GL_CCW);
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
		glEnablei(GL_BLEND, 0);

		if (eBlend == BLEND_ALPHA)
			glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		else if (eBlend == BLEND_ADDITIVE)
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		else if (eBlend == BLEND_BOTH)
			glBlendFunc(GL_ONE, GL_ONE);
	}
	else
		glDisablei(GL_BLEND, 0);

	GetContext().m_eBlend = eBlend;
}

void CRenderingContext::SetDepthMask(bool bDepthMask)
{
	glDepthMask(bDepthMask);
	GetContext().m_bDepthMask = bDepthMask;
}

void CRenderingContext::SetDepthTest(bool bDepthTest)
{
	if (bDepthTest)
		glEnable(GL_DEPTH_TEST);
	else
		glDisable(GL_DEPTH_TEST);
	GetContext().m_bDepthTest = bDepthTest;
}

void CRenderingContext::SetDepthFunction(depth_function_t eDepthFunction)
{
	if (eDepthFunction == DF_LEQUAL)
		glDepthFunc(GL_LEQUAL);
	else if (eDepthFunction == DF_LESS)
		glDepthFunc(GL_LESS);

	GetContext().m_eDepthFunction = eDepthFunction;
}

void CRenderingContext::SetBackCulling(bool bCull)
{
	if (bCull)
		glEnable(GL_CULL_FACE);
	else
		glDisable(GL_CULL_FACE);
	GetContext().m_bCull = bCull;
}

void CRenderingContext::SetWinding(bool bWinding)
{
	GetContext().m_bWinding = bWinding;
	glFrontFace(bWinding?GL_CCW:GL_CW);
}

void CRenderingContext::ClearColor(const ::Color& clrClear)
{
	glClearColor((float)(clrClear.r())/255, (float)(clrClear.g())/255, (float)(clrClear.b())/255, (float)(clrClear.a())/255);
	glClear(GL_COLOR_BUFFER_BIT);
}

void CRenderingContext::ClearDepth()
{
	glClear(GL_DEPTH_BUFFER_BIT);
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
	BeginRenderTriFan();
		Normal(Vector(-1, 0, 0));
		TexCoord(0.0f, 0.0f);
		Vertex(vecMins);
		TexCoord(1.0f, 0.0f);
		Vertex(vecMins + vecRight);
		TexCoord(1.0f, 1.0f);
		Vertex(vecMins + vecRight + vecUp);
		TexCoord(0.0f, 1.0f);
		Vertex(vecMins + vecUp);
	EndRender();

	// The left face
	BeginRenderTriFan();
		Normal(Vector(0, 0, -1));
		TexCoord(0.0f, 0.0f);
		Vertex(vecMins);
		TexCoord(0.0f, 1.0f);
		Vertex(vecMins + vecUp);
		TexCoord(1.0f, 1.0f);
		Vertex(vecMins + vecUp + vecForward);
		TexCoord(1.0f, 0.0f);
		Vertex(vecMins + vecForward);
	EndRender();

	// The bottom face
	BeginRenderTriFan();
		Normal(Vector(0, -1, 0));
		TexCoord(0.0f, 0.0f);
		Vertex(vecMins);
		TexCoord(1.0f, 0.0f);
		Vertex(vecMins + vecForward);
		TexCoord(1.0f, 1.0f);
		Vertex(vecMins + vecForward + vecRight);
		TexCoord(0.0f, 1.0f);
		Vertex(vecMins + vecRight);
	EndRender();

	// The top face
	BeginRenderTriFan();
		Normal(Vector(0, 1, 0));
		TexCoord(0.0f, 1.0f);
		Vertex(vecMins + vecRight + vecUp + vecForward);
		TexCoord(0.0f, 0.0f);
		Vertex(vecMins + vecUp + vecForward);
		TexCoord(1.0f, 0.0f);
		Vertex(vecMins + vecUp);
		TexCoord(1.0f, 1.0f);
		Vertex(vecMins + vecRight + vecUp);
	EndRender();

	// The right face
	BeginRenderTriFan();
		Normal(Vector(0, 0, 1));
		TexCoord(0.0f, 1.0f);
		Vertex(vecMins + vecRight + vecUp + vecForward);
		TexCoord(1.0f, 1.0f);
		Vertex(vecMins + vecRight + vecUp);
		TexCoord(1.0f, 0.0f);
		Vertex(vecMins + vecRight);
		TexCoord(0.0f, 0.0f);
		Vertex(vecMins + vecForward + vecRight);
	EndRender();

	// The front face
	BeginRenderTriFan();
		Normal(Vector(1, 0, 0));
		TexCoord(0.0f, 1.0f);
		Vertex(vecMins + vecRight + vecUp + vecForward);
		TexCoord(0.0f, 0.0f);
		Vertex(vecMins + vecForward + vecRight);
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

	BeginRenderTriFan();
		TexCoord(0.0f, 1.0f);
		Vertex(-vecRight + vecUp);
		TexCoord(0.0f, 0.0f);
		Vertex(-vecRight - vecUp);
		TexCoord(1.0f, 0.0f);
		Vertex(vecRight - vecUp);
		TexCoord(1.0f, 1.0f);
		Vertex(vecRight + vecUp);
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
		glUseProgram(0);
		return;
	}

	strncpy(oContext.m_szProgram, pShader->m_sName.c_str(), PROGRAM_LEN);

	m_iProgram = m_pShader->m_iProgram;
	glUseProgram((GLuint)m_pShader->m_iProgram);

	oContext.m_bProjectionUpdated = false;
	oContext.m_bViewUpdated = false;
}

void CRenderingContext::SetUniform(const char* pszName, int iValue)
{
	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniform1i(iUniform, iValue);
}

void CRenderingContext::SetUniform(const char* pszName, float flValue)
{
	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniform1f(iUniform, flValue);
}

void CRenderingContext::SetUniform(const char* pszName, const Vector& vecValue)
{
	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniform3fv(iUniform, 1, &vecValue.x);
}

void CRenderingContext::SetUniform(const char* pszName, const Vector4D& vecValue)
{
	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniform4fv(iUniform, 1, vecValue);
}

void CRenderingContext::SetUniform(const char* pszName, const ::Color& clrValue)
{
	Vector4D vecValue(clrValue);  // Convert it to a vector so it's a 0-1 value and not a 0-255 value
	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniform4fv(iUniform, 1, &vecValue.x);
}

void CRenderingContext::SetUniform(const char* pszName, const Matrix4x4& mValue)
{
	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniformMatrix4fv(iUniform, 1, false, mValue);
}

void CRenderingContext::SetUniform(const char* pszName, size_t iSize, const float* aflValues)
{
	int iUniform = glGetUniformLocation((GLuint)m_iProgram, pszName);
	glUniform1fv(iUniform, iSize, aflValues);
}

void CRenderingContext::BindTexture(size_t iTexture, int iChannel, bool bMultisample)
{
	glActiveTexture(GL_TEXTURE0+iChannel);

	if (bMultisample)
		glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, (GLuint)iTexture);
	else
		glBindTexture(GL_TEXTURE_2D, (GLuint)iTexture);
}

void CRenderingContext::SetColor(const ::Color& c)
{
	m_clrRender = c;
}

void CRenderingContext::BeginRenderTris()
{
	s_avecTexCoord.clear();
	for (size_t i = 0; i < s_aavecTexCoords.size(); i++)
		s_aavecTexCoords[i].clear();
	s_avecNormals.clear();
	s_avecVertices.clear();

	m_bTexCoord = false;
	m_bNormal = false;

	m_iDrawMode = GL_TRIANGLES;
}

void CRenderingContext::BeginRenderTriFan()
{
	s_avecTexCoord.clear();
	for (size_t i = 0; i < s_aavecTexCoords.size(); i++)
		s_aavecTexCoords[i].clear();
	s_avecNormals.clear();
	s_avecVertices.clear();

	m_bTexCoord = false;
	m_bNormal = false;

	m_iDrawMode = GL_TRIANGLE_FAN;
}

void CRenderingContext::BeginRenderTriStrip()
{
	s_avecTexCoord.clear();
	for (size_t i = 0; i < s_aavecTexCoords.size(); i++)
		s_aavecTexCoords[i].clear();
	s_avecNormals.clear();
	s_avecVertices.clear();

	m_bTexCoord = false;
	m_bNormal = false;

	m_iDrawMode = GL_TRIANGLE_STRIP;
}

void CRenderingContext::BeginRenderLines()
{
	s_avecTexCoord.clear();
	for (size_t i = 0; i < s_aavecTexCoords.size(); i++)
		s_aavecTexCoords[i].clear();
	s_avecNormals.clear();
	s_avecVertices.clear();

	m_bTexCoord = false;
	m_bNormal = false;

	m_iDrawMode = GL_LINES;
}

void CRenderingContext::BeginRenderLineLoop()
{
	s_avecTexCoord.clear();
	for (size_t i = 0; i < s_aavecTexCoords.size(); i++)
		s_aavecTexCoords[i].clear();
	s_avecNormals.clear();
	s_avecVertices.clear();

	m_bTexCoord = false;
	m_bNormal = false;

	m_iDrawMode = GL_LINE_LOOP;
}

void CRenderingContext::BeginRenderLineStrip()
{
	s_avecTexCoord.clear();
	for (size_t i = 0; i < s_aavecTexCoords.size(); i++)
		s_aavecTexCoords[i].clear();
	s_avecNormals.clear();
	s_avecVertices.clear();

	m_bTexCoord = false;
	m_bNormal = false;

	m_iDrawMode = GL_LINE_STRIP;
}

void CRenderingContext::BeginRenderDebugLines()
{
	BeginRenderLines();
}

void CRenderingContext::BeginRenderPoints(float flSize)
{
	s_avecTexCoord.clear();
	for (size_t i = 0; i < s_aavecTexCoords.size(); i++)
		s_aavecTexCoords[i].clear();
	s_avecNormals.clear();
	s_avecVertices.clear();

	m_bTexCoord = false;
	m_bNormal = false;

	glPointSize( flSize );
	m_iDrawMode = GL_POINTS;
}

void CRenderingContext::TexCoord(float s, float t, int iChannel)
{
	if (iChannel >= (int)s_avecTexCoord.size())
		s_avecTexCoord.resize(iChannel+1);
	s_avecTexCoord[iChannel] = Vector2D(s, t);

	m_bTexCoord = true;
}

void CRenderingContext::TexCoord(const Vector2D& v, int iChannel)
{
	if (iChannel >= (int)s_avecTexCoord.size())
		s_avecTexCoord.resize(iChannel+1);
	s_avecTexCoord[iChannel] = v;

	m_bTexCoord = true;
}

void CRenderingContext::TexCoord(const Vector& v, int iChannel)
{
	if (iChannel >= (int)s_avecTexCoord.size())
		s_avecTexCoord.resize(iChannel+1);
	s_avecTexCoord[iChannel] = v;

	m_bTexCoord = true;
}

void CRenderingContext::Normal(const Vector& v)
{
	m_vecNormal = v;
	m_bNormal = true;
}

void CRenderingContext::Vertex(const Vector& v)
{
	if (m_bTexCoord)
	{
		if (s_aavecTexCoords.size() < s_avecTexCoord.size())
			s_aavecTexCoords.resize(s_avecTexCoord.size());

		for (size_t i = 0; i < s_aavecTexCoords.size(); i++)
			s_aavecTexCoords[i].push_back(s_avecTexCoord[i]);
	}

	if (m_bNormal)
		s_avecNormals.push_back(m_vecNormal);

	s_avecVertices.push_back(v);
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

	if (m_bTexCoord)
	{
		for (size_t i = 0; i < MAX_TEXTURE_CHANNELS; i++)
		{
			if (m_pShader->m_aiTexCoordAttributes[i] != ~0)
			{
				glEnableVertexAttribArray(m_pShader->m_aiTexCoordAttributes[i]);
				glVertexAttribPointer(m_pShader->m_aiTexCoordAttributes[i], 2, GL_FLOAT, false, 0, s_aavecTexCoords[0].data());
			}
		}
	}

	if (m_bNormal && m_pShader->m_iNormalAttribute != ~0)
	{
		glEnableVertexAttribArray(m_pShader->m_iNormalAttribute);
		glVertexAttribPointer(m_pShader->m_iNormalAttribute, 3, GL_FLOAT, false, 0, s_avecNormals.data());
	}

	glEnableVertexAttribArray(m_pShader->m_iPositionAttribute);
	glVertexAttribPointer(m_pShader->m_iPositionAttribute, 3, GL_FLOAT, false, 0, s_avecVertices.data());

	glDrawArrays(m_iDrawMode, 0, s_avecVertices.size());

	glDisableVertexAttribArray(m_pShader->m_iPositionAttribute);
	for (size_t i = 0; i < MAX_TEXTURE_CHANNELS; i++)
	{
		if (m_pShader->m_aiTexCoordAttributes[i] != ~0)
			glDisableVertexAttribArray(m_pShader->m_aiTexCoordAttributes[i]);
	}
	if (m_pShader->m_iNormalAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iNormalAttribute);
	if (m_pShader->m_iColorAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iColorAttribute);
}

void CRenderingContext::CreateVBO(size_t& iVBO, size_t& iVBOSize)
{
	TAssert(m_pRenderer);
	if (!m_pRenderer)
		return;

	TAssert(s_avecVertices.size());
	if (!s_avecVertices.size())
		return;

	TAssert(m_iDrawMode == GL_TRIANGLES);

	size_t iDataSize = 0;
	iDataSize += s_avecVertices.size()*3;

	for (size_t i = 0; i < s_aavecTexCoords.size(); i++)
		iDataSize += s_aavecTexCoords[i].size()*2;

	iDataSize += s_avecNormals.size()*3;

	vector<float> aflData;
	aflData.reserve(iDataSize);

	TAssert(!s_avecNormals.size() || s_avecVertices.size() == s_avecNormals.size());
	for (size_t i = 0; i < s_aavecTexCoords.size(); i++)
		TAssert(!s_aavecTexCoords[i].size() || s_avecVertices.size() == s_aavecTexCoords[i].size());

	for (size_t i = 0; i < s_avecVertices.size(); i++)
	{
		Vector& vecVert = s_avecVertices[i];
		aflData.push_back(vecVert.x);
		aflData.push_back(vecVert.y);
		aflData.push_back(vecVert.z);

		if (s_avecNormals.size())
		{
			Vector& vecNormal = s_avecNormals[i];
			aflData.push_back(vecNormal.x);
			aflData.push_back(vecNormal.y);
			aflData.push_back(vecNormal.z);
		}

		for (size_t j = 0; j < s_aavecTexCoords.size(); j++)
		{
			if (s_aavecTexCoords[j].size())
			{
				Vector2D& vecUV = s_aavecTexCoords[j][i];
				aflData.push_back(vecUV.x);
				aflData.push_back(vecUV.y);
			}
		}
	}

	iVBO = m_pRenderer->LoadVertexDataIntoGL(aflData.size()*sizeof(float), aflData.data());
	iVBOSize = s_avecVertices.size();
}

void CRenderingContext::BeginRenderVertexArray(size_t iBuffer)
{
	if (iBuffer)
		glBindBuffer(GL_ARRAY_BUFFER, iBuffer);
}

#define BUFFER_OFFSET(i) ((char *)NULL + (i))

void CRenderingContext::SetPositionBuffer(float* pflBuffer, size_t iStride)
{
	TAssert(m_pShader->m_iPositionAttribute != ~0);
	glEnableVertexAttribArray(m_pShader->m_iPositionAttribute);
	glVertexAttribPointer(m_pShader->m_iPositionAttribute, 3, GL_FLOAT, false, iStride, pflBuffer);
}

void CRenderingContext::SetPositionBuffer(size_t iOffset, size_t iStride)
{
	TAssert(iOffset%4 == 0);	// Should be multiples of four because it's offsets in bytes and we're always working with floats or doubles
	TAssert(m_pShader->m_iPositionAttribute != ~0);
	glEnableVertexAttribArray(m_pShader->m_iPositionAttribute);
	glVertexAttribPointer(m_pShader->m_iPositionAttribute, 3, GL_FLOAT, false, iStride, BUFFER_OFFSET(iOffset));
}

void CRenderingContext::SetNormalsBuffer(float* pflBuffer, size_t iStride)
{
	if (m_pShader->m_iNormalAttribute == ~0)
		return;

	glEnableVertexAttribArray(m_pShader->m_iNormalAttribute);
	glVertexAttribPointer(m_pShader->m_iNormalAttribute, 3, GL_FLOAT, false, iStride, pflBuffer);
}

void CRenderingContext::SetNormalsBuffer(size_t iOffset, size_t iStride)
{
	if (m_pShader->m_iNormalAttribute == ~0)
		return;

	TAssert(iOffset%4 == 0);	// Should be multiples of four because it's offsets in bytes and we're always working with floats or doubles
	glEnableVertexAttribArray(m_pShader->m_iNormalAttribute);
	glVertexAttribPointer(m_pShader->m_iNormalAttribute, 3, GL_FLOAT, false, iStride, BUFFER_OFFSET(iOffset));
}

void CRenderingContext::SetTangentsBuffer(float* pflBuffer, size_t iStride)
{
	if (m_pShader->m_iTangentAttribute == ~0)
		return;

	glEnableVertexAttribArray(m_pShader->m_iTangentAttribute);
	glVertexAttribPointer(m_pShader->m_iTangentAttribute, 3, GL_FLOAT, false, iStride, pflBuffer);
}

void CRenderingContext::SetTangentsBuffer(size_t iOffset, size_t iStride)
{
	if (m_pShader->m_iTangentAttribute == ~0)
		return;

	TAssert(iOffset%4 == 0);	// Should be multiples of four because it's offsets in bytes and we're always working with floats or doubles
	glEnableVertexAttribArray(m_pShader->m_iTangentAttribute);
	glVertexAttribPointer(m_pShader->m_iTangentAttribute, 3, GL_FLOAT, false, iStride, BUFFER_OFFSET(iOffset));
}

void CRenderingContext::SetBitangentsBuffer(float* pflBuffer, size_t iStride)
{
	if (m_pShader->m_iBitangentAttribute == ~0)
		return;

	glEnableVertexAttribArray(m_pShader->m_iBitangentAttribute);
	glVertexAttribPointer(m_pShader->m_iBitangentAttribute, 3, GL_FLOAT, false, iStride, pflBuffer);
}

void CRenderingContext::SetBitangentsBuffer(size_t iOffset, size_t iStride)
{
	if (m_pShader->m_iBitangentAttribute == ~0)
		return;

	TAssert(iOffset%4 == 0);	// Should be multiples of four because it's offsets in bytes and we're always working with floats or doubles
	glEnableVertexAttribArray(m_pShader->m_iBitangentAttribute);
	glVertexAttribPointer(m_pShader->m_iBitangentAttribute, 3, GL_FLOAT, false, iStride, BUFFER_OFFSET(iOffset));
}

void CRenderingContext::SetTexCoordBuffer(float* pflBuffer, size_t iStride, size_t iChannel)
{
	TAssert(iChannel >= 0 && iChannel < MAX_TEXTURE_CHANNELS);

	if (m_pShader->m_aiTexCoordAttributes[iChannel] == ~0)
		return;

	glEnableVertexAttribArray(m_pShader->m_aiTexCoordAttributes[iChannel]);
	glVertexAttribPointer(m_pShader->m_aiTexCoordAttributes[iChannel], 2, GL_FLOAT, false, iStride, pflBuffer);
}

void CRenderingContext::SetTexCoordBuffer(size_t iOffset, size_t iStride, size_t iChannel)
{
	TAssert(iChannel >= 0 && iChannel < MAX_TEXTURE_CHANNELS);

	if (m_pShader->m_aiTexCoordAttributes[iChannel] == ~0)
		return;

	TAssert(iOffset%4 == 0);	// Should be multiples of four because it's offsets in bytes and we're always working with floats or doubles
	glEnableVertexAttribArray(m_pShader->m_aiTexCoordAttributes[iChannel]);
	glVertexAttribPointer(m_pShader->m_aiTexCoordAttributes[iChannel], 2, GL_FLOAT, false, iStride, BUFFER_OFFSET(iOffset));
}

void CRenderingContext::SetCustomIntBuffer(const char* pszName, size_t iSize, size_t iOffset, size_t iStride)
{
	int iAttribute = glGetAttribLocation(m_iProgram, pszName);

	TAssert(iAttribute != ~0);
	if (iAttribute == ~0)
		return;

	TAssert(iOffset%4 == 0);	// Should be multiples of four because it's offsets in bytes and we're always working with floats or doubles
	glEnableVertexAttribArray(iAttribute);
	glVertexAttribIPointer(iAttribute, iSize, GL_INT, iStride, BUFFER_OFFSET(iOffset));
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
		glLineWidth(1);
		glDrawArrays(GL_LINES, 0, iVertices);
	}
	else
		glDrawArrays(GL_TRIANGLES, 0, iVertices);

	glDisableVertexAttribArray(m_pShader->m_iPositionAttribute);
	for (size_t i = 0; i < MAX_TEXTURE_CHANNELS; i++)
	{
		if (m_pShader->m_aiTexCoordAttributes[i] != ~0)
			glDisableVertexAttribArray(m_pShader->m_aiTexCoordAttributes[i]);
	}
	if (m_pShader->m_iNormalAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iNormalAttribute);
	if (m_pShader->m_iTangentAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iTangentAttribute);
	if (m_pShader->m_iBitangentAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iBitangentAttribute);
	if (m_pShader->m_iColorAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iColorAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
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

	glDrawElements(GL_TRIANGLES, iTriangles*3, GL_UNSIGNED_INT, piIndices);

	glDisableVertexAttribArray(m_pShader->m_iPositionAttribute);
	for (size_t i = 0; i < MAX_TEXTURE_CHANNELS; i++)
	{
		if (m_pShader->m_aiTexCoordAttributes[i] != ~0)
			glDisableVertexAttribArray(m_pShader->m_aiTexCoordAttributes[i]);
	}
	if (m_pShader->m_iNormalAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iNormalAttribute);
	if (m_pShader->m_iTangentAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iTangentAttribute);
	if (m_pShader->m_iBitangentAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iBitangentAttribute);
	if (m_pShader->m_iColorAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iColorAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
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

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iBuffer);

	bool bWireframe = false;
	glDrawElements(bWireframe?GL_LINES:GL_TRIANGLES, iVertices, GL_UNSIGNED_INT, nullptr);

	glDisableVertexAttribArray(m_pShader->m_iPositionAttribute);
	for (size_t i = 0; i < MAX_TEXTURE_CHANNELS; i++)
	{
		if (m_pShader->m_aiTexCoordAttributes[i] != ~0)
			glDisableVertexAttribArray(m_pShader->m_aiTexCoordAttributes[i]);
	}
	if (m_pShader->m_iNormalAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iNormalAttribute);
	if (m_pShader->m_iTangentAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iTangentAttribute);
	if (m_pShader->m_iBitangentAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iBitangentAttribute);
	if (m_pShader->m_iColorAttribute != ~0)
		glDisableVertexAttribArray(m_pShader->m_iColorAttribute);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void CRenderingContext::ReadPixels(size_t x, size_t y, size_t w, size_t h, Vector4D* pvecPixels)
{
	glReadPixels(x, y, w, h, GL_RGBA, GL_FLOAT, pvecPixels);
}

void CRenderingContext::Finish()
{
	glFinish();
}

CRenderingContext::CRenderContext::CRenderContext()
{
	m_bProjectionUpdated = false;
	m_bViewUpdated = false;
	m_bTransformUpdated = false;
}
