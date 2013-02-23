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

#include "shaders.h"

#include <fstream>

#include <GL3/gl3w.h>
#include <time.h>

#include <common.h>
#include <common_platform.h>
#include <strutils.h>

#include <renderer/application.h>
#include <datamanager/data.h>
#include <datamanager/dataserializer.h>

using namespace std;

CShaderLibrary* CShaderLibrary::s_pShaderLibrary = NULL;
static CShaderLibrary g_ShaderLibrary = CShaderLibrary();

CShaderLibrary::CShaderLibrary()
{
	s_pShaderLibrary = this;

	m_bCompiled = false;
	m_iSamples = -1;

	FILE* f = tfopen("shaders/functions.si", "r");

	TAssert(f);
	if (f)
	{
		string sLine;
		while (fgetts(sLine, f))
			m_sFunctions += sLine;

		fclose(f);
	}

	f = tfopen("shaders/header.si", "r");

	TAssert(f);
	if (f)
	{
		string sLine;
		while (fgetts(sLine, f))
			m_sHeader += sLine;

		fclose(f);
	}
}

CShaderLibrary::~CShaderLibrary()
{
	for (size_t i = 0; i < m_aShaders.size(); i++)
	{
		CShader* pShader = &m_aShaders[i];
		glDetachShader((GLuint)pShader->m_iProgram, (GLuint)pShader->m_iVShader);
		glDetachShader((GLuint)pShader->m_iProgram, (GLuint)pShader->m_iFShader);
		glDeleteProgram((GLuint)pShader->m_iProgram);
		glDeleteShader((GLuint)pShader->m_iVShader);
		glDeleteShader((GLuint)pShader->m_iFShader);
	}

	s_pShaderLibrary = NULL;
}

void CShaderLibrary::AddShader(const string& sFile)
{
	TAssert(!Get()->m_bCompiled);
	if (Get()->m_bCompiled)
		return;

	std::basic_ifstream<char> f(sFile.c_str());

	if (!f.is_open())
	{
		TAssert(false);
		// Couldn't open shader file
		return;
	}

	std::shared_ptr<CData> pData(new CData());
	CDataSerializer::Read(f, pData.get());

	CData* pName = pData->FindChild("Name");
	CData* pVertex = pData->FindChild("Vertex");
	CData* pFragment = pData->FindChild("Fragment");

	TAssert(pName);
	if (!pName)
		return;

	TAssert(pVertex);
	if (!pVertex)
		return;

	TAssert(pFragment);
	if (!pFragment)
		return;

	Get()->m_aShaders.push_back(CShader(pName->GetValueString(), pVertex->GetValueString(), pFragment->GetValueString()));
	Get()->m_aShaderNames[pName->GetValueString()] = Get()->m_aShaders.size()-1;

	auto& oShader = Get()->m_aShaders.back();

	for (size_t i = 0; i < pData->GetNumChildren(); i++)
	{
		CData* pChild = pData->GetChild(i);
		if (pChild->GetKey() == "Parameter")
		{
			auto& oParameter = oShader.m_aParameters[pChild->GetValueString()];
			oParameter.m_sName = pChild->GetValueString();

			for (size_t j = 0; j < pChild->GetNumChildren(); j++)
			{
				CData* pUniform = pChild->GetChild(j);
				if (pUniform->GetKey() == "Uniform")
				{
					oParameter.m_aActions.push_back(CShader::CParameter::CUniform());
					auto& oUniform = oParameter.m_aActions.back();
					oUniform.m_sName = pUniform->GetValueString();
					oUniform.m_bTexture = false;
					CData* pValue = pUniform->FindChild("Value");
					CData* pTexture = pUniform->FindChild("Texture");
					TAssert(!(pValue && pTexture));
					TAssert(pValue || pTexture);

					if (pValue)
						oUniform.m_sValue = pValue->GetValueString();
					else if (pTexture)
					{
						oUniform.m_sValue = pTexture->GetValueString();
						oShader.m_asTextures.push_back(pUniform->GetValueString());
						oUniform.m_bTexture = true;
					}
				}
				else if (pUniform->GetKey() == "Blend")
				{
					string& sBlend = oParameter.m_sBlend;
					sBlend = pUniform->GetValueString();
				}
			}
		}
		else if (pChild->GetKey() == "Defaults")
		{
			for (size_t j = 0; j < pChild->GetNumChildren(); j++)
			{
				CData* pUniform = pChild->GetChild(j);
				auto& oDefault = oShader.m_aDefaults[pUniform->GetKey()];
				oDefault.m_sName = pUniform->GetKey();
				oDefault.m_sValue = pUniform->GetValueString();
			}
		}
	}
}

