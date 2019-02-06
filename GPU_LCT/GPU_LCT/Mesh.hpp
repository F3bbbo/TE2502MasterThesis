#ifndef MESH_HPP
#define MESH_HPP

#include <vector>
#include <array>
#include <utility>
#include <glm/glm.hpp>

#include "data_structures.hpp"

class Mesh
{
public:
	SymEdge* first;
	Mesh();
	~Mesh();
	void Initialize_as_quad(glm::vec2 scale, glm::vec2 translate);
private:
	std::vector<std::pair<glm::vec2, int>> m_vertices; // Each vertice keeps track of how many times it is referenced
	std::vector<std::pair<glm::ivec2, std::vector<int>>> m_edges; // Edges keeps track of the constraints it represents
	std::vector<std::pair<glm::ivec3, glm::ivec3>> m_faces;
};

#endif