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

#include "mtrand.h"

#define MT_SIZE 624
static size_t g_aiMT[MT_SIZE];
static size_t g_iMTI = 0;

// Mersenne Twister implementation from Wikipedia.
void mtsrand(size_t iSeed)
{
	g_aiMT[0] = iSeed;
	for (size_t i = 1; i < MT_SIZE; i++)
	{
		size_t inner1 = g_aiMT[i-1];
		size_t inner2 = (g_aiMT[i-1]>>30);
		size_t inner = inner1 ^ inner2;
		g_aiMT[i] = (0x6c078965 * inner) + i;
	}

	g_iMTI = 0;
}

size_t mtrand()
{
	if (g_iMTI == 0)
	{
		for (size_t i = 0; i < MT_SIZE; i++)
		{
			size_t y = (0x80000000&(g_aiMT[i])) + (0x7fffffff&(g_aiMT[(i+1) % MT_SIZE]));
			g_aiMT[i] = g_aiMT[(i + 397)%MT_SIZE] ^ (y>>1);
			if ((y%2) == 1)
				g_aiMT[i] = g_aiMT[i] ^ 0x9908b0df;
		}
	}

	size_t y = g_aiMT[g_iMTI];
	y = y ^ (y >> 11);
	y = y ^ ((y << 7) & (0x9d2c5680));
	y = y ^ ((y << 15) & (0xefc60000));
	y = y ^ (y >> 18);

	g_iMTI = (g_iMTI + 1) % MT_SIZE;

	return y;
}
