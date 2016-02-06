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

#ifndef TINKER_RENDERINGCONTEXT_H
#define TINKER_RENDERINGCONTEXT_H

#include <vector>
#include <string>

#include <matrix.h>
#include <vector2d.h>
#include <color.h>

#include "render_common.h"

#define PROGRAM_LEN 32

class CRenderingContext
{
protected:
	class CRenderContext
	{
	public:
		CRenderContext();

	public:
		Matrix4x4			m_mProjection;
		Matrix4x4			m_mView;
		Matrix4x4			m_mTransformations;

		bool                m_bProjectionUpdated;
		bool                m_bViewUpdated;
		bool                m_bTransformUpdated;

		const class CFrameBuffer*	m_pFrameBuffer;
		char				m_szProgram[PROGRAM_LEN];	// Not a string for perf reasons
		class CShader*		m_pShader;

		int                 m_iViewportX;
		int                 m_iViewportY;
		int                 m_iViewportWidth;
		int                 m_iViewportHeight;

		blendtype_t			m_eBlend;
		float				m_flAlpha;
		bool				m_bDepthMask;
		bool				m_bDepthTest;
		depth_function_t	m_eDepthFunction;
		bool				m_bCull;
		bool				m_bWinding;
	};

public:
							CRenderingContext(class CRenderer* pRenderer = nullptr, bool bInherit = false);	// Make bInherit true if you want to preserve and not clobber GL settings set previously
	virtual					~CRenderingContext();

public:
	void					SetProjection(const Matrix4x4& m);
	void					SetView(const Matrix4x4& m);

	Matrix4x4				GetProjection() { return GetContext().m_mProjection; }
	Matrix4x4				GetView() { return GetContext().m_mView; }

	void					SetPosition(const Vector& vecPosition);
	void					Transform(const Matrix4x4& m);
	void					Translate(const Vector& vecTranslate);
	void					Rotate(float flAngle, Vector vecAxis);
	void					Scale(float flX, float flY, float flZ);
	void					ResetTransformations();
	void					LoadTransform(const Matrix4x4& m);

	void					SetViewport(int x, int y, int w, int h);
	void					SetBlend(blendtype_t eBlend);
	void					SetAlpha(float flAlpha) { GetContext().m_flAlpha = flAlpha; };
	void					SetDepthMask(bool bDepthMask);
	void					SetDepthTest(bool bDepthTest);
	void					SetDepthFunction(depth_function_t eDepthFunction);
	void					SetBackCulling(bool bCull);
	void					SetWinding(bool bWinding);	// True is default

	float					GetAlpha() { return GetContext().m_flAlpha; };
	blendtype_t				GetBlend() { return GetContext().m_eBlend; };
	depth_function_t		GetDepthFunction() { return GetContext().m_eDepthFunction; };
	bool					GetWinding() { return GetContext().m_bWinding; };

	void					ClearColor(const ::Color& clrClear = ::Color(0.0f, 0.0f, 0.0f, 1.0f));
	void					ClearDepth();

	void					RenderWireBox(const Vector& vecMins, const Vector& vecMaxs);
	void					RenderBox(const Vector& vecMins, const Vector& vecMaxs);

	void					RenderBillboard(size_t iTexture, float flRadius, Vector vecUp, Vector vecRight);

	void					UseProgram(const char* pszProgram);
	void					UseProgram(class CShader* pShader);		// Can save on the program name lookup
	void					SetupMaterial();
	size_t					GetActiveProgram() { return m_iProgram; }
	class CShader*			GetActiveShader() { return m_pShader; }
	void					SetUniform(const char* pszName, int iValue);
	void					SetUniform(const char* pszName, float flValue);
	void					SetUniform(const char* pszName, const Vector& vecValue);
	void					SetUniform(const char* pszName, const Vector4D& vecValue);
	void					SetUniform(const char* pszName, const ::Color& clrColor);
	void					SetUniform(const char* pszName, const Matrix4x4& mValue);
	void					SetUniform(const char* pszName, size_t iSize, const float* aflValues);
	void					BindTexture(size_t iTexture, int iChannel = 0, bool bMultisample = false);
	void					BindBufferTexture(const CFrameBuffer& oBuffer, int iChannel = 0);
	void					SetColor(const ::Color& c);	// Set the mesh's uniform color. Do this before BeginRender*

	// Immediate mode emulation
	void					BeginRenderTris();
	void					BeginRenderTriFan();
	void					BeginRenderTriStrip();
	void					BeginRenderLines();
	void					BeginRenderLineLoop();
	void					BeginRenderLineStrip();
	void					BeginRenderPoints(float flSize=1);
	void					BeginRenderDebugLines();
	void					TexCoord(float s, float t, int iChannel = 0);
	void					TexCoord(const Vector2D& v, int iChannel = 0);
	void					TexCoord(const Vector& v, int iChannel = 0);
	void					Normal(const Vector& v);
	void					Tangent(const Vector& v);
	void					Bitangent(const Vector& v);
	void					Vertex(const Vector& v);
	void					EndRender();
	void                    CreateVBO(size_t& iVBO, size_t& iVBOSize);

	void					BeginRenderVertexArray(size_t iBuffer=0);
	void					SetPositionBuffer(float* pflBuffer, size_t iStrideBytes=0);
	void					SetPositionBuffer(size_t iOffsetBytes, size_t iStrideBytes);
	void					SetNormalsBuffer(float* pflBuffer, size_t iStrideBytes=0);
	void					SetNormalsBuffer(size_t iOffsetBytes, size_t iStrideBytes);
	void					SetTangentsBuffer(float* pflBuffer, size_t iStrideBytes=0);
	void					SetTangentsBuffer(size_t iOffsetBytes, size_t iStrideBytes);
	void					SetBitangentsBuffer(float* pflBuffer, size_t iStrideBytes=0);
	void					SetBitangentsBuffer(size_t iOffsetBytes, size_t iStrideBytes);
	void					SetTexCoordBuffer(float* pflBuffer, size_t iStrideBytes=0, size_t iChannel=0);
	void					SetTexCoordBuffer(size_t iOffsetBytes, size_t iStrideBytes, size_t iChannel=0);
	void					SetCustomIntBuffer(const char* pszName, size_t iSize, size_t iOffset, size_t iStride);
	void					EndRenderVertexArray(size_t iVertices, bool bWireframe = false);
	void					EndRenderVertexArrayTriangles(size_t iTriangles, int* piIndices);
	void					EndRenderVertexArrayIndexed(size_t iBuffer, size_t iVertices);

	// Reads w*h RGBA pixels in float format
	void					ReadPixels(size_t x, size_t y, size_t w, size_t h, Vector4D* pvecPixels);

	void					Finish();	// Flush and block until complete

protected:
	inline CRenderContext&	GetContext() { return s_aContexts.back(); }

public:
	class CRenderer*		m_pRenderer;
	class CShader*			m_pShader;

	size_t					m_iProgram;

	::Color					m_clrRender;

	int						m_iDrawMode;
	bool					m_bTexCoord;

	bool					m_bNormal;
	Vector                  m_vecNormal;

	bool                    m_bTangents;
	Vector                  m_vecTangent;
	Vector                  m_vecBitangent;

	// For building meshes
	Vector m_normal;
	Vector m_tangent;
	Vector m_bitangent;
	Vector2D m_texcoords;// [MAX_TEXTURE_CHANNELS];
	Vector4D m_color;
	static std::vector<float>       s_dynamic_verts;
	int    m_num_verts;

	static std::vector<CRenderContext> s_aContexts;
};

#endif
