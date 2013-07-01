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

#if 0
int main(int argc, char* argv[])
{
	// Create a game
	CGame game(argc, argv);

	// Open the game's window
	game.OpenWindow(640, 480, false, false);
	game.SetMouseCursorEnabled(false);

	game.Load();

	// Run the game loop!
	game.GameLoop();

	return 0;
}

#endif

#include <iostream>
#include "quaternion.h"

int main(int argc, char* argv[])
{
	printf("Quaternions for:\n\n");

	Quaternion q1(Vector(1, 0, 0), 0);
	printf("Rotating 0 degrees around (1, 0, 0): (%f, %f, %f, %f)\n", q1.w, q1.x, q1.y, q1.z);

	std::string s;
	std::getline(std::cin, s);

	Quaternion q2(Vector(1, 0, 0), 90);
	printf("Rotating 90 degrees around (1, 0, 0): (%f, %f, %f, %f)\n", q2.w, q2.x, q2.y, q2.z);

	std::getline(std::cin, s);

	Quaternion q3(Vector(1, 0, 0), 180);
	printf("Rotating 180 degrees around (1, 0, 0): (%f, %f, %f, %f)\n", q3.w, q3.x, q3.y, q3.z);

	std::getline(std::cin, s);

	Quaternion q4(Vector(0, 1, 0), 90);
	printf("Rotating 90 degrees around (0, 1, 0): (%f, %f, %f, %f)\n", q4.w, q4.x, q4.y, q4.z);

	std::getline(std::cin, s);

	Quaternion q5(Vector(0, 1, 0), 180);
	printf("Rotating 180 degrees around (0, 1, 0): (%f, %f, %f, %f)\n", q5.w, q5.x, q5.y, q5.z);

	std::getline(std::cin, s);
}
