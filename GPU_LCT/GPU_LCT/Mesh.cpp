#include "Mesh.hpp"
#include <map>
#include <array>
#include <vector>
Mesh::Mesh()
{
}

Mesh::~Mesh()
{
}

void Mesh::Initialize_as_quad(glm::vec2 scale = glm::vec2{ 1.f, 1.f }, glm::vec2 translate = glm::vec2{ 0.f, 0.f })
{
	// First vertex is top left for openGl, remaining order is ccw
	m_vertices = { {{-1.f, 1.f}, 0}, {{-1.f, -1.f}, 0}, {{1.f, -1.f}, 0}, {{1.f, 1.f}, 0} };
	for (auto& vertex : m_vertices) vertex.first = vertex.first * scale + translate;
	m_edges = { {{0, 1}, {}} , {{1, 2}, {}}, {{2, 3}, {}} , {{3, 0}, {}}, {{1, 3}, {}} };
	// faces of edge indices
	//m_faces = { {0, 4, 3}, {4, 1, 2} };
	// faces of vertex indices
	m_faces = { {0, 1, 3}, {1, 2, 3} };
	// Build SymEdge structure
	std::vector<SymEdge*> sym_edges;
	// key is edge index, value is its SymEdge index
	std::map<int, int> edge_neighbour;
	for (unsigned int i = 0; i < m_faces.size(); i++)
	{
		std::array<int, 3> sym_edge_indices;
		//std::array<int, 3> face_edge_indices;
		//Create the sym edges triangle with next operator
		for (unsigned int j = 0; j < m_faces[i].length(); j++) {
			SymEdge* tmp = new SymEdge();
			// find out which edge belongs to this edge
			int next_i = (j + 1) % m_faces[i].length();
			for (unsigned int k = 0; k < m_edges.size(); k++)
			{
				if ((m_edges[k].first[0] == m_faces[i][j] && m_edges[k].first[1] == m_faces[i][next_i]) ||
					(m_edges[k].first[1] == m_faces[i][j] && m_edges[k].first[0] == m_faces[i][next_i])) {
					tmp->edge = k;
				}
			}
			tmp->face = i;
			sym_edges.push_back(tmp);
			sym_edge_indices[j] = sym_edges.size() - 1;
		}
		//Add next adjencency on this triangle
		for (unsigned int j = 0; j < sym_edge_indices.size(); j++)
		{
			int next_i = (j + 1) % sym_edge_indices.size();
			sym_edges[sym_edge_indices[j]]->nxt = sym_edges[sym_edge_indices[next_i]];
		}

		//Add rots to triangle
		for (unsigned int j = 0; j < sym_edge_indices.size(); j++) {
			//check if there exist another triangle adjecent to current triangle at current edge
			auto it = edge_neighbour.find(sym_edges[sym_edge_indices[j]]->edge);
			if (it != edge_neighbour.end()) {
				//add rot to both triangles
				SymEdge* curr_edge = sym_edges[sym_edge_indices[j]];
				curr_edge->nxt->rot = sym_edges[it->second];
				curr_edge = sym_edges[it->second];
				curr_edge->nxt->rot = sym_edges[sym_edge_indices[j]];
			}
			else {
				// if 
				edge_neighbour[sym_edges[sym_edge_indices[j]]->edge] = sym_edge_indices[j];
			}
		}
	}
}
