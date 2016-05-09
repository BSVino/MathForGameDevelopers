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

#include "game.h"

#include <algorithm>

#ifdef _WIN32
#include "sys/timeb.h"
#endif

#ifdef __clang__
#include <sys/timeb.h>
#include <stdlib.h>
#endif

#include <float.h>
#include <assert.h>
#include <math.h>

#include "mtrand.h"

#include "main.h"

#define VTB_IMPLEMENTATION
#include "vtb.h"

using std::vector;


// This is actually midpoint rule
double BoringRectangles(double t0, double t1, int n)
{
	double h = (t1-t0) / n;
	double sum = 0;

	for (int k = 0; k < n; k++)
	{
		double t = t0 + k*h + h/2;
		double rectangle_area = h * exp(t);
		sum += rectangle_area;
	}

	return sum;
}

double CompositeSimpsonsRule(double t0, double t1, int n)
{
	double h = (t1-t0) / n;
	double endpoints = exp(t0) + exp(t1);
	double X4 = 0;
	double X2 = 0;

	for (int k = 1; k < n; k += 2)
	{
		double t = t0 + k*h;
		X4 += exp(t);
	}

	for (int k = 2; k < n; k += 2)
	{
		double t = t0 + k*h;
		X2 += exp(t);
	}

	return (h / 3) * (endpoints + 4 * X4 + 2 * X2);
}



int main(int argc, char* argv[])
{
	printf("Actual value:      %.16f\n", exp(1) - 1);
	printf("Boring rectangles: %.16f\n", BoringRectangles(0, 1, 64));
	printf("Composite Simpson: %.16f\n", CompositeSimpsonsRule(0, 1, 64));

	mtsrand(0);

	// Create a game
	CGame game(argc, argv);

	// Open the game's window
	if (!game.OpenWindow(1000, 564, false, false))
	{
		printf("Couldn't open OpenGL Window!\n");
		return 1;
	}

	game.SetMouseCursorEnabled(false);

	game.Load();

	// Run the game loop!
	game.GameLoop();

	return 0;
}
