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

#include <map>
#include <vector>
#include <string>
#include <string.h>

#include <vector.h>
#include <vector2d.h>
#include <vector4d.h>

#define MAX_TEXTURE_CHANNELS 2

class CShader
{
public:
							CShader(const std::string& sName, const std::string& sVertexFile, const std::string& sFragmentFile);

public:
	bool					Compile();
	void					Destroy();

	std::string             FindType(const std::string& sName) const;

	size_t					FindTextureByUniform(const std::string& sUniform) const;

public:
	std::string					m_sName;
	std::string					m_sVertexFile;
	std::string					m_sFragmentFile;
	size_t					m_iVShader;
	size_t					m_iFShader;
	size_t					m_iProgram;

	size_t					m_iPositionAttribute;
	size_t					m_iNormalAttribute;
	size_t					m_iTangentAttribute;
	size_t					m_iBitangentAttribute;
	size_t					m_aiTexCoordAttributes[MAX_TEXTURE_CHANNELS];
	size_t					m_iColorAttribute;

	class CParameter
	{
	public:
		std::string				m_sName;

		class CUniform
		{
		public:
			std::string			m_sName;
			union
			{
				float		m_flValue;
				int			m_iValue;
				bool		m_bValue;
			};
			Vector2D		m_vec2Value;
			Vector			m_vecValue;
			Vector4D		m_vec4Value;
			std::string			m_sValue;
			bool			m_bTexture;
		};

		std::vector<CUniform>	m_aActions;

		std::string				m_sBlend;
	};

	class CUniform
	{
	public:
		std::string					m_sUniformType;
		CParameter::CUniform*	m_pDefault;
	};

	std::map<std::string, CParameter>			m_aParameters;	// What the shader.txt has.
	std::map<std::string, CUniform>				m_asUniforms;	// What the hardware has. Values are types.
	std::map<std::string, CParameter::CUniform>	m_aDefaults;	// Defaults for each uniform as specified by shader .txt (not GLSL)
	std::vector<std::string>					m_asTextures;	// List of textures for purposes of assigning to channels and whatnot.
};

class CShaderLibrary
{
	friend class CShader;

public:
							CShaderLibrary();
							~CShaderLibrary();

public:
	static size_t			GetNumShaders() { return Get()->m_aShaders.size(); };

	static CShader*			GetShader(const std::string& sName);
	static CShader*			GetShader(size_t i);

	static void				AddShader(const std::string& sFile);

	static size_t			GetProgram(const std::string& sName);

	static void				CompileShaders(int iSamples = -1);
	static void				DestroyShaders();

	static bool				IsCompiled() { return Get()->m_bCompiled; };

	static std::string			GetShaderHeader() { return Get()->m_sHeader; }
	static std::string			GetShaderFunctions() { return Get()->m_sFunctions; }

	static CShaderLibrary*	Get() { return s_pShaderLibrary; };

protected:
	void					ClearLog();
	void					WriteLog(const std::string& sFile, const char* pszLog, const char* pszShaderText);

protected:
	std::map<std::string, size_t>	m_aShaderNames;
	std::vector<CShader>		m_aShaders;
	bool					m_bCompiled;

	bool					m_bLogNeedsClearing;

	int						m_iSamples;

	std::string					m_sHeader;
	std::string					m_sFunctions;

private:
	static CShaderLibrary*	s_pShaderLibrary;
};
