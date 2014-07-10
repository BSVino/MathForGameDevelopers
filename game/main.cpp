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

#include <vector>
#include <assert.h>

using namespace std;

class CGraph
{
public:
	class CEdge
	{
	public:
		int first;
		int second;
	};

	class CNode
	{
	public:
		vector<int> edges;
	};

	void AddNode()
	{
		nodes.push_back(CNode());
	}

	void AddEdge(int a, int b)
	{
		edges.push_back(CEdge());
		edges.back().first = a;
		edges.back().second = b;

		nodes[a].edges.push_back(edges.size()-1);
		nodes[b].edges.push_back(edges.size()-1);
	}

	CNode* GetNode(int i)
	{
		return &nodes[i];
	}

	CEdge* GetEdge(int i)
	{
		return &edges[i];
	}

protected:
	vector<CNode> nodes;
	vector<CEdge> edges;
};

int main()
{
	CGraph g;
	g.AddNode();
	g.AddNode();
	g.AddNode();

	g.AddEdge(0, 1);
	g.AddEdge(1, 2);
	g.AddEdge(0, 2);

	printf("Node 0 has %d edges.\n", g.GetNode(0)->edges.size());

	for (int i = 0; i < g.GetNode(0)->edges.size(); i++)
	{
		int edge = g.GetNode(0)->edges[i];
		printf("Node 0 has an edge that goes between nodes %d and %d\n", g.GetEdge(edge)->first, g.GetEdge(edge)->second);
	}
}

#if 0
#include "game.h"

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
