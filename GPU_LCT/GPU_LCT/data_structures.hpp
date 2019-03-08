#pragma once

#ifndef DATA_STRUCTURES_HPP
#define DATA_STRUCTURES_HPP
#include <glm/glm.hpp> 

namespace CPU
{

	struct SymEdge
	{
		SymEdge* nxt = nullptr;
		SymEdge* rot = nullptr;

		int vertex;
		int edge;
		int face;
		SymEdge* sym() { return this->nxt->rot; };
		SymEdge* crot() { return (this->sym() != nullptr) ? this->sym()->nxt : nullptr; };
		SymEdge* prev() { return this->nxt->nxt; };
	};

	struct VertexRef
	{
		glm::vec2 vertice;
		int ref_counter;
	};

	struct Edge
	{
		glm::ivec2 edge;
		std::vector<int> constraint_ref;
	};

	struct Face
	{
		glm::ivec3 vert_i;
		unsigned int explored = 0; //number indicating last iteration being explored
	};

	enum class LocateType {
		VERTEX,
		EDGE,
		FACE,
		NEXT,
		NONE
	};

	struct LocateRes {
		int hit_index = -1;
		SymEdge* sym_edge = nullptr;
		LocateType type = LocateType::NONE;
	};
}
#endif