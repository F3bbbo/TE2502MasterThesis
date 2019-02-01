#ifndef MESH_HPP
#define MESH_HPP

#include <vector>
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
	std::vector<glm::vec2> m_vertices;
	std::vector<glm::ivec2> m_edges;
	std::vector<glm::ivec3> m_faces;
};

#endif