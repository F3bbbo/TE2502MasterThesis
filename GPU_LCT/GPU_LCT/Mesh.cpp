#include "Mesh.hpp"

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
}

void Mesh::Initialize_as_quad(glm::vec2 scale = glm::vec2{ 1.f, 1.f }, glm::vec2 translate = glm::vec2{ 0.f, 0.f })
{
	// First vertex is top left for openGl, remaining order is ccw
	m_vertices = { {-1.f, 1.f}, {-1.f, -1.f}, {1.f, -1.f}, {1.f, 1.f} };
	for (auto& vertex : m_vertices) vertex = vertex * scale + translate;
	m_edges = { {0, 1}, {1, 2}, {2, 3} , {3, 0}, {1, 3} };
	m_faces = { {0, 4, 3}, {4, 1, 2} };

	// Build SymEdge structure
	for (auto& face : m_faces)
	{

	}
}
