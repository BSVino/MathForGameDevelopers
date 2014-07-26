#include "graph.h"

void CGraph::AddNode()
{
	CGraph::nodes.push_back(CNode());
}

void CGraph::AddEdge(node_t a, node_t b, float weight)
{
	edges.push_back(CGraph::CEdge());
	edges.back().first = a;
	edges.back().second = b;
	edges.back().weight = weight;

	nodes[a].edges.push_back(edges.size()-1);
	nodes[b].edges.push_back(edges.size()-1);
}

int CGraph::GetNumNodes()
{
	return nodes.size();
}

CGraph::CNode* CGraph::GetNode(node_t i)
{
	return &nodes[i];
}

int CGraph::GetNumEdges()
{
	return edges.size();
}

CGraph::CEdge* CGraph::GetEdge(edge_t i)
{
	return &edges[i];
}

node_t CGraph::FollowEdge(node_t node, edge_t edge)
{
	if (edges[edge].first == node)
		return edges[edge].second;
	else
		return edges[edge].first;
}
