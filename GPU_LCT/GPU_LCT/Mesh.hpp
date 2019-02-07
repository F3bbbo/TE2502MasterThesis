#ifndef MESH_HPP
#define MESH_HPP

#include <vector>
#include <array>
#include <glm/glm.hpp>

#include "data_structures.hpp"

struct VertexRef
{
	glm::vec2 vertice;
	int ref_counter;
};

struct EdgeConstraints
{
	glm::ivec2 edge;
	std::vector<int> constraint_ref;
};

class Mesh
{
public:
	SymEdge* first;
	Mesh();
	~Mesh();
	void Initialize_as_quad(glm::vec2 scale, glm::vec2 translate);
private:
	std::vector<VertexRef> m_vertices; // Each vertice keeps track of how many times it is referenced
	std::vector<EdgeConstraints> m_edges; // Edges keeps track of the constraints it represents
	std::vector<glm::ivec3> m_faces; // Indices to vertices that makes up each face
};

#endif