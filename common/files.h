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

#include "common_platform.h"
#include "strutils.h"

// Takes a path + filename + extension and removes path and extension to return only the filename.
inline std::string GetFilename(const std::string& sFilename)
{
	int iLastChar = -1;
	int i = -1;

	while (++i < (int)sFilename.length())
		if (sFilename[i] == '\\' || sFilename[i] == '/')
			iLastChar = i;

	std::string sReturn = sFilename.c_str() + iLastChar + 1;

	i = -1;
	while (++i < (int)sReturn.length())
		if (sReturn[i] == '.')
			iLastChar = i;

	if (iLastChar >= 0)
		return sReturn.substr(0, iLastChar);

	return sReturn;
}

// Takes a path + filename + extension and removes path to return only the filename and extension.
inline std::string GetFilenameAndExtension(const std::string& sFilename)
{
	int iLastChar = -1;
	int i = -1;

	while (++i < (int)sFilename.length())
		if (sFilename[i] == '\\' || sFilename[i] == '/')
			iLastChar = i;

	std::string sReturn = sFilename.c_str() + iLastChar + 1;

	return sReturn;
}

inline std::string GetDirectory(const std::string& sFilename)
{
	int iLastSlash = -1;
	int i = -1;
	std::string sResult = sFilename;

	while (++i < (int)sResult.length())
		if (sResult[i] == '\\' || sResult[i] == '/')
			iLastSlash = i;

	if (iLastSlash >= 0)
		return sResult.substr(0, iLastSlash);
	else
		return ".";
}

inline std::string ToForwardSlashes(const std::string& sFilename)
{
	std::string sResult = sFilename;

	for (size_t i = 0; i < sResult.length(); i++)
		if (sResult[i] == '\\')
			sResult[i] = '/';

	return sResult;
}

inline std::string GetRelativePath(const std::string& sPath, const std::string& sFrom)
{
	std::string sAbsolutePath = FindAbsolutePath(sPath);
	std::string sAbsoluteFrom = FindAbsolutePath(sFrom);

	int iIdentical = 0;

	while ((int)sAbsolutePath.length() >= iIdentical && (int)sAbsoluteFrom.length() >= iIdentical && sAbsolutePath[iIdentical] == sAbsoluteFrom[iIdentical])
		iIdentical++;

	std::string sBasePath = sAbsolutePath.substr(iIdentical);
	std::string sBaseFrom = sAbsoluteFrom.substr(iIdentical);

	while (sBasePath[0] == '/' || sBasePath[0] == '\\')
		sBasePath = sBasePath.substr(1);

	if (!sBaseFrom.length())
		return sBasePath;

	size_t iDirectories = 1;
	for (size_t i = 0; i < sBaseFrom.length(); i++)
	{
		if (sBaseFrom[i] == '/' || sBaseFrom[i] == '\\')
			iDirectories++;
	}

	std::string sResult;
	for (size_t i = 0; i < iDirectories; i++)
		sResult += "../";

	return sResult + sBasePath;
}

inline void CreateDirectory(const std::string& sPath)
{
	std::string sSubPath = sPath;

	if (IsDirectory(sPath))
		return;

	std::vector<std::string> asPaths;
	while (true)
	{
		sSubPath = GetDirectory(sSubPath);
		if (sSubPath == ".")
			break;

		asPaths.push_back(sSubPath);
	}

	for (size_t i = 0; i < asPaths.size(); i++)
	{
		if (IsDirectory(asPaths[asPaths.size()-i-1]))
			continue;

		CreateDirectoryNonRecursive(asPaths[asPaths.size()-i-1]);
	}

	CreateDirectoryNonRecursive(sPath);
}

inline bool IsAbsolutePath(const std::string& sPath)
{
	std::string sTrimmedPath = trim(sPath);

	char cFirst = sTrimmedPath[0];
	if ((cFirst > 'A' && cFirst < 'Z') || (cFirst > 'a' && cFirst < 'z'))
	{
		if (sTrimmedPath[1] == ':')
		{
			if (sTrimmedPath[2] == '\\' || sTrimmedPath[2] == '/')
				return true;
		}
	}

	if (cFirst == '/')
		return true;

	if (cFirst == '\\')
		return true;

	if (cFirst == '~')
		return true;

	return false;
}
