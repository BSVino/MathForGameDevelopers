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

#include "dataserializer.h"

#include <strutils.h>

#include "data.h"

using namespace std;

void CDataSerializer::Read(std::basic_istream<char>& sStream, CData* pData)
{
	if (!sStream.good())
		return;

	if (!pData)
		return;

	char szLine[1024];
	string sLine;

	CData* pCurrentData = pData;
	CData* pLastData = NULL;

	while (sStream.getline(szLine, 1024))
	{
		sLine = string(szLine);

		size_t iComment = sLine.find("//");
		if (iComment != string::npos)
			sLine = sLine.substr(0, iComment);

		sLine = trim(sLine);

		if (sLine.length() == 0)
			continue;

		if (sLine[0] == '{')
		{
			pCurrentData = pLastData;
			continue;
		}

		if (sLine[0] == '}')
		{
			pCurrentData = pCurrentData->GetParent();
			continue;
		}

		vector<string> asTokens;
		explode(sLine, asTokens, ":");

		if (asTokens.size() == 1)
			pLastData = pCurrentData->AddChild(trim(sLine));
		else if (asTokens.size() >= 2)
			pLastData = pCurrentData->AddChild(trim(asTokens[0]), trim(sLine.substr(sLine.find(':')+1)));
	}
}

static void SaveData(std::basic_ostream<char>& sStream, CData* pData, size_t iLevel)
{
	string sTabs;
	for (size_t i = 0; i < iLevel; i++)
		sTabs += "\t";

	for (size_t i = 0; i < pData->GetNumChildren(); i++)
	{
		CData* pChild = pData->GetChild(i);

		if (pChild->GetValueString().length())
			sStream << (sTabs + pChild->GetKey() + ": " + pChild->GetValueString() + "\n").c_str();
		else
			sStream << (sTabs + pChild->GetKey() + "\n").c_str();

		if (pChild->GetNumChildren())
		{
			sStream << (sTabs + "{\n").c_str();
			SaveData(sStream, pChild, iLevel+1);
			sStream << (sTabs + "}\n").c_str();
		}
	}
}

void CDataSerializer::Save(std::basic_ostream<char>& sStream, CData* pData)
{
	if (!sStream)
		return;

	if (!pData)
		return;

	SaveData(sStream, pData, 0);
}
