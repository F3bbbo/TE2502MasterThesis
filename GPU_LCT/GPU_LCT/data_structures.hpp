#pragma once

#ifndef DATA_STRUCTURES_HPP
#define DATA_STRUCTURES_HPP

struct SymEdge
{
	SymEdge* nxt;
	SymEdge* rot;

	int vertex;
	int edge;
	int face;
};
#endif