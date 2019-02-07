#pragma once

#ifndef DATA_STRUCTURES_HPP
#define DATA_STRUCTURES_HPP

struct SymEdge
{
	SymEdge* nxt = nullptr;
	SymEdge* rot = nullptr;

	int vertex;
	int edge;
	int face;
	SymEdge* sym() { return this->nxt->rot; };
};
#endif