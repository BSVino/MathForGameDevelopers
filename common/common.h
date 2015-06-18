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

#ifndef COMMON_H
#define COMMON_H

#define DECLARE_CLASS( className, baseClassName ) \
	typedef baseClassName BaseClass; \
	typedef className ThisClass; \

#ifdef __GNUC__

#include <csignal>

#define TDebugBreak() \
	::raise(SIGTRAP); \

#else

#define TDebugBreak() \
	__debugbreak(); \

#endif

// tinker_platform.h
extern void DebugPrint(const char* pszText);

#ifdef _DEBUG

#define TAssert(x) \
{ \
	if (!(x)) \
	{ \
		DebugPrint("Assert failed: " #x "\n"); \
		TDebugBreak(); \
	} \
} \

#else

#if defined(_T_RELEASE_ASSERTS)
#define TAssert(x) \
{ \
	if (!(x)) \
		DebugPrint("Assert failed: " #x "\n"); \
} \

#else

#define TAssert(x) {}

#endif

#endif

// If you hit this, the code is either incomplete or untested.
#define TUnimplemented() TAssert(false)

#ifdef __GNUC__
// Clang or GCC
#ifdef __clang__
#else
#if __GNUC__ < 4 || __GNUC_MINOR__ < 6

const                        // this is a const object...
class {
public:
  template<class T>          // convertible to any type
    operator T*() const      // of null non-member
    { return 0; }            // pointer...
  template<class C, class T> // or any type of null
    operator T C::*() const  // member pointer...
    { return 0; }
private:
  void operator&() const;    // whose address can't be taken
} nullptr = {};              // and whose name is nullptr

#endif
#endif

// For std::shared_ptr
#include <memory>

#endif

#endif
