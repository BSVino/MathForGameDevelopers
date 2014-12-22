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

	int players = 100000;
	vector<int> scores1;
	scores1.reserve(players);

	for (int n = 0; n < players; n++)
		scores1.push_back((int)abs((int)mtrand()));

	vector<int> scores2 = scores1;

	vector<int> scores_sorted;
	scores_sorted.reserve(players);




	struct timeb t1, t2;

	ftime(&t1);

	// Insertion sort
	{
		while (scores1.size()) // While the list of scores is not empty
		{
			// Find the highest score (look at every bit of data in the scores list)
			int highest = -1;
			for (size_t i = 0; i < scores1.size(); i++)
			{
				if (highest < 0 || scores1[i] > scores1[highest])
					highest = i;
			}

			// Add it to the end of the sorted array
			scores_sorted.push_back(scores1[highest]);

			// Remove it from the input array
			scores1.erase(scores1.begin()+highest, scores1.begin()+highest+1);
		}
	}

	ftime(&t2);

	long elapsed_ms = (long)(t2.time - t1.time) * 1000 + (t2.millitm - t1.millitm);

	printf("Highest score: %d\n", scores_sorted[0]);
	printf("Insertion sort time: %dms\n", elapsed_ms);


	scores_sorted.clear();

	ftime(&t1);

	// Heap sort
	{
		// Build a heap from the scores (every item is bigger than the items below it
		std::make_heap(scores2.begin(), scores2.end());

		// While the input list still has items
		while (scores2.size())
		{
			std::pop_heap(scores2.begin(), scores2.end());
			scores_sorted.push_back(scores2.back());
			scores2.pop_back();
		}
	}

	ftime(&t2);

	elapsed_ms = (long)(t2.time - t1.time) * 1000 + (t2.millitm - t1.millitm);

	printf("Highest score: %d\n", scores_sorted[0]);
	printf("Heap sort time: %dms\n", elapsed_ms);




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
