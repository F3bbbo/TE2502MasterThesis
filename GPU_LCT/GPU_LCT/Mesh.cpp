#include "Mesh.hpp"
#include "trig_functions.hpp"
#include <stack>

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
	m_faces = { {{0, 1, 3}, 0}, {{3, 1, 2}, 0} };

	std::vector<glm::ivec3> triangle_edges;
	for (auto& face : m_faces)
	{
		glm::ivec3 edge_indexes = { -1, -1, -1 };

		// For each edge in the face
		for (int face_edge = 0; face_edge < 3; face_edge++)
		{
			// Get the vertex indices for that edge from the face
			glm::ivec2 curr_edge = { face.vert_i[face_edge], face.vert_i[(face_edge + 1) % 3] };

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
			tri_sym_edges[edge_id]->vertex = m_faces[face_id].vert_i[edge_id];
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

std::vector<VertexRef> const & Mesh::get_vertex_list()
{
	return m_vertices;
}

std::vector<Edge> const & Mesh::get_edge_list()
{
	return m_edges;
}

std::vector<Face> const & Mesh::get_face_list()
{
	return m_faces;
}

std::array<glm::vec2, 2> Mesh::get_edge(int index)
{
	std::array<glm::vec2, 2> ret;
	ret[0] = m_vertices[m_edges[index].edge[0]].vertice;
	ret[1] = m_vertices[m_edges[index].edge[1]].vertice;
	return ret;
}

std::array<glm::vec2, 3> Mesh::get_triangle(int index)
{
	std::array<glm::vec2, 3> ret;
	ret[0] = m_vertices[m_faces[index].vert_i[0]].vertice;
	ret[1] = m_vertices[m_faces[index].vert_i[1]].vertice;
	ret[2] = m_vertices[m_faces[index].vert_i[2]].vertice;
	return ret;
}

glm::vec2 Mesh::get_vertex(int index)
{
	return m_vertices[index].vertice;
}


void Mesh::next_iter()
{
	m_iter_id++;
}

LocateRes Mesh::Locate_point(glm::vec2 p)
{
	return Oriented_walk(first, p);
}


LocateRes Mesh::Oriented_walk(SymEdge* start_edge, const glm::vec2& p)
{
	LocateRes res;
	res.sym_edge = start_edge;
	res.type = LocateType::NEXT;
	// Standard walk mode
	next_iter();
	while (res.type == LocateType::NEXT)
	{
		res = Standard_walk(res.sym_edge, p);
	}

	// Epsilon based walk mode

	res.type = LocateType::NEXT;
	next_iter();
	while (res.type == LocateType::NEXT)
	{
		res = Epsilon_walk(res.sym_edge, p);
	}

	return res;
}

LocateRes Mesh::Standard_walk(SymEdge * current_edge, const glm::vec2 & p)
{
	LocateRes res;
	SymEdge * curr_edge = current_edge;
	//next_edge = nullptr;
	// find which edge to proceed with
	auto tri_v = get_triangle(curr_edge->face);
	glm::vec2 tri_c = tri_centroid(tri_v[0], tri_v[1], tri_v[2]);
	// set res to current triangle, because if no edge are found it is inside current triangle
	res.hit_index = current_edge->face;
	res.sym_edge = current_edge;
	res.type = LocateType::FACE;
	for (unsigned int i = 0; i < 3; i++)
	{
		//check if current sym_edge has an opposing sym_edge
		if (curr_edge->sym() != nullptr)
		{
			//check if edge segment and middle triangle to point segment intersects
			auto edge_v = get_edge(curr_edge->edge);
			if (line_seg_intersection_ccw(edge_v[0], edge_v[1], tri_c, p)) {
				// Check that the next face has not yet been explored in current walk
				int next_face_i = curr_edge->sym()->face;
				if (m_faces[next_face_i].explored != m_iter_id) {
					res.sym_edge = curr_edge->sym();
					res.hit_index = next_face_i;
					res.type = LocateType::NEXT;
					m_faces[next_face_i].explored = m_iter_id;
					break;
				}
				else {
					res.type = LocateType::NONE;
				}
			}
		}
		curr_edge = curr_edge->nxt;

	}
	return res;
}

LocateRes Mesh::Epsilon_walk(SymEdge * current_edge, const glm::vec2 & p)
{
	LocateRes res;

	// loop through edges of triangle to check if point is on any of the edges
	for (unsigned int i = 0; i < 3; i++) {
		//check against edge vertex
		if (point_equal(p, m_vertices[current_edge->vertex].vertice)) {
			res.hit_index = current_edge->vertex;
			res.sym_edge = current_edge;
			res.type = LocateType::VERTEX;
			return res;
		}
		//check against edge
		auto seg = get_edge(current_edge->edge);
		if (point_segment_test(p, seg[0], seg[1])) {
			res.hit_index = current_edge->edge;
			res.sym_edge = current_edge;
			res.type = LocateType::EDGE;
			return res;
		}
		current_edge = current_edge->nxt;
	}

	// do standard walk to check if point is inside triangle otherwise we need to investigate another triangle
	res = Standard_walk(current_edge, p);
	return res;
}

int Mesh::Insert_point_in_edge(glm::vec2 p, SymEdge * e)
{
	//project p onto edge e and add it to the vertices
	auto edge = get_edge(e->edge);
	int vertex_index = m_vertices.size();
	m_vertices.push_back({ point_segment_projection(p, edge[0], edge[1]), 0 });
	// copy crep list
	auto orig_crep = m_edges[e->edge].constraint_ref;
	// insert vertex  in both  faces



	return vertex_index;
}

int Mesh::Insert_point_in_face(glm::vec2 p, SymEdge * e)
{
	// add points to vertex list
	int vertex_index = m_vertices.size();
	m_vertices.push_back({ p, 0 });
	// insert vertex into face
	std::array<SymEdge*, 3> orig_face;
	std::array<SymEdge*, 3> orig_sym;
	SymEdge* curr_e = e;
	//save edges of the original triangle
	for (unsigned int i = 0; i < orig_face.size(); i++)
	{
		orig_face[i] = curr_e;
		orig_sym[i] = curr_e->sym();
		curr_e = curr_e->nxt;
	}
	// create the new triangles
	for (unsigned int i = 0; i < orig_face.size(); i++)
	{
		curr_e = orig_face[i];
		// next edge in original triangle
		int next_id = (i + 1) % orig_face.size();
		// create first edge of new triangle
		SymEdge* tmp = new SymEdge();
		tmp->vertex = orig_face[next_id]->vertex;
		orig_face[i]->nxt = tmp;
		curr_e = orig_face[i]->nxt;
		// create second edge of new triangle
		tmp = new SymEdge();
		tmp->vertex = vertex_index;
		tmp->nxt = orig_face[i];
		curr_e->nxt = tmp;
		// add face to edges
		int face_index = m_faces.size();
		Face face;
		face.vert_i[0] = orig_face[i]->vertex;
		face.vert_i[1] = orig_face[i]->nxt->vertex;
		face.vert_i[2] = orig_face[i]->nxt->nxt->vertex;
		orig_face[i]->face = face_index;
		orig_face[i]->nxt->face = face_index;
		orig_face[i]->nxt->nxt->face = face_index;
		m_faces.push_back(face);
	}
	std::stack<SymEdge*> flip_stack;
	// connect the new triangles together
	for (unsigned int i = 0; i < orig_face.size(); i++)
	{
		// next and previous edge in original triangle
		int next_id = (i + 1) % orig_face.size();
		//int prev_id = (i - 1) % orig_face.size();
		// get next edge of current face
		auto edge = orig_face[i]->nxt;
		// opposing edge
		auto edge_sym = orig_face[next_id]->nxt->nxt;
		// add edge to edge list
		int edge_index = m_edges.size();
		m_edges.push_back({ {edge->vertex, edge_sym->vertex}, {} });
		edge->edge = edge_index;
		edge_sym->edge = edge_index;
		// connect sym of the edges
		edge->nxt->rot = edge_sym;
		edge_sym->nxt->rot = edge;
		// connect orignal edge with its sym
		orig_face[i]->nxt->rot = orig_sym[i];
		// add edge to stack
		flip_stack.push(edge_sym);
	}


	return vertex_index;
}

void Mesh::flip_edges(SymEdge* point, std::stack<SymEdge*>&& edge_indices)
{
	while (edge_indices.size() > 0)
	{
		SymEdge* sym_edge = edge_indices.top();
		edge_indices.pop();

		if (m_edges[sym_edge->edge].constraint_ref.size() == 0 && !is_delaunay(point, sym_edge))
		{
			edge_indices.push(sym_edge->sym()->nxt);
			edge_indices.push(edge_indices.top()->nxt);
			
			// Flip SymEdge
			SymEdge* e1 = sym_edge->nxt;
			SymEdge* e2 = e1->nxt;
			SymEdge* e3 = sym_edge->sym()->nxt;
			SymEdge* e4 = e3->nxt;

			int face_id[2] = { sym_edge->face, sym_edge->sym()->face };

			// Change all links and indices
			e1->nxt = sym_edge;
			e1->rot = e4->sym();
			e1->face = face_id[0];

			e2->nxt = e3;
			e2->rot = sym_edge;
			e2->face = face_id[1];

			e3->nxt = sym_edge->sym();
			e3->rot = e2->sym();
			e3->face = face_id[1];

			e4->nxt = e1;
			e4->rot = sym_edge->sym();
			e4->face = face_id[0];

			sym_edge->nxt = e4;
			sym_edge->rot = e1->sym();
			sym_edge->vertex = e2->vertex;
			sym_edge->face = face_id[0];

			sym_edge->sym()->nxt = e2;
			sym_edge->sym()->rot = e3->sym();
			sym_edge->vertex = e4->vertex;
			sym_edge->face = face_id[1];

			// Create the new edge
			m_edges[sym_edge->edge].edge = { sym_edge->vertex, sym_edge->sym()->vertex };
		}
	}
}

bool Mesh::is_delaunay(SymEdge* point, SymEdge* edge)
{
	if (point->sym() != nullptr)
	{
		SymEdge* other = point->nxt->sym()->nxt->nxt;
		glm::mat4x4 mat;

		std::array<glm::vec2, 3> face_vertices = get_triangle(point->face);
		
		for (int i = 0; i < 3; i++)
		{
			mat[0][i] = face_vertices[i].x;
			mat[1][i] = face_vertices[i].y;
			mat[2][i] = mat[0][i] * mat[0][i] + mat[1][i] * mat[1][i];
			mat[3][i] = 1;
		}

		auto point = get_vertex(other->vertex);
		mat[0][3] = point.x;
		mat[1][3] = point.y;
		mat[2][3] = mat[0][3] * mat[0][3] + mat[1][3] * mat[1][3];
		mat[3][3] = 1;

		if (glm::determinant(mat) > 0)
			return false;
	}
	return true;
}

