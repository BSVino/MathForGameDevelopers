/*
vtb.h - Vino's Tool Box

This software is in the public domain. Where that dedication is not
recognized, you are granted a perpetual, irrevocable license to copy
and modify this file as you see fit. No warranty is offered or implied.

This header file has a collection of useful utilities that I use often in my
projects. You'll find most of this stuff in the header of any large project.
I'm putting it all in one place for code reuse reasons.

See https://github.com/nothings/stb for the inspiration
See https://www.youtube.com/watch?v=eAhWIO1Ra6M for the motivation

COMPILING AND LINKING
	You must

	#define VTB_IMPLEMENTATION

	in exactly one C or C++ file that includes this header, before the include
	like this:

	#define VTB_IMPLEMENTATION
	#include "vtb.h"

	All other files can be just #include "vtb.h" without the #define

*/

#ifndef VTB_H
#define VTB_H

#ifdef VTB_STATIC
#define VTBDEF static
#else
#define VTBDEF extern
#endif





// VDebugBreak - Forces a trap to break to the debugger

#ifdef __GNUC__

#if defined(__i386__) || defined(__x86_64__)
#define VDebugBreak() __asm__ __volatile__ ( "int $3\n\t" )
#else
#include <csignal>

#define VDebugBreak() \
	::raise(SIGTRAP); \

#endif

#else

#define VDebugBreak() \
	__debugbreak(); \

#endif

#ifdef _MSC_VER
#define VPRAGMA_WARNING_PUSH __pragma(warning(push))
#define VPRAGMA_WARNING_DISABLE(n) __pragma(warning(disable:n))
#define VPRAGMA_WARNING_POP __pragma(warning(pop))
#else
#define VPRAGMA_WARNING_PUSH
#define VPRAGMA_WARNING_DISABLE(n)
#define VPRAGMA_WARNING_POP
#endif





// vtb_debug_print - Forwards the text to the debugger's output pane, if it exists.
// The Windows equivalent is OutputDebugString.
VTBDEF void vtb_debug_print(const char* text);





// VAssert(x) - Triggers a breakpoint if x is false.

#ifdef _DEBUG

#include <stdio.h>

#define VAssert(x) \
do { \
	VPRAGMA_WARNING_PUSH \
	VPRAGMA_WARNING_DISABLE(4127) /* conditional expression is constant */ \
	if (!(x)) \
	VPRAGMA_WARNING_POP \
	{ \
		char buf[1024]; \
		sprintf(buf, "Assert failed: %s (%s:%d)\n", #x, __FILE__, __LINE__); \
		vtb_debug_print(buf); \
		VDebugBreak(); \
	} \
	VPRAGMA_WARNING_PUSH \
	VPRAGMA_WARNING_DISABLE(4127) /* conditional expression is constant */ \
} while (0) \
VPRAGMA_WARNING_POP \

#else

#define VAssert(x) do { \
	VPRAGMA_WARNING_PUSH \
	VPRAGMA_WARNING_DISABLE(4127) /* conditional expression is constant */ \
	if (!(x)) /* Still use the value so that the compiler doesn't complain about unused values. */ \
	VPRAGMA_WARNING_POP \
	{} \
	VPRAGMA_WARNING_PUSH \
	VPRAGMA_WARNING_DISABLE(4127) /* conditional expression is constant */ \
} while (0) \
VPRAGMA_WARNING_POP \

#endif





// VCheck and VAssert - These are the same thing internally, but VAssert means program
// state is definitely wrong and probably unrecoverable (eg algorithm
// invariants), while VCheck just means something less serious but still
// unexpected happened. Generally use VCheck for things like checking function
// parameters, and other things that you can recover from, and VAssert for
// things that the program should die if they go wrong.
#define VCheck(x) VAssert(x)





// VUnimplemented - Indicates code that has not been written yet. Generally I don't
// write code that I don't immediately need, and in branches that need to exist
// for completeness but that I currently don't use, I place a VUnimplemented().
// This saves time so that I don't write things I don't use, and it reduces bugs
// because I don't write code that has no clients and so is not well enough tested.
//
// Example:
// if (a.is_empty())
//   a.push(foo);
// else
//   VUnimplemented(); // I don't currently use the non-empty case.
//
#if defined(__ANDROID__)
// If you hit this, the code is incomple
#define VUnimplemented() do { \
	char s[1000]; \
	sprintf(s, "VUnimplemented file " __FILE__ " line %d\n", __LINE__); \
	DebugPrint(s); \
	TAssertNoMsg(false); \
	} while (0)
