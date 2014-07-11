#pragma once

#include <vector>

#include "vector.h"

typedef int node_t;
typedef int edge_t;

class CGraph
{
public:
	class CEdge
	{
	public:
		node_t first;
		node_t second;
	};

	class CNode
	{
	public:
		CNode()
		{
			group = -1;
		}

		std::vector<edge_t> edges;

		int group;

		Vector debug_position;

	public:
		bool HasBeenSeen()
		{
			return group >= 0;
		}
	};

	void AddNode();
	void AddEdge(node_t a, node_t b);

	int    GetNumNodes();
	CNode* GetNode(node_t i);

	int    GetNumEdges();
	CEdge* GetEdge(edge_t i);

	node_t FollowEdge(node_t node, edge_t edge);

protected:
	std::vector<CNode> nodes;
	std::vector<CEdge> edges;
};


