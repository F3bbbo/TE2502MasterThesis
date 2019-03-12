#include "Mesh.hpp"
#include <map>

namespace CPU
{
	Mesh::Mesh()
	{
	}

	Mesh::~Mesh()
	{
	}

	void Mesh::initialize_as_quad(glm::vec2 scale = glm::vec2{ 1.f, 1.f }, glm::vec2 translate = glm::vec2{ 0.f, 0.f })
	{
		// First vertex is top left for openGL, remaining order is ccw
		m_vertices = { {{-1.f, 1.f}, 0}, {{-1.f, -1.f}, 0}, {{1.f, -1.f}, 0}, {{1.f, 1.f}, 0} };
		for (auto& vertex : m_vertices) vertex.vertice = vertex.vertice * scale + translate;
		m_edges = { {{0, 1}, {0}} , {{1, 2}, {0}}, {{2, 3}, {0}} , {{3, 0}, {0}}, {{3, 1}, {}} };
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

	std::vector<VertexRef> Mesh::get_vertex_list()
	{
		return m_vertices;
	}

	std::vector<Edge> Mesh::get_edge_list()
	{
		if (m_free_edges.size() == 0)
			return m_edges;
		else
		{
			std::vector<Edge> edges;
			for (size_t ei = 0; ei < m_edges.size(); ei++)
			{
				auto it = std::find(m_free_edges.begin(), m_free_edges.end(), (int)ei); // The cast from unsigned to signed can be a problem if we have more than 2,147,483,647 edges .... or something like that
				if (it == m_free_edges.end())
					edges.push_back(m_edges[ei]);
			}
			return edges;
		}
	}

	std::vector<Face> Mesh::get_face_list()
	{
		if (m_free_faces.size() == 0)
			return m_faces;
		else
		{
			std::vector<Face> faces;
			for (size_t fi = 0; fi < m_faces.size(); fi++)
			{
				auto it = std::find(m_free_faces.begin(), m_free_faces.end(), (int)fi); // The cast from unsigned to signed can be a problem if we have more than 2,147,483,647 edges .... or something like that
				if (it == m_free_faces.end())
					faces.push_back(m_faces[fi]);
			}
			return faces;
		}
	}

	std::array<glm::vec2, 2> Mesh::get_edge(int index)
	{
		std::array<glm::vec2, 2> ret;
		ret[0] = m_vertices[m_edges[index].edge[0]].vertice;
		ret[1] = m_vertices[m_edges[index].edge[1]].vertice;
		return ret;
	}

	glm::vec2 Mesh::get_other_edge_vertex(int eindex, int vindex)
	{
		if (m_edges[eindex].edge[0] != vindex)
			return get_vertex(m_edges[eindex].edge[0]);
		else
			return get_vertex(m_edges[eindex].edge[1]);
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

	int Mesh::locate_face(glm::vec2 p)
	{
		for (unsigned int i = 0; i < m_faces.size(); i++)
		{
			auto face = get_triangle(i);
			if (point_triangle_test(p, face[0], face[1], face[2]))
				return i;
		}
		return -1;
	}


	void Mesh::next_iter()
	{
		m_iter_id++;
	}

	LocateRes Mesh::Locate_point(glm::vec2 p)
	{
		return oriented_walk(first, p);
	}


	LocateRes Mesh::oriented_walk(SymEdge* start_edge, const glm::vec2& p)
	{
		LocateRes res;
		res.sym_edge = start_edge;
		res.type = LocateType::NEXT;
		// Standard walk mode
		next_iter();
		while (res.type == LocateType::NEXT)
		{
			res = standard_walk(res.sym_edge, p);
		}

		// Epsilon based walk mode

		res.type = LocateType::NEXT;
		next_iter();
		while (res.type == LocateType::NEXT)
		{
			res = epsilon_walk(res.sym_edge, p);
		}

		return res;
	}

	LocateRes Mesh::standard_walk(SymEdge * current_edge, const glm::vec2 & p)
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

	LocateRes Mesh::epsilon_walk(SymEdge * current_edge, const glm::vec2 & p)
	{
		LocateRes res;

		// loop through edges of triangle to check if point is on any of the edges
		for (unsigned int i = 0; i < 3; i++) {
			//check against edge vertex
			if (point_equal(p, m_vertices[current_edge->vertex].vertice)) {
				// find the symedge that can be rotated to find all the other symedges with the same vertex
				SymEdge* start_edge = current_edge;
				while (current_edge->sym() != nullptr) {
					current_edge = current_edge->sym()->nxt;
					if (current_edge == start_edge)
						break;
				}
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
		res = standard_walk(current_edge, p);
		return res;
	}

	SymEdge* Mesh::insert_point_in_edge(glm::vec2 p, SymEdge * e)
	{
		// save original e and e_sym to delete them later
		SymEdge* orig_e = e;
		SymEdge* orig_e_sym = e->sym();

		//project p onto edge e and add it to the vertices
		auto edge = get_edge(e->edge);
		int vertex_index = add_vert(point_segment_projection(p, edge[0], edge[1]));
		// copy crep list
		auto orig_crep = m_edges[e->edge].constraint_ref;
		// insert vertex  in both  faces
		std::vector<SymEdge*> orig_face;
		std::vector<SymEdge*> orig_sym;
		SymEdge* curr_e = e;
		// Add the orignal edges from the first triangle
		for (unsigned int i = 0; i < 2; i++) {
			curr_e = curr_e->nxt;
			orig_face.push_back(curr_e);
			orig_sym.push_back(curr_e->sym());
		}
		// Array of the original edge symedges to be reused when creating new ones
		std::stack<SymEdge*> available_symedges;
		available_symedges.push(orig_e);
		// Add the orignal edges from the second triangle if there is one
		curr_e = orig_e_sym;
		if (curr_e != nullptr) {
			for (unsigned int i = 0; i < 2; i++) {
				curr_e = curr_e->nxt;
				orig_face.push_back(curr_e);
				orig_sym.push_back(curr_e->sym());
			}
			// Add sym() edge to orig edge to available symedge list; 
			available_symedges.push(orig_e_sym);
		}
		else {
			orig_face.push_back(e);
			orig_sym.push_back(e->sym());
		}

		unsigned int num_new_tri = orig_face.size();
		// check which case it is then remove the old structures 
		// and set the correct number of new triangles
		if (num_new_tri < 4)
		{
			num_new_tri = 2;
		}
		else {
			remove_face(orig_e_sym->face);
		}
		remove_face(orig_e->face);
		remove_edge(orig_e->edge);

		// Create new triangles
		for (unsigned int i = 0; i < num_new_tri; i++)
		{
			curr_e = orig_face[i];
			// next edge in original triangle
			int next_id = (i + 1) % orig_face.size();
			// create first edge of new triangle
			SymEdge* tmp;
			if (available_symedges.size() == 0)
			{
				tmp = new SymEdge();
			}
			else {
				tmp = available_symedges.top();
				available_symedges.pop();
			}
			tmp->vertex = orig_face[next_id]->vertex;
			orig_face[i]->nxt = tmp;
			curr_e = orig_face[i]->nxt;
			// create second edge of new triangle
			if (available_symedges.size() == 0)
			{
				tmp = new SymEdge();
			}
			else {
				tmp = available_symedges.top();
				available_symedges.pop();
			}
			tmp->vertex = vertex_index;
			tmp->nxt = orig_face[i];
			curr_e->nxt = tmp;
			// add face to edges
			int face_index = add_face({ orig_face[i]->vertex, orig_face[i]->nxt->vertex, orig_face[i]->nxt->nxt->vertex });
			orig_face[i]->face = face_index;
			orig_face[i]->nxt->face = face_index;
			orig_face[i]->nxt->nxt->face = face_index;
		}
		std::stack<SymEdge*> flip_stack;
		// connect the new triangles together
		// check if it is a outer corner edge
		if (orig_face.size() < 4)
		{
			//first new triangle connections
			orig_face[0]->nxt->rot = orig_sym[0];
			orig_face[0]->nxt->nxt->rot = orig_face[1]->nxt->nxt;

			// second new triangle connections
			orig_face[1]->nxt->rot = orig_sym[1];
			orig_face[1]->rot = orig_face[0]->nxt;
			// create edges in m_edge list
			// tri 1 single edge
			int edge_i = add_edge({ { orig_face[0]->vertex, orig_face[0]->nxt->nxt->vertex}, orig_crep });
			orig_face[0]->nxt->nxt->edge = edge_i;
			//edge_i++;
			// shared edge
			edge_i = add_edge({ {orig_face[0]->nxt->vertex, orig_face[0]->nxt->sym()->vertex}, {} });
			orig_face[0]->nxt->edge = edge_i;
			orig_face[0]->nxt->nxt->rot->edge = edge_i;
			//edge_i++;
			// tri 2 single edge
			edge_i = add_edge({ {orig_face[1]->nxt->vertex, orig_face[1]->nxt->nxt->vertex}, orig_crep });
			orig_face[1]->nxt->edge = edge_i;
			//add edges to flip stack
			flip_stack.push(orig_face[0]);
			flip_stack.push(orig_face[1]);
		}
		else {
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
				int edge_index = add_edge({ { edge->vertex, edge_sym->vertex }, (i % 2 == 1) ? orig_crep : std::vector<int>() });
				edge->edge = edge_index;
				edge_sym->edge = edge_index;
				// connect sym of the edges
				edge->nxt->rot = edge_sym;
				edge_sym->nxt->rot = edge;
				// connect orignal edge with its sym
				orig_face[i]->nxt->rot = orig_sym[i];
				// add edge to stack
				flip_stack.push(orig_face[i]);
			}
		}

		SymEdge* ret_edge = orig_face[0]->nxt->nxt;
		flip_edges(std::move(flip_stack));
		//return an edge that has the new point as vertex
		//and is the first one when rotating ccw
		return ret_edge;
	}

	SymEdge* Mesh::insert_point_in_face(glm::vec2 p, SymEdge * e)
	{
		// add points to vertex list
		int vertex_index = add_vert(p);
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
		// remove old face;
		remove_face(curr_e->face);
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
			int face_index = add_face({ orig_face[i]->vertex,orig_face[i]->nxt->vertex, orig_face[i]->nxt->nxt->vertex });
			orig_face[i]->face = face_index;
			orig_face[i]->nxt->face = face_index;
			orig_face[i]->nxt->nxt->face = face_index;
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
			int edge_index = add_edge({ edge->vertex, edge_sym->vertex });
			edge->edge = edge_index;
			edge_sym->edge = edge_index;
			// connect sym of the edges
			edge->nxt->rot = edge_sym;
			edge_sym->nxt->rot = edge;
			// connect orignal edge with its sym
			orig_face[i]->nxt->rot = orig_sym[i];
			// add edge to stack
			flip_stack.push(orig_face[i]);
		}

		SymEdge* ret_edge = orig_face[0]->nxt->nxt;
		flip_edges(std::move(flip_stack));
		//return an edge that has the new point as vertex 
		return ret_edge;
	}

	void Mesh::insert_constraint(std::vector<glm::vec2>&& points, int cref)
	{
		std::vector<int> vertex_list;
		std::vector<SymEdge*> sym_edge_list;
		for (glm::vec2 point : points)
		{
			LocateRes lr = Locate_point(point);
			if (lr.type == LocateType::FACE)
			{
				SymEdge* new_p = insert_point_in_face(point, lr.sym_edge);
				sym_edge_list.push_back(new_p);
				vertex_list.push_back(new_p->vertex);
			}
			else if (lr.type == LocateType::EDGE)
			{
				SymEdge* new_p = insert_point_in_edge(point, lr.sym_edge);
				sym_edge_list.push_back(new_p);
				vertex_list.push_back(new_p->vertex);
			}
			else if (lr.type == LocateType::VERTEX)
			{
				sym_edge_list.push_back(lr.sym_edge);
				vertex_list.push_back(lr.sym_edge->vertex);
			}
		}
		for (size_t vertex = 0; vertex < sym_edge_list.size() - 1; vertex++)
		{
			if (sym_edge_list[vertex]->vertex != vertex_list[vertex])
			{
				sym_edge_list[vertex] = oriented_walk(sym_edge_list[vertex],
					m_vertices[vertex_list[vertex]].vertice).sym_edge;
			}
			if (sym_edge_list[vertex + 1]->vertex != vertex_list[vertex + 1]) {
				sym_edge_list[vertex + 1] = oriented_walk(sym_edge_list[vertex + 1],
					m_vertices[vertex_list[vertex + 1]].vertice).sym_edge;
			}
			insert_segment(sym_edge_list[vertex], sym_edge_list[vertex + 1], cref);
		}
	}

	void Mesh::transform_into_LCT()
	{
		std::vector<std::pair<glm::vec2, SymEdge*>> pRefs;
		do {
			pRefs = disturbance_linear_pass(first);
			for (auto& pRef : pRefs)
			{
				insert_point_in_edge(pRef.first, pRef.second);
			}
		} while (pRefs.size() != 0);

	}

	void Mesh::flip_edges(std::stack<SymEdge*>&& edge_indices)
	{
		while (edge_indices.size() > 0)
		{
			SymEdge* sym_edge = edge_indices.top();
			edge_indices.pop();
			SymEdge* sym_edge_sym = sym_edge->sym();
			if (sym_edge_sym == nullptr)
				continue;

			if (m_edges[sym_edge->edge].constraint_ref.size() == 0 && !is_delaunay(sym_edge))
			{
				edge_indices.push(sym_edge_sym->nxt);
				edge_indices.push(sym_edge_sym->nxt->nxt);

				// Flip SymEdge begins here
				SymEdge* e1 = sym_edge->nxt;
				SymEdge* e2 = e1->nxt;
				SymEdge* e3 = sym_edge_sym->nxt;
				SymEdge* e4 = e3->nxt;

				SymEdge* e1_sym = e1->sym();
				SymEdge* e2_sym = e2->sym();
				SymEdge* e3_sym = e3->sym();
				SymEdge* e4_sym = e4->sym();

				int face_id[2] = { sym_edge->face, sym_edge_sym->face };

				// Change all links and indices
				e1->nxt = sym_edge;
				e1->rot = e4_sym;
				e1->face = face_id[0];

				e2->nxt = e3;
				e2->rot = sym_edge;
				e2->face = face_id[1];

				e3->nxt = sym_edge_sym;
				e3->rot = e2_sym;
				e3->face = face_id[1];

				e4->nxt = e1;
				e4->rot = sym_edge_sym;
				e4->face = face_id[0];

				sym_edge->nxt = e4;
				sym_edge->rot = e1_sym;
				sym_edge->vertex = e2->vertex;
				sym_edge->face = face_id[0];

				sym_edge_sym->nxt = e2;
				sym_edge_sym->rot = e3_sym;
				sym_edge_sym->vertex = e4->vertex;
				sym_edge_sym->face = face_id[1];

				// Only one face index changes during a flip for each face
				for (int i = 0; i < 3; i++)
				{
					if (e3->vertex == m_faces[face_id[0]].vert_i[i])
						m_faces[face_id[0]].vert_i[i] = e4->vertex;
				}

				for (int i = 0; i < 3; i++)
				{
					if (e1->vertex == m_faces[face_id[1]].vert_i[i])
						m_faces[face_id[1]].vert_i[i] = e2->vertex;
				}

				// Create the new edge
				m_edges[sym_edge->edge].edge = { sym_edge->vertex, sym_edge_sym->vertex };
			}
		}
	}

	bool Mesh::is_delaunay(SymEdge* edge)
	{
		if (edge->sym() != nullptr)
		{
			SymEdge* other = edge->sym()->nxt->nxt;
			glm::mat4x4 mat;

			std::array<glm::vec2, 3> face_vertices = get_triangle(edge->face);

			for (int i = 0; i < 3; i++)
			{
				mat[0][i] = face_vertices[i].x;
				mat[1][i] = face_vertices[i].y;
				mat[2][i] = mat[0][i] * mat[0][i] + mat[1][i] * mat[1][i];
				mat[3][i] = 1.f;
			}

			auto other_point = get_vertex(other->vertex);
			mat[0][3] = other_point.x;
			mat[1][3] = other_point.y;
			mat[2][3] = mat[0][3] * mat[0][3] + mat[1][3] * mat[1][3];
			mat[3][3] = 1.f;

			if (glm::determinant(mat) > 0)
				return false;
		}
		return true;
	}

	void Mesh::insert_segment(SymEdge* v1, SymEdge* v2, int cref)
	{
		// Step 1
		std::vector<int> crossed_edge_list;
		std::vector<SymEdge*> edge_list = get_intersecting_edge_list(v1, v2, crossed_edge_list);
		glm::vec2 p1 = m_vertices[v1->vertex].vertice;
		glm::vec2 p2 = m_vertices[v2->vertex].vertice;

		// For each intersected edge we want to, 1. insert points where contraints cross and 2. get a list of all the vertices that intersects the new constrained edge, that includes existing points.

		for (auto cei : crossed_edge_list)
		{
			if (m_edges[(edge_list[cei])->edge].constraint_ref.size() > 0)
			{
				glm::ivec2 edge_index = m_edges[(edge_list[cei])->edge].edge;
				glm::vec2 intersection_point = line_line_intersection_point(m_vertices[edge_index.x].vertice, m_vertices[edge_index.y].vertice, m_vertices[v1->vertex].vertice, m_vertices[v2->vertex].vertice);
				// TODO: Change locate_point to oriented_walk
				LocateRes point_location = Locate_point(intersection_point);
				if (point_location.type == LocateType::EDGE)
				{
					insert_point_in_edge(intersection_point, point_location.sym_edge);
					continue;
				}
			}
		}

		crossed_edge_list.clear();
		edge_list = get_intersecting_edge_list(oriented_walk(v1, p1).sym_edge, oriented_walk(v2, p2).sym_edge, crossed_edge_list);

		// Step 2
		std::deque<std::vector<SymEdge*>> non_tringulated_faces;
		std::vector<SymEdge*> top_face_points;
		std::vector<SymEdge*> bottom_face_points;

		int current_cei = 0;
		bool prev_crossed = false;
		for (size_t ei = 1; ei < edge_list.size(); ei++)
		{
			if (crossed_edge_list.size() != 0 && ei == crossed_edge_list[current_cei])
			{
				// Open new face
				if (!prev_crossed)
				{
					top_face_points.push_back(edge_list[ei - 1]);
					top_face_points.push_back(edge_list[ei - 1]->nxt->nxt);
					bottom_face_points.push_back(edge_list[ei - 1]);
					prev_crossed = true;
				}

				remove_face(edge_list[ei]->face);

				if (edge_list[ei]->nxt->nxt->vertex == top_face_points.back()->vertex)
					top_face_points.push_back(edge_list[ei]->nxt);

				if (edge_list[ei]->nxt->nxt->vertex == bottom_face_points.back()->nxt->vertex)
					bottom_face_points.push_back(edge_list[ei]->nxt->nxt);

				current_cei = (current_cei + 1) % crossed_edge_list.size();
			}
			else {
				// Close prev face
				if (prev_crossed)
				{
					bottom_face_points.push_back(edge_list[ei - 1]->sym()->nxt);
					bottom_face_points.push_back(edge_list[ei - 1]->sym()->nxt->nxt);
					top_face_points.push_back(edge_list[ei - 1]->sym()->nxt->nxt);
					std::reverse(top_face_points.begin(), top_face_points.end());

					non_tringulated_faces.push_back(std::move(bottom_face_points));
					std::vector<SymEdge*> bottom_face_points_syms;
					bottom_face_points_syms.reserve(non_tringulated_faces.back().size());
					for (auto symedge : non_tringulated_faces.back())
						bottom_face_points_syms.push_back(symedge->sym());
					non_tringulated_faces.push_back(std::move(bottom_face_points_syms));


					non_tringulated_faces.push_back(std::move(top_face_points));
					std::vector<SymEdge*> top_face_points_syms;
					top_face_points_syms.reserve(non_tringulated_faces.back().size());
					for (auto symedge : non_tringulated_faces.back())
						top_face_points_syms.push_back(symedge->sym());
					non_tringulated_faces.push_back(std::move(top_face_points_syms));

					remove_face(edge_list[ei - 1]->sym()->face);

					prev_crossed = false;
				}
				else
					non_tringulated_faces.push_back({ edge_list[ei - 1] });
			}
		}

		for (int cel : crossed_edge_list)
		{
			if (m_edges[edge_list[cel]->edge].constraint_ref.size() == 0)
			{
				remove_edge(edge_list[cel]->edge);
				delete edge_list[cel];
			}
		}

		// step 3
		while (!non_tringulated_faces.empty())
		{
			auto element = non_tringulated_faces.front();
			if (element.size() == 1)
			{
				m_edges[element[0]->edge].constraint_ref.push_back(cref);
				non_tringulated_faces.pop_front();
			}
			else
			{
				// bottom
				SymEdge* ba = new SymEdge();
				ba->vertex = non_tringulated_faces[0].back()->vertex;
				ba->edge = add_edge({ { non_tringulated_faces[0].back()->vertex, non_tringulated_faces[0].front()->vertex }, {cref} });
				triangulate_pseudopolygon_delaunay(
					non_tringulated_faces[0].data(),
					non_tringulated_faces[1].data(),
					0, non_tringulated_faces.front().size() - 1, ba);

				non_tringulated_faces.pop_front();
				non_tringulated_faces.pop_front();

				// top
				SymEdge* ab = new SymEdge();
				ab->vertex = non_tringulated_faces[0].back()->vertex;
				ab->edge = ba->edge;
				triangulate_pseudopolygon_delaunay(
					non_tringulated_faces[0].data(),
					non_tringulated_faces[1].data(),
					0, non_tringulated_faces.front().size() - 1, ab);

				non_tringulated_faces.pop_front();
				non_tringulated_faces.pop_front();

				ab->nxt->rot = ba;
				ba->nxt->rot = ab;
			}
		}
	}

	std::vector<SymEdge*> Mesh::get_intersecting_edge_list(SymEdge* v1, SymEdge* v2, std::vector<int>& vertex_list)
	{
		SymEdge* triangle = v1;
		std::array<glm::vec2, 2> constraint_edge = { m_vertices[v1->vertex].vertice, m_vertices[v2->vertex].vertice };
		std::vector<SymEdge*> edge_list;

		while (true)
		{
			while (true)
			{
				// Check for parallel edge
				std::array<glm::vec2, 2> edge = { m_vertices[triangle->vertex].vertice, m_vertices[triangle->nxt->vertex].vertice };
				if (glm::dot(glm::normalize(constraint_edge[1] - constraint_edge[0]), glm::normalize(edge[1] - edge[0])) > 1 - EPSILON)
				{
					/*vertex_list.push_back(edge_list.size());*/
					edge_list.push_back(triangle);

					if (face_contains_vertex(v2->vertex, triangle->face))
					{
						edge_list.push_back(triangle->nxt);
						return edge_list;
					}
					else
					{
						triangle = triangle->sym()->rot;
						continue;
					}
				}
				// Check for parallel next edge
				edge = { m_vertices[triangle->vertex].vertice, m_vertices[triangle->nxt->nxt->vertex].vertice };
				if (glm::dot(glm::normalize(constraint_edge[1] - constraint_edge[0]), glm::normalize(edge[1] - edge[0])) > 1 - EPSILON)
				{
					/*vertex_list.push_back(edge_list.size());*/
					edge_list.push_back(triangle->rot);

					if (face_contains_vertex(v2->vertex, triangle->face))
					{
						edge_list.push_back(triangle->rot->nxt);
						return edge_list;
					}
					else
					{
						triangle = triangle->rot->sym()->rot;
						continue;
					}
				}

				// If not at endpoint, check to which edge we should walk
				std::array<glm::vec2, 2> other_edge = get_edge(triangle->nxt->edge);
				if (line_seg_intersection_ccw(constraint_edge[0], constraint_edge[1], other_edge[0], other_edge[1]))
				{
					edge_list.push_back(triangle);
					vertex_list.push_back(edge_list.size());
					edge_list.push_back(triangle->nxt);
					triangle = triangle->nxt->sym()->nxt;
					break;
				}

				/*if (triangle->rot == v1 || triangle->rot == nullptr)
					return edge_list;*/

				triangle = triangle->rot;
			}

			// walk towards the constraint endpoins while gathering the intersected edges, stop if we reach the triangle that contains the segment endpoint
			while (triangle != nullptr)
			{
				// Check if we have arrived to a triangle that contains the segment endpoint
				if (face_contains_vertex(v2->vertex, triangle->face))
				{
					edge_list.push_back(triangle->nxt);
					return edge_list;
				}

				// Checks if the segment intersects a vertex
				if (point_segment_test(m_vertices[triangle->nxt->vertex].vertice, constraint_edge[0], constraint_edge[1]))
				{
					triangle = triangle->sym()->rot;
					break;
				}

				int checks = 0;
				while (checks < 2)
				{
					// Checks if the segment intersects an edge
					std::array<glm::vec2, 2> other_edge = get_edge(triangle->edge);
					if (line_seg_intersection_ccw(constraint_edge[0], constraint_edge[1], other_edge[0], other_edge[1]))
					{
						vertex_list.push_back(edge_list.size());
						edge_list.push_back(triangle);
						triangle = triangle->sym()->nxt;
						break;
					}

					triangle = triangle->nxt;
					checks++;
				}
			}
		}
	}

	bool Mesh::face_contains_vertex(int vertex, int face)
	{
		if (m_faces[face].vert_i[0] == vertex || m_faces[face].vert_i[1] == vertex || m_faces[face].vert_i[2] == vertex)
			return true;
		else
			return false;
	}

	bool Mesh::edge_contains_vertex(int vertex, int edge)
	{
		if (m_edges[edge].edge[0] == vertex || m_edges[edge].edge[1] == vertex)
			return true;
		else
			return false;
	}

	bool Mesh::is_constrained(int edge)
	{
		if (m_edges[edge].constraint_ref.size() > 0)
			return true;
		else
			return false;
	}

	void Mesh::triangulate_pseudopolygon_delaunay(SymEdge** points, SymEdge** syms, unsigned int start_i, unsigned int end_i, SymEdge* edge_ab)
	{
		int list_size = end_i - start_i + 1;
		if (list_size > 3)
		{
			int c = start_i + 1;
			for (unsigned int i = c + 1; i <= end_i; i++) {
				// Check if other point is inside the circumference of the triangle 
				// created by the current best point.
				if (point_in_circle({
					m_vertices[points[end_i]->vertex].vertice,
					m_vertices[points[start_i]->vertex].vertice,
					m_vertices[points[c]->vertex].vertice,
					m_vertices[points[i]->vertex].vertice })) {
					c = i;
				}
			}
			SymEdge* edge_1;
			SymEdge* edge_2;
			SymEdge* edge_1_sym;
			SymEdge* edge_2_sym;
			// check if edge_1 is an outer edge of the untriangulated area
			if (c - start_i < 2)
			{
				edge_1 = points[start_i];
				edge_1_sym = syms[start_i];
			}
			else {
				//create the new symedges of the new triangle
				edge_1 = new SymEdge();
				edge_1->vertex = points[end_i]->vertex;
				edge_1->edge = add_edge({ points[start_i]->vertex, points[c]->vertex });
				// create symedge of next recursive call
				edge_1_sym = new SymEdge();
				edge_1_sym->vertex = points[c]->vertex;
				edge_1_sym->edge = edge_1->edge;
				// continue the recursive retriangulation
				triangulate_pseudopolygon_delaunay(points, syms, start_i, c, edge_1_sym);
			}
			// check if edge_2 is an outer edge of the untriangulated area
			if (end_i - c < 2)
			{
				edge_2 = points[c];
				edge_2_sym = syms[c];
			}
			else {
				//create the new symedges of the new triangle
				edge_2 = new SymEdge();
				edge_2->vertex = points[c]->vertex;
				edge_2->edge = add_edge({ points[c]->vertex, points[end_i]->vertex });
				// create symedge of next recursive call
				edge_2_sym = new SymEdge();
				edge_2_sym->vertex = points[end_i]->vertex;
				edge_2_sym->edge = edge_2->edge;
				// continue the recursive retriangulation
				triangulate_pseudopolygon_delaunay(points, syms, c, end_i, edge_2_sym);
			}
			// connect the symedges of the new triangle
			edge_ab->nxt = edge_1;
			edge_1->nxt = edge_2;
			edge_2->nxt = edge_ab;
			int face_i = add_face({ edge_ab->vertex, edge_1->vertex, edge_2->vertex });
			edge_ab->face = face_i;
			edge_1->face = face_i;
			edge_2->face = face_i;
			// connect the triangle with its new neighbor symedges
			edge_1->nxt->rot = edge_1_sym;
			edge_2->nxt->rot = edge_2_sym;
			// connect the neighbour triangles with the new triangle
			if (edge_1_sym != nullptr)
				edge_1_sym->nxt->rot = edge_1;
			if (edge_2_sym != nullptr)
				edge_2_sym->nxt->rot = edge_2;
		}
		else if (list_size == 3) {
			// combine symedges together to a face
			edge_ab->nxt = points[start_i];
			points[start_i]->nxt = points[start_i + 1];
			points[start_i + 1]->nxt = edge_ab;
			// add the new face
			int face_i = add_face({ edge_ab->vertex, points[start_i]->vertex, points[start_i + 1]->vertex });
			edge_ab->face = face_i;
			points[start_i]->face = face_i;
			points[start_i + 1]->face = face_i;
			//fix sym to outer symedges
			points[start_i]->nxt->rot = syms[start_i];
			points[start_i + 1]->nxt->rot = syms[start_i + 1];
			if (syms[start_i] != nullptr)
				syms[start_i]->nxt->rot = points[start_i];
			if (syms[start_i + 1] != nullptr)
				syms[start_i + 1]->nxt->rot = points[start_i + 1];
		}
		return;
	}

	int Mesh::add_vert(glm::vec2 v)
	{
		int index;
		if (m_free_verts.empty()) {
			index = m_vertices.size();
			m_vertices.push_back({ v, 0 });
		}
		else {
			index = m_free_verts.front();
			m_free_verts.pop_front();
			m_vertices[index].ref_counter = 0;
			m_vertices[index].vertice = v;
		}
		return index;
	}

	void Mesh::remove_vert(int index)
	{
		if (m_vertices[index].ref_counter <= 1) {
			m_free_verts.push_back(index);
		}
		else {
			m_vertices[index].ref_counter--;
		}
	}

	int Mesh::add_edge(glm::ivec2 e)
	{
		return add_edge({ e, {} });
	}

	int Mesh::add_edge(Edge e)
	{
		int index;
		if (m_free_edges.empty()) {
			index = m_edges.size();
			m_edges.push_back(e);
		}
		else {
			index = m_free_edges.front();
			m_free_edges.pop_front();
			m_edges[index].constraint_ref = e.constraint_ref;
			m_edges[index].edge = e.edge;
		}
		return index;
	}

	void Mesh::remove_edge(int index)
	{
		m_free_edges.push_back(index);
	}

	int Mesh::add_face(glm::ivec3 f)
	{
		int index;
		if (m_free_faces.empty()) {
			index = m_faces.size();
			m_faces.push_back({ f, 0 });
		}
		else {
			index = m_free_faces.front();
			m_free_faces.pop_front();
			m_faces[index].vert_i = f;
			m_faces[index].explored = 0;

		}
		return index;
	}

	void Mesh::remove_face(int index)
	{
		m_free_faces.push_back(index);
	}

	//---------------------------------------------
	// LCT
	//---------------------------------------------

	bool Mesh::possible_disturbance(SymEdge* tranveral_corner, SymEdge* segment)
	{
		glm::vec2 a = get_vertex(tranveral_corner->prev()->vertex);
		glm::vec2 b = get_vertex(tranveral_corner->vertex);
		glm::vec2 c = get_vertex(tranveral_corner->nxt->vertex);
		std::array<glm::vec2, 2> segment_endpoints = get_edge(segment->edge);

		glm::vec2 sector_c = project_point_on_line(b, a, c);
		float dist = 2.f * line_length(sector_c - a);
		sector_c = a + glm::normalize(c - a) * dist;
		if (edge_intersects_sector(a, b, sector_c, segment_endpoints))
			return true;

		glm::vec2 p = get_symmetrical_corner(a, b, c);
		sector_c = c + glm::normalize(a - c) * dist;

		if (edge_intersects_sector(a, b, sector_c, segment_endpoints))
			return true;

		return false;
	}

	float Mesh::is_disturbed(SymEdge* constraint, SymEdge* b_sym, SymEdge* v_sym, glm::vec2 e)
	{
		// 1
		if (!no_colliniear_constraints(v_sym))
			return -1.0f;

		glm::vec2 v = get_vertex(v_sym->vertex);
		glm::vec2 a = get_vertex(b_sym->prev()->vertex);
		glm::vec2 b = get_vertex(b_sym->vertex);
		glm::vec2 c = get_vertex(b_sym->nxt->vertex);

		// 2
		if (!is_orthogonally_projectable(v, a, c))
			return -1.0f;

		// 3
		std::array<glm::vec2, 2> c_endpoints = get_edge(constraint->edge);
		glm::vec2 v_prim = project_point_on_line(v, c_endpoints[0], c_endpoints[1]);
		if (!(line_seg_intersection_ccw(v, v_prim, a, c) && line_seg_intersection_ccw(v, v_prim, b, c)))
			return -1.0f;

		// 4
		float dist_v_segment = line_length(project_point_on_line(v, c_endpoints[0], c_endpoints[1]) - v);
		if (!(dist_v_segment < local_clearance(b_sym, constraint)))
			return -1.0f;

		// 5
		if (!(dist_v_segment < line_length(v - e)))
			return -1.0f;

		return dist_v_segment;
	}

	float Mesh::local_clearance(SymEdge* b, SymEdge* segment)
	{
		glm::vec2 b_vert = get_vertex(b->vertex);
		if (segment == nullptr)
		{
			glm::vec2 a_vert = get_vertex(b->prev()->vertex);
			glm::vec2 c_vert = get_vertex(b->nxt->vertex);
			return glm::min(line_length(a_vert - b_vert), line_length(c_vert - b_vert));
		}
		else
		{
			std::array<glm::vec2, 2> c_endpoints = get_edge(segment->edge);
			glm::vec2 b_prim = project_point_on_line(b_vert, c_endpoints[0], c_endpoints[1]);
			return line_length(b_vert - b_prim);
		}
	}

	bool Mesh::no_colliniear_constraints(SymEdge* v)
	{
		SymEdge* edge = v;
		bool reverse_direction = false;
		std::vector<SymEdge*> constrained_edges;
		while (true)
		{
			if (is_constrained(edge->edge))
			{
				std::array<glm::vec2, 2> curr_edge = { get_vertex(edge->vertex), get_other_edge_vertex(edge->edge, edge->vertex) };
				for (SymEdge* c_edge : constrained_edges)
				{
					std::array<glm::vec2, 2> constraint_edge = { get_vertex(edge->vertex), get_other_edge_vertex(c_edge->edge, c_edge->vertex) };
					if (glm::dot(glm::normalize(constraint_edge[1] - constraint_edge[0]), glm::normalize(curr_edge[1] - curr_edge[0])) < -1 + EPSILON)
						return false;
				}
				constrained_edges.push_back(edge);
			}

			if (!reverse_direction)
			{
				if (edge->rot == v)
					return true;

				if (edge->rot != nullptr)
					edge = edge->rot;
				else
				{
					reverse_direction = true;
					edge = v->crot();
					if (edge == nullptr)
						return true;
				}
			}
			else
			{
				edge = edge->crot();
				if (edge == nullptr)
					return true;
			}
		}
	}

	bool Mesh::is_orthogonally_projectable(glm::vec2 v, glm::vec2 a, glm::vec2 b)
	{
		glm::vec2 line = b - a;
		glm::vec2 projected_point = project_point_on_line(v, a, b);

		float projected_length = glm::dot(glm::normalize(line), projected_point - a);

		if (projected_length < 0.f || projected_length * projected_length > line_length2(line))
			return false;

		return true;
	}

	bool Mesh::edge_intersects_sector(glm::vec2 a, glm::vec2 b, glm::vec2 c, std::array<glm::vec2, 2> segment)
	{
		// Assumes that b is the origin of the sector
		glm::vec2 center_prim = project_point_on_line(b, segment[0], segment[1]);
		float center_prim_length = line_length(center_prim - b);
		float sector_radius = glm::min(line_length(a - b), line_length(c - b));

		if (line_length(a - b) < line_length(c - b))
			c = b + glm::normalize(c - b) * sector_radius;
		else
			a = b + glm::normalize(a - b) * sector_radius;

		bool inside_triangle = point_inside_triangle(a, c, b, center_prim);
		bool inside_circle = center_prim_length <= sector_radius;
		glm::vec2 point = line_line_intersection_point(b, center_prim, a, c);
		bool other_side_of_ac = glm::dot(point - b, center_prim - b) > 0 && center_prim_length > line_length(point - b);
		if (inside_triangle || (inside_circle && other_side_of_ac))
			return true;
		return false;
	}

	SymEdge* Mesh::find_closest_constraint(SymEdge* ac)
	{
		// Assumes that ac has been checked outside of the function
		std::stack<SymEdge*> unvisited_triangles;
		unvisited_triangles.push(ac->nxt);
		unvisited_triangles.push(ac->prev());
		SymEdge* ret = nullptr;
		float dist = FLT_MAX;
		SymEdge* b = ac->sym()->nxt->nxt;
		glm::vec2 b_vert = get_vertex(b->vertex);
		while (!unvisited_triangles.empty())
		{
			SymEdge* edge = unvisited_triangles.top();
			unvisited_triangles.pop();

			// checks if either sector is intersected by the edge
			if (!possible_disturbance(b, edge))
				break;
			if (is_constrained(edge->edge))
			{
				std::array<glm::vec2, 2> segment_endpoints = get_edge(edge->edge);
				glm::vec2 b_prim = project_point_on_line(b_vert, segment_endpoints[0], segment_endpoints[1]);
				if (line_length(b_prim - b_vert) < dist && !(point_equal(b_prim, get_vertex(b->prev()->vertex)) || point_equal(b_prim, get_vertex(b->nxt->vertex))))
				{
					dist = line_length(b_prim - b_vert);
					ret = edge;
				}
			}

			if (edge->sym() != nullptr)
			{
				unvisited_triangles.push(edge->sym()->nxt);
				unvisited_triangles.push(edge->sym()->nxt->nxt);
			}
		}
		return ret;
	}

	glm::vec2 Mesh::calculate_pref(SymEdge * c, SymEdge * d)
	{
		auto tri = get_triangle(d->face);
		glm::vec2 circle_center = circle_center_from_points(tri[0], tri[1], tri[2]);
		float radius = glm::distance(circle_center, tri[0]);
		auto constraint_e = get_edge(c->edge);
		auto inter_points = ray_circle_intersection(constraint_e, circle_center, radius);
		if (inter_points.size() == 1) {
			return inter_points[0];
		}
		else if (inter_points.size() == 2) {
			return (inter_points[0] + inter_points[1]) / 2.0f;
		}
		std::cout << "Invalid pRef.\n";
		return glm::vec2();
	}

	std::vector<std::pair<glm::vec2, SymEdge*>> Mesh::disturbance_linear_pass(SymEdge * start_edge)
	{
		next_iter();
		std::vector<std::pair<glm::vec2, SymEdge*>> pRefs;
		std::deque<SymEdge*> triangles;
		triangles.push_back(start_edge);
		m_faces[start_edge->face].explored = m_iter_id;
		while (triangles.size() != 0)
		{
			// Pop current triangle exploring from the triangle list
			SymEdge* curr_e = triangles.front();
			triangles.pop_front();
			// Add new triangles to the triangle list
			for (unsigned int i = 0; i < 2; i++)
			{
				curr_e = curr_e->nxt;
				if (curr_e->sym() != nullptr && m_faces[curr_e->sym()->face].explored != m_iter_id)
				{
					triangles.push_back(curr_e->sym());
					m_faces[curr_e->sym()->face].explored = m_iter_id;
				}
			}
			// Check for disturbances of current triangle
			//std::cout << "Curr_tri " << curr_e->face << std::endl;
			//if (curr_e->face == 11) {
			//	int a = 2;
			//}
			find_triangle_disturbances(curr_e, pRefs);

		}
		return pRefs;
	}

	void Mesh::find_triangle_disturbances(SymEdge * tri, std::vector<std::pair<glm::vec2, SymEdge*>> &prefs)
	{
		SymEdge* curr_tri = tri;
		// Check if any of edges of the triangle is constraints
		std::vector<SymEdge*> constraints;
		std::vector<SymEdge*> edge_ac;
		for (unsigned int i = 0; i < 3; i++)
		{
			if (m_edges[curr_tri->edge].constraint_ref.size() != 0)
			{
				constraints.push_back(curr_tri);
				edge_ac.push_back(curr_tri);
			}
			curr_tri = curr_tri->nxt;
		}

		// if 0 constraints found go through triangle again looking for close enough 
		// constraints
		if (constraints.size() == 0)
		{

			SymEdge* curr_e = tri;
			for (unsigned int i = 0; i < 3; i++)
			{
				SymEdge* constraint = nullptr;
				constraint = find_closest_constraint(curr_e);
				// if constraint was found check for disturbances
				if (constraint != nullptr)
				{
					SymEdge* first_disturb = find_constraint_disturbance(constraint, curr_e, true);
					// Add pRef to list if there is a disturbance
					if (first_disturb != nullptr)
					{
						prefs.push_back(std::pair<glm::vec2, SymEdge*>(
							calculate_pref(constraints[0], first_disturb),
							constraints[0]));
					}

					first_disturb = find_constraint_disturbance(constraint, curr_e, false);
					// Add pRef to list if there is a disturbance
					if (first_disturb != nullptr)
					{
						prefs.push_back(std::pair<glm::vec2, SymEdge*>(
							calculate_pref(constraint, first_disturb),
							constraint));
					}
				}
				curr_e = curr_e->nxt;
			}
		}
		else if (constraints.size() == 1)
		{

			SymEdge* first_disturb = find_constraint_disturbance(constraints[0], edge_ac[0], true);
			// Add pRef to list if there is a disturbance
			if (first_disturb != nullptr)
			{
				prefs.push_back(std::pair<glm::vec2, SymEdge*>(
					calculate_pref(constraints[0], first_disturb),
					constraints[0]));
			}

			first_disturb = find_constraint_disturbance(constraints[0], edge_ac[0], false);
			// Add pRef to list if there is a disturbance
			if (first_disturb != nullptr)
			{
				prefs.push_back(std::pair<glm::vec2, SymEdge*>(
					calculate_pref(constraints[0], first_disturb),
					constraints[0]));
			}
		}
		// if more than 1 constraints exist on the triangle no traversals through 
		// the triangle are possible.
		return;
	}

	SymEdge* Mesh::find_constraint_disturbance(SymEdge * constraint, SymEdge * edge_ac, bool right)
	{

		std::stack<SymEdge*> explore_stack;
		std::array<glm::vec2, 3> R;
		if (right) {
			// Calculate right R triangle
			R[0] = m_vertices[edge_ac->vertex].vertice;
			R[1] = m_vertices[edge_ac->prev()->vertex].vertice;
			{
				glm::vec2 ac = R[0] - m_vertices[edge_ac->nxt->vertex].vertice;
				glm::vec2 dir = glm::normalize(ac);
				glm::vec2 ab = R[1] - m_vertices[edge_ac->nxt->vertex].vertice;
				float b_prim = glm::dot(dir, ab);
				R[2] = R[1] + (dir * (glm::length(ac) - b_prim));
			}
			// set start of explore stack
			if (edge_ac->nxt->nxt->sym() != nullptr)
				explore_stack.push(edge_ac->nxt->nxt->sym());
		}
		else {
			//Then do the left side
			// Calculate R for left side
			R[0] = m_vertices[edge_ac->nxt->vertex].vertice;
			R[1] = m_vertices[edge_ac->prev()->vertex].vertice;
			{
				glm::vec2 ac = R[0] - m_vertices[edge_ac->vertex].vertice;
				glm::vec2 dir = glm::normalize(ac);
				glm::vec2 ab = R[1] - m_vertices[edge_ac->vertex].vertice;
				float b_prim = glm::dot(dir, ab);
				R[2] = R[1] + (dir * (glm::length(ac) - b_prim));
			}
			// set start of explore stack
			if (edge_ac->nxt->sym() != nullptr)
				explore_stack.push(edge_ac->nxt->sym());
		}


		//First do the right side
		//std::cout << "Right side\n";


		std::map<int, bool> traversed_edges;
		// Initilize first disturbance variables
		float best_dist = FLT_MAX;
		SymEdge* first_disturb = nullptr;
		while (!explore_stack.empty())
		{
			SymEdge* curr_e = explore_stack.top();
			explore_stack.pop();
			// TODO: check if point is an disturbance
			//std::cout << "Explore_face: " << curr_e->face << std::endl;
			SymEdge* pot_disturb = curr_e->nxt->nxt;
			float dist = is_disturbed(constraint, edge_ac->prev(), pot_disturb,
				m_vertices[right ? pot_disturb->nxt->vertex : pot_disturb->prev()->vertex].vertice);

			if (dist > 0.0f)
			{
				if (dist < best_dist)
				{
					//std::cout << "Disturbance index: " << pot_disturb->vertex << " Dist: " << dist << "\n";
					first_disturb = pot_disturb;
					best_dist = dist;
				}
			}
			// add next edges 
			curr_e = curr_e->nxt;
			for (unsigned int i = 0; i < 2; i++)
			{
				//Skip edges that are constraints
				if (m_edges[curr_e->edge].constraint_ref.size() == 0)
				{
					// Check if edge has previously been traversed
					if (traversed_edges.find(curr_e->edge) == traversed_edges.end())
					{
						// Add edge index to traversed map
						traversed_edges.insert(std::pair<int, bool>(curr_e->edge, true));
						// Check if edge is inside the R area.
						auto edge = get_edge(curr_e->edge);
						if (segment_triangle_test(edge[0], edge[1], R[0], R[1], R[2]))
							explore_stack.push(curr_e->sym());
					}
				}
				curr_e = curr_e->nxt;
			}

		}

		return first_disturb;
	}
}