#else
// If you hit this, the code is incomple
#define VUnimplemented() VAssert(false)
#endif





// VUntested - Sometimes where I would put a VUnimplemented, instead I will write
// a general outline for what the code should look like, but drop a VUntested()
// in front. It's often easier to write the code while I still have the context
// in mind, but I don't want to invest the time to debug the code. If you see
// this, it means you shouldn't take for granted that the code has been debugged.

#define VUntested() VAssert(false)





// VStubbed - For porting large sections of code. Instead of trying to write
// all of the ported code at once, I comment anything that doesn't compile and
// write: VStubbed("Font Loading") or whatever it is that's not ported yet.
// This way I can get code running faster.
#ifdef _DEBUG

// Ruthlessly remove this if you see it.
#define VStubbed(x) \
	do { \
		static bool seen = false; \
		if (!seen) { \
			seen = true; \
			vtb_debug_print("STUBBED: " x "\n"); \
		} \
	} while (false); \

#else

// Code will not build in release mode until stubs are removed.
#define VStubbed(x) ERROR!

#endif





#ifdef __ANDROID__
// A platform whose major input method is touch, and does not have a mouse.
#define V_PLATFORM_TOUCH

// A platform whose rendering capabilities are limited.
#define V_PLATFORM_MOBILE
#endif





inline int vmin(int a, int b)
{
	return (a < b) ? a : b;
}

inline int vmax(int a, int b)
{
	return (a > b) ? a : b;
}

inline long vmin(long a, long b)
{
	return (a < b) ? a : b;
}

inline long vmax(long a, long b)
{
	return (a > b) ? a : b;
}

inline float vmin(float a, float b)
{
	return (a < b) ? a : b;
}

inline float vmax(float a, float b)
{
	return (a > b) ? a : b;
}




// VInvalid - I often use unsigned data types as handles. The very last representable
// number is reserved as an invalid value for error checking purposes. VInvalid is a
// type- safe way of handling those invalid values. Example:
//
// typedef uint32 ListHandle;
//
// ListHandle a = VInvalid(ListHandle);
// if (a == VInvalid(ListHandle))
//   ...
//
#define VInvalid(type) ((type)~0)





// VStackAllocate - Allocate data on the stack
#ifdef _MSC_VER
// No VLA's. Use alloca()
#include <malloc.h>
#define VStackAllocate(type, name, bytes) type* name = (type*)alloca(bytes)
#else
#define VStackAllocate(type, name, bytes) type name[bytes]
#endif





// VArraySize - A macro for anything defined in the format: type name[size];
// VArraySize(name) will return size.
// Note: It only works on the original array, not on a pointer to that array.
#define VArraySize(x) (sizeof(x)/sizeof(x[0]))

// Rounds x up to the nearest multiple of n. Useful for aligning memory.
inline int VAlign(int x, int n)
{
	int over = x%n;
	return over?(x+n-over):x;
}

// Rounds v up to the next highest multiple of 2.
inline int VPo2(int v)
{
	// From https://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
	v--;
	v |= v >> 1;
	v |= v >> 2;
	v |= v >> 4;
	v |= v >> 8;
	v |= v >> 16;
	v++;

	return v;
}


// A snprintf implementation for VS versions before 2015.
// From http://stackoverflow.com/questions/2915672/snprintf-and-visual-studio-2010
#if defined(_MSC_VER) && _MSC_VER < 1900

#include <stdarg.h>

inline int vtb__vsnprintf(char* outBuf, size_t size, const char* format, va_list ap)
{
    int count = -1;

    if (size != 0)
        count = _vsnprintf_s(outBuf, size, _TRUNCATE, format, ap);
    if (count == -1)
        count = _vscprintf(format, ap);

    return count;
}

inline int vtb__snprintf(char *outBuf, size_t size, const char *format, ...)
{
    int count;
    va_list ap;

    va_start(ap, format);
    count = vtb__vsnprintf(outBuf, size, format, ap);
    va_end(ap);

    return count;
}

#define snprintf vtb__snprintf
#define vsnprintf vtb__vsnprintf

#endif

#endif // VTB_H



#ifdef VTB_IMPLEMENTATION

#ifdef __ANDROID__
VTBDEF void vtb_debug_print(const char* text)
{
	__android_log_print(ANDROID_LOG_INFO, "Debug", "%s", text);
}
#elif _MSC_VER
#include <windows.h>
VTBDEF void vtb_debug_print(const char* text)
{
	OutputDebugStringA(text);
}
#else
#include <stdio.h>
VTBDEF void vtb_debug_print(const char* text)
{
	puts(text);
}
#endif

#endif