void CShaderLibrary::CompileShaders(int iSamples)
{
	if (iSamples != -1)
		Get()->m_iSamples = iSamples;

	TAssert(Get()->m_iSamples != -1);

	Get()->ClearLog();

//	if (GameServer()->GetWorkListener())
//		GameServer()->GetWorkListener()->SetAction("Compiling shaders", Get()->m_aShaders.size());

	if (Get()->m_bCompiled)
	{
		// If this is a recompile just blow through them.
		for (size_t i = 0; i < Get()->m_aShaders.size(); i++)
			Get()->m_aShaders[i].Compile();
	}
	else
	{
		bool bShadersCompiled = true;
		for (size_t i = 0; i < Get()->m_aShaders.size(); i++)
		{
			bShadersCompiled &= Get()->m_aShaders[i].Compile();

			if (!bShadersCompiled)
				break;

	//		if (GameServer()->GetWorkListener())
	//			GameServer()->GetWorkListener()->WorkProgress(i);
		}

		if (bShadersCompiled)
			Get()->m_bCompiled = true;
	}
}

void CShaderLibrary::DestroyShaders()
{
	for (size_t i = 0; i < Get()->m_aShaders.size(); i++)
		Get()->m_aShaders[i].Destroy();

	Get()->m_bCompiled = false;
}

void CShaderLibrary::ClearLog()
{
	m_bLogNeedsClearing = true;
}

void CShaderLibrary::WriteLog(const string& sFile, const char* pszLog, const char* pszShaderText)
{
	if (!pszLog || strlen(pszLog) == 0)
		return;

	string sLogFile = GetAppDataDirectory(Application()->AppDirectory(), "shaders.txt");

	if (m_bLogNeedsClearing)
	{
		// Only clear it if we're actually going to write to it so we don't create the file.
		FILE* fp = tfopen(sLogFile, "w");
		fclose(fp);
		m_bLogNeedsClearing = false;
	}

	char szText[100];
	strncpy(szText, pszShaderText, 99);
	szText[99] = '\0';

	FILE* fp = tfopen(sLogFile, "a");
	fprintf(fp, ("Shader compile output for file: " + sFile + " timestamp: %d\n").c_str(), (int)time(NULL));
	fprintf(fp, "%s\n\n", pszLog);
	fprintf(fp, "%s...\n\n", szText);
	fclose(fp);
}

CShader* CShaderLibrary::GetShader(const string& sName)
{
	map<string, size_t>::const_iterator i = Get()->m_aShaderNames.find(sName);
	if (i == Get()->m_aShaderNames.end())
		return NULL;

	return &Get()->m_aShaders[i->second];
}

CShader* CShaderLibrary::GetShader(size_t i)
{
	if (i >= Get()->m_aShaders.size())
		return NULL;

	return &Get()->m_aShaders[i];
}

size_t CShaderLibrary::GetProgram(const string& sName)
{
	TAssert(Get());
	if (!Get())
		return 0;

	TAssert(Get()->GetShader(sName));
	if (!Get()->GetShader(sName))
		return 0;

	return Get()->GetShader(sName)->m_iProgram;
}

CShader::CShader(const string& sName, const string& sVertexFile, const string& sFragmentFile)
{
	m_sName = sName;
	m_sVertexFile = sVertexFile;
	m_sFragmentFile = sFragmentFile;
	m_iVShader = 0;
	m_iFShader = 0;
	m_iProgram = 0;
}

