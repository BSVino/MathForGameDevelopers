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

#include <functional>
#include <algorithm>
#include <cctype>
#include <iostream>
#include <sstream>

#include <string>
#include <stdio.h>
#include <stdarg.h>

using std::find_if;
using std::not1;
using std::ptr_fun;

template <class F, class T>
inline std::basic_string<T> convertstring(const std::basic_string<F>& s);

// It's inline so I don't have to make a strutils.cpp :P
inline void strtok(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiters = " \r\n\t")
{
    // Skip delimiters at beginning.
    std::string::size_type lastPos = str.find_first_not_of(delimiters, 0);
    // Find first "non-delimiter".
    std::string::size_type pos     = str.find_first_of(delimiters, lastPos);

    while (std::string::npos != pos || std::string::npos != lastPos)
    {
        // Found a token, add it to the vector.
        tokens.push_back(str.substr(lastPos, pos - lastPos));
        // Skip delimiters.  Note the "not_of"
        lastPos = str.find_first_not_of(delimiters, pos);
        // Find next "non-delimiter"
        pos = str.find_first_of(delimiters, lastPos);
    }
}

// explode is slightly different in that repeated delineators return multiple tokens.
// ie "a|b||c" returns { "a", "b", "", "c" } whereas strtok will cut out the blank result.
// Basically it works like PHP's explode.
inline void explode(const std::string& str, std::vector<std::string>& tokens, const std::string& delimiter = " ")
{
    std::string::size_type lastPos = str.find_first_of(delimiter, 0);
    std::string::size_type pos = 0;

    while (true)
    {
        tokens.push_back(str.substr(pos, lastPos - pos));

		if (lastPos == std::string::npos)
			break;

		pos = lastPos+1;
        lastPos = str.find_first_of(delimiter, pos);
    }
}

inline std::string implode(const std::string& sGlue, std::vector<std::string>& asStrings)
{
	if (!asStrings.size())
		return 0;

	std::string sResult = asStrings[0];

	for (size_t i = 1; i < asStrings.size(); i++)
		sResult += sGlue + asStrings[i];

	return sResult;
}

inline std::string ltrim(std::string s)
{
	s.erase(s.begin(), std::find_if(s.begin(), s.end(), not1(ptr_fun<int, int>(isspace))));
	return s;
}

inline std::string rtrim(std::string s)
{
	s.erase(std::find_if(s.rbegin(), s.rend(), not1(ptr_fun<int, int>(isspace))).base(), s.end());
	return s;
}

inline std::string trim(std::string s)
{
	return ltrim(rtrim(s));
}

inline void writestring(std::ostream& o, const std::string& s)
{
	size_t iStringSize = s.length();
	o.write((char*)&iStringSize, sizeof(iStringSize));
	o.write((char*)s.c_str(), s.size()*sizeof(std::string::value_type));
}

inline std::string readstring(std::istream& i)
{
	size_t iStringSize;
	i.read((char*)&iStringSize, sizeof(iStringSize));

	std::string s;
	std::string::value_type c[2];
	c[1] = '\0';
	for (size_t j = 0; j < iStringSize; j++)
	{
		i.read((char*)&c[0], sizeof(std::string::value_type));
		s.append(c);
	}

	return s;
}

template <class F, class T>
inline std::basic_string<T> convertstring(const std::basic_string<F>& s)
{
	if (sizeof(F) == sizeof(T))
		return (T*)s.c_str();

	std::basic_string<T> t;
	size_t iSize = s.size();
	t.resize(iSize);

	for (size_t i = 0; i < iSize; i++)
		t[i] = (T)s[i];

	return t;
}

inline std::wstring convert_to_wstring(const std::string& s)
{
	std::wstring t;
	size_t iSize = s.size();
	t.resize(iSize);

	for (size_t i = 0; i < iSize; i++)
		t[i] = (wchar_t)s[i];

	return t;
}

inline std::string convert_from_wstring(const std::wstring& s)
{
	std::string t;
	size_t iSize = s.size();
	t.resize(iSize);

	for (size_t i = 0; i < iSize; i++)
		t[i] = (char)s[i];

	return t;
}

