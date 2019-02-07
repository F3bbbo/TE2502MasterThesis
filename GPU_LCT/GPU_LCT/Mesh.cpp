#include "Mesh.hpp"

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
}

void Mesh::Initialize_as_quad(glm::vec2 scale = glm::vec2{ 1.f, 1.f }, glm::vec2 translate = glm::vec2{ 0.f, 0.f })
{
	// First vertex is top left for openGL, remaining order is ccw
	m_vertices = { {{-1.f, 1.f}, 0}, {{-1.f, -1.f}, 0}, {{1.f, -1.f}, 0}, {{1.f, 1.f}, 0} };
	for (auto& vertex : m_vertices) vertex.vertice = vertex.vertice * scale + translate;
	m_edges = { {{0, 1}, {}} , {{1, 2}, {}}, {{2, 3}, {}} , {{3, 0}, {}}, {{3, 1}, {}} };
	m_faces = { {0, 1, 3}, {3, 1, 2} };

	std::vector<glm::ivec3> triangle_edges;
	for (auto& face : m_faces)
	{
		glm::ivec3 edge_indexes = {-1, -1, -1};

		// For each edge in the face
		for (int face_edge = 0; face_edge < 3; face_edge++)
		{
			// Get the vertex indices for that edge from the face
			glm::ivec2 curr_edge = {face[face_edge], face[(face_edge + 1) % 3]};

			// For all edges in our m_edges list, check if they match. If they do not match swap the elements and see if they match
			for (size_t edge_id = 0; edge_id < m_edges.size(); edge_id++)
			{
				if ((curr_edge.x == m_edges[edge_id].edge.x && curr_edge.y == m_edges[edge_id].edge.y) || (curr_edge.x == m_edges[edge_id].edge.y && curr_edge.y == m_edges[edge_id].edge.x))
				{
					edge_indexes[face_edge] = edge_id;
					break;
				}
			}
		}
		triangle_edges.push_back(edge_indexes);
	}


	// Build SymEdge structure
	std::vector<SymEdge*> sym_edges;
	std::array<SymEdge*, 3> tri_sym_edges;

	for (size_t face_id = 0; face_id < m_faces.size(); face_id++)
	{
		for (int edge_id = 0; edge_id < 3; edge_id++)
		{
			tri_sym_edges[edge_id] = new SymEdge();
			tri_sym_edges[edge_id]->face = face_id;
			tri_sym_edges[edge_id]->edge = triangle_edges[face_id][edge_id];
			tri_sym_edges[edge_id]->vertex = m_faces[face_id][edge_id];
		}

		tri_sym_edges[0]->nxt = tri_sym_edges[1];
		tri_sym_edges[1]->nxt = tri_sym_edges[2];
		tri_sym_edges[2]->nxt = tri_sym_edges[0];

		sym_edges.push_back(tri_sym_edges[0]);
		sym_edges.push_back(tri_sym_edges[1]);
		sym_edges.push_back(tri_sym_edges[2]);
	}

	// We want to find the rot for the sym_edge pointer
	for (SymEdge* sym_edge : sym_edges)
	{
		// By check all the other sym_edges
		for (SymEdge* other_sym_edge : sym_edges)
		{
			// If both edges starts at the same vertex
			if (sym_edge->face != other_sym_edge->face && sym_edge->vertex == other_sym_edge->vertex)
			{
				if (sym_edge->nxt->nxt->edge == other_sym_edge->edge)
					sym_edge->rot = other_sym_edge;
			}
		}
	}
	
	first = sym_edges[0];
}