bool CShader::Compile()
{
	string sShaderHeader = CShaderLibrary::GetShaderHeader();

	if (CShaderLibrary::Get()->m_iSamples)
		sShaderHeader += "#define USE_MULTISAMPLE_TEXTURES 1\n";

	sShaderHeader += CShaderLibrary::GetShaderFunctions();

	FILE* f = tfopen("shaders/" + m_sVertexFile + ".vs", "r");

	TAssert(f);
	if (!f)
		return false;

	string sVertexShader = sShaderHeader;
	sVertexShader += "uniform mat4x4 mProjection;\n";
	sVertexShader += "uniform mat4x4 mView;\n";
	sVertexShader += "uniform mat4x4 mGlobal;\n";

	string sLine;
	while (fgetts(sLine, f))
		sVertexShader += sLine;

	fclose(f);

	f = tfopen("shaders/" + m_sFragmentFile + ".fs", "r");

	TAssert(f);
	if (!f)
		return false;

	string sFragmentShader = sShaderHeader;
	sFragmentShader += "out vec4 vecOutputColor;\n";

	while (fgetts(sLine, f))
		sFragmentShader += sLine;

	fclose(f);

	size_t iVShader = glCreateShader(GL_VERTEX_SHADER);
	const char* pszStr = sVertexShader.c_str();
	glShaderSource((GLuint)iVShader, 1, &pszStr, NULL);
	glCompileShader((GLuint)iVShader);

	int iVertexCompiled;
	glGetShaderiv((GLuint)iVShader, GL_COMPILE_STATUS, &iVertexCompiled);

	if (iVertexCompiled != GL_TRUE || Application()->HasCommandLineSwitch("--debug-gl"))
	{
		int iLogLength = 0;
		char szLog[1024];
		glGetShaderInfoLog((GLuint)iVShader, 1024, &iLogLength, szLog);
		CShaderLibrary::Get()->WriteLog(m_sVertexFile + ".vs", szLog, pszStr);
	}

	size_t iFShader = glCreateShader(GL_FRAGMENT_SHADER);
	pszStr = sFragmentShader.c_str();
	glShaderSource((GLuint)iFShader, 1, &pszStr, NULL);
	glCompileShader((GLuint)iFShader);

	int iFragmentCompiled;
	glGetShaderiv((GLuint)iFShader, GL_COMPILE_STATUS, &iFragmentCompiled);

	if (iFragmentCompiled != GL_TRUE || Application()->HasCommandLineSwitch("--debug-gl"))
	{
		int iLogLength = 0;
		char szLog[1024];
		glGetShaderInfoLog((GLuint)iFShader, 1024, &iLogLength, szLog);
		CShaderLibrary::Get()->WriteLog(m_sFragmentFile + ".fs", szLog, pszStr);
	}

	size_t iProgram = glCreateProgram();

	glBindAttribLocation(iProgram, 0, "vecPosition");		// Force position at location 0. ATI cards won't work without this.

	glAttachShader((GLuint)iProgram, (GLuint)iVShader);
	glAttachShader((GLuint)iProgram, (GLuint)iFShader);
	glLinkProgram((GLuint)iProgram);

	int iProgramLinked;
	glGetProgramiv((GLuint)iProgram, GL_LINK_STATUS, &iProgramLinked);

	if (iProgramLinked != GL_TRUE || Application()->HasCommandLineSwitch("--debug-gl"))
	{
		int iLogLength = 0;
		char szLog[1024];
		glGetProgramInfoLog((GLuint)iProgram, 1024, &iLogLength, szLog);
		CShaderLibrary::Get()->WriteLog("link", szLog, "link");
	}

	if (iVertexCompiled != GL_TRUE || iFragmentCompiled != GL_TRUE || iProgramLinked != GL_TRUE)
	{
		TAssert(false);
		// Shader compilation failed. Check shaders.txt

		Destroy();

		return false;
	}

	m_iProgram = iProgram;
	m_iVShader = iVShader;
	m_iFShader = iFShader;

	m_iPositionAttribute = glGetAttribLocation(m_iProgram, "vecPosition");
	m_iNormalAttribute = glGetAttribLocation(m_iProgram, "vecNormal");
	m_iTangentAttribute = glGetAttribLocation(m_iProgram, "vecTangent");
	m_iBitangentAttribute = glGetAttribLocation(m_iProgram, "vecBitangent");
	for (size_t i = 0; i < MAX_TEXTURE_CHANNELS; i++)
		m_aiTexCoordAttributes[i] = glGetAttribLocation(m_iProgram, sprintf("vecTexCoord%d", i).c_str());
	m_iColorAttribute = glGetAttribLocation(m_iProgram, "vecVertexColor");

	glBindFragDataLocation(m_iProgram, 0, "vecOutputColor");

	TAssert(m_iPositionAttribute != ~0);

	int iNumUniforms;
	glGetProgramiv(m_iProgram, GL_ACTIVE_UNIFORMS, &iNumUniforms);

	char szUniformName[256];
	GLsizei iLength;
	GLint iSize;
	GLenum iType;
	for (int i = 0; i < iNumUniforms; i++)
	{
		glGetActiveUniform(m_iProgram, i, sizeof(szUniformName), &iLength, &iSize, &iType, szUniformName);

		string sUniformName = szUniformName;
		if (sUniformName == "mProjection")
			continue;
		if (sUniformName == "mView")
			continue;
		if (sUniformName == "mGlobal")
			continue;

		CShader::CUniform& oUniform = m_asUniforms[sUniformName];
		oUniform.m_pDefault = nullptr;
		switch (iType)
		{
		case GL_FLOAT: oUniform.m_sUniformType = "float"; break;
		case GL_FLOAT_VEC2: oUniform.m_sUniformType = "vec2"; break;
		case GL_FLOAT_VEC3: oUniform.m_sUniformType = "vec3"; break;
		case GL_FLOAT_VEC4: oUniform.m_sUniformType = "vec4"; break;
		case GL_INT: oUniform.m_sUniformType = "int"; break;
		case GL_BOOL: oUniform.m_sUniformType = "bool"; break;
		case GL_FLOAT_MAT4: oUniform.m_sUniformType = "mat4"; break;
		case GL_SAMPLER_2D: oUniform.m_sUniformType = "sampler2D"; break;
		default: TUnimplemented();
		}
	}

	for (auto it = m_aParameters.begin(); it != m_aParameters.end(); it++)
	{
		for (size_t j = 0; j < it->second.m_aActions.size(); j++)
		{
			auto it2 = m_asUniforms.find(it->second.m_aActions[j].m_sName);
			TAssert(it2 != m_asUniforms.end());
			if (it2 == m_asUniforms.end())
			{
				TAssert(false);
				// A shader specifies a uniform that is not in the linked program.
				continue;
			}

			CShader::CUniform& oUniform = it2->second;

			// This is almost cheating
			CData d;
			d.SetValue(it->second.m_aActions[j].m_sValue);

			if (oUniform.m_sUniformType == "float")
				it->second.m_aActions[j].m_flValue = d.GetValueFloat();
			else if (oUniform.m_sUniformType == "vec2")
				it->second.m_aActions[j].m_vec2Value = d.GetValueVector2D();
			else if (oUniform.m_sUniformType == "vec3")
				it->second.m_aActions[j].m_vecValue = d.GetValueVector();
			else if (oUniform.m_sUniformType == "vec4")
				it->second.m_aActions[j].m_vec4Value = d.GetValueVector4D();
			else if (oUniform.m_sUniformType == "int")
				it->second.m_aActions[j].m_iValue = d.GetValueInt();
			else if (oUniform.m_sUniformType == "bool")
				it->second.m_aActions[j].m_bValue = d.GetValueBool();
			else if (oUniform.m_sUniformType == "mat4")
			{
				TUnimplemented();
			}
			else if (oUniform.m_sUniformType == "sampler2D")
			{
				// No op.
			}
			else
				TUnimplemented();
		}
	}

	for (auto it = m_aDefaults.begin(); it != m_aDefaults.end(); it++)
	{
		auto it2 = m_asUniforms.find(it->first);
		TAssert(it2 != m_asUniforms.end());
		if (it2 == m_asUniforms.end())
		{
			TAssert(false);
			// A shader specifies a default for a uniform that is not in the linked program.
			continue;
		}

		CShader::CUniform& oUniform = it2->second;
		oUniform.m_pDefault = &it->second;

		// Again with the cheating.
		CData d;
		d.SetValue(it->second.m_sValue);

		if (oUniform.m_sUniformType == "float")
			it->second.m_flValue = d.GetValueFloat();
		else if (oUniform.m_sUniformType == "vec2")
			it->second.m_vec2Value = d.GetValueVector2D();
		else if (oUniform.m_sUniformType == "vec3")
			it->second.m_vecValue = d.GetValueVector();
		else if (oUniform.m_sUniformType == "vec4")
			it->second.m_vec4Value = d.GetValueVector4D();
		else if (oUniform.m_sUniformType == "int")
			it->second.m_iValue = d.GetValueInt();
		else if (oUniform.m_sUniformType == "bool")
			it->second.m_bValue = d.GetValueBool();
		else if (oUniform.m_sUniformType == "mat4")
		{
			TUnimplemented(); 
		}
		else if (oUniform.m_sUniformType == "sampler2D")
		{
			TUnimplemented(); // Can't set a default texture... yet.
		}
		else
			TUnimplemented();
	}

	return true;
}

void CShader::Destroy()
{
	glDetachShader((GLuint)m_iProgram, (GLuint)m_iVShader);
	glDetachShader((GLuint)m_iProgram, (GLuint)m_iFShader);
	glDeleteShader((GLuint)m_iVShader);
	glDeleteShader((GLuint)m_iFShader);
	glDeleteProgram((GLuint)m_iProgram);
}

string CShader::FindType(const string& sName) const
{
	auto it = m_aParameters.find(sName);
	TAssert(it != m_aParameters.end());
	if (it == m_aParameters.end())
		return "unknown";

	const CShader::CParameter* pShaderPar = &it->second;

	string sType;
	for (size_t j = 0; j < pShaderPar->m_aActions.size(); j++)
		if (pShaderPar->m_aActions[j].m_sValue == "[value]")
			return m_asUniforms.find(pShaderPar->m_aActions[j].m_sName)->second.m_sUniformType;

	return "unknown";
}

size_t CShader::FindTextureByUniform(const string& sUniform) const
{
	for (size_t i = 0; i < m_asTextures.size(); i++)
	{
		if (m_asTextures[i] == sUniform)
			return i;
	}

	return ~0;
}