inline std::string sprintf(std::string s, ...)
{
	va_list arguments;
	va_start(arguments, s);

	std::string p;

#ifdef WITH_EASTL
	p.sprintf_va_list(s.c_str(), arguments);
#else

#ifdef _MSC_VER
#define VSNPRINTF8 _vsnprintf
#else
#define VSNPRINTF8 vsnprintf
#endif
	std::string q = " ";
	int iCharacters = VSNPRINTF8(&q[0], 0, s.c_str(), arguments);

	if(iCharacters >= (int)q.length())
	{
		q.resize(iCharacters*2);

		va_start(arguments, s);
		iCharacters = VSNPRINTF8(&q[0], q.size()-1, s.c_str(), arguments);
	}
	else if(iCharacters < 0)
	{
		while (iCharacters < 0)
		{
			q.resize(q.size()*2);

			va_start(arguments, s);
			iCharacters = VSNPRINTF8(&q[0], q.size()-1, s.c_str(), arguments);
		}
	}

	p = &q[0];
#endif

	va_end(arguments);

	return p;
}

inline int stoi(const std::string& s)
{
	std::istringstream i(s.c_str());
	int x;
	if (!(i >> x))
		return 0;

	return x;
}

inline double stof(const std::string& s)
{
	std::istringstream i(s.c_str());
	double x;
	if (!(i >> x))
		return 0;

	return x;
}

template <class C>
inline C* strtok(C* s, const C* delim, C** last)
{
	C* spanp;
	int c, sc;
	C* tok;


	if (s == NULL && (s = *last) == NULL)
		return (NULL);

	/*
	 * Skip (span) leading delimiters (s += strspn(s, delim), sort of).
	 */
cont:
	c = *s++;
	for (spanp = (C*)delim; (sc = *spanp++) != 0;) {
		if (c == sc)
			goto cont;
	}

	if (c == 0) {		/* no non-delimiter characters */
		*last = NULL;
		return (NULL);
	}
	tok = s - 1;

	/*
	 * Scan token (scan for delimiters: s += strcspn(s, delim), sort of).
	 * Note that delim must have one NUL; we stop if we see that, too.
	 */
	for (;;) {
		c = *s++;
		spanp = (C *)delim;
		do {
			if ((sc = *spanp++) == c) {
				if (c == 0)
					s = NULL;
				else
					s[-1] = 0;
				*last = s;
				return (tok);
			}
		} while (sc != 0);
	}
	/* NOTREACHED */
}

template <class C>
inline C* strdup(const C* s)
{
	size_t len = (1+std::char_traits<C>::length(s)) * sizeof(C);
	C* p = (C*)malloc(len);

  	return p ? (C*)memcpy(p, s, len) : NULL;
}

inline std::string pretty_float(float f, int iMaxLength=8)
{
	std::string s = sprintf("%." + sprintf("%d", iMaxLength) + "f", f);

	size_t i = s.length();
	while (s[i-1] == '0')
		i--;

	if (s[i-1] == '.')
		i--;

	return s.substr(0, i);
}

#include <string>

inline FILE* tfopen(const std::string& sFile, const std::string& sMode)
{
	std::string sBinaryMode = sMode;
	bool bHasB = false;
	for (size_t i = 0; i < sBinaryMode.length(); i++)
	{
		if (sMode[i] == 'b')
		{
			bHasB = true;
			break;
		}
	}

	// Open all files in binary mode to preserve unicodeness.
	if (!bHasB)
		sBinaryMode = sMode + "b";

	return fopen(sFile.c_str(), sBinaryMode.c_str());
}

inline bool fgetts(std::string& str, FILE* fp)
{
	static char szLine[1024];
	char* r = fgets(szLine, 1023, fp);

	if (!r)
		return false;

	str = szLine;
	return !!r;
}

inline char* tstrncpy(char* d, size_t d_size, const char* s, size_t n)
{
#ifdef _WIN32
	return std::char_traits<char>::_Copy_s(d, d_size, s, n);
#else
	if (d_size < n)
		n = d_size;

	return std::char_traits<char>::copy(d, s, n);
#endif
}

inline size_t tstrlen(const char* s)
{
	return std::char_traits<char>::length(s);
}

inline int tstrncmp(const char* s1, const char* s2, size_t n)
{
	return std::char_traits<char>::compare(s1, s2, n);
}

inline bool str_endswith(const std::string& sString, const std::string& sEnding)
{
	if (sString.length() < sEnding.length())
		return false;

	return (sString.substr(sString.length() - sEnding.length()) == sEnding);
}

