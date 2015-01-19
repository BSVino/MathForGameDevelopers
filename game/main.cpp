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

#include "mtrand.h"
int main(int argc, char* argv[])
{
	mtsrand(0);

	struct timeb t1, t2;

	int a = (int)abs((int)mtrand());
	int b = (int)abs((int)mtrand()%100);

	ftime(&t1);

	int sum = 0;
	for (int i = 0; i < 10; i++)
	{
		int q = 0;
		while (q*b + b <= a)
			q = q + 1;

		sum += q;
	}

	ftime(&t2);

	long elapsed_ms = (long)(t2.time - t1.time) * 1000 + (t2.millitm - t1.millitm);

	printf("Sum: %d\n", sum);
	printf("Normal time: %dms\n", elapsed_ms);


	ftime(&t1);

	sum = 0;
	for (int i = 0; i < 10; i++)
	{
		int q = 0;
		while (q*b + b <= a)
			q = q + 2;

		while (q*b > a)
			q = q - 1;

		sum += q;
	}

	ftime(&t2);

	elapsed_ms = (long)(t2.time - t1.time) * 1000 + (t2.millitm - t1.millitm);

	printf("Sum: %d\n", sum);
	printf("2x unroll time: %dms\n", elapsed_ms);




	return 0;












	// Create a game
	CGame game(argc, argv);

	// Open the game's window
	game.OpenWindow(1000, 564, false, false);
	game.SetMouseCursorEnabled(false);

	game.Load();

	// Run the game loop!
	game.GameLoop();

	return 0;
}
