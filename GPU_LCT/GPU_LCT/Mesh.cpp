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
	res = Standard_walk(current_edge, p);
	return res;
}

SymEdge* Mesh::Insert_point_in_edge(glm::vec2 p, SymEdge * e)
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
	// Add the orignal edges from the second triangle if there is one
	curr_e = e->sym();
	if (curr_e != nullptr) {
		for (unsigned int i = 0; i < 2; i++) {
			curr_e = curr_e->nxt;
			orig_face.push_back(curr_e);
			orig_sym.push_back(curr_e->sym());
		}
	}
	else {
		orig_face.push_back(e);
		orig_sym.push_back(e->rot);
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
		orig_face[1]->rot = orig_face[1]->nxt;
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
		// Delete old symedge 
		delete orig_e;
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
			int edge_index = m_edges.size();
			add_edge({ edge->vertex, edge_sym->vertex });
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
		// Delete old symedges
		delete orig_e;
		delete orig_e_sym;
	}

	SymEdge* ret_edge = orig_face[0]->nxt->nxt;
    flip_edges(std::move(flip_stack));
    //return an edge that has the new point as vertex
	//and is the first one when rotating ccw
    return ret_edge;
}

SymEdge* Mesh::Insert_point_in_face(glm::vec2 p, SymEdge * e)
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
	std::vector<SymEdge*> vertex_list;
	for (glm::vec2 point : points)
	{
		LocateRes lr = Locate_point(point);
		if (lr.type == LocateType::FACE)
			vertex_list.push_back(Insert_point_in_face(point, lr.sym_edge));
		else if (lr.type == LocateType::EDGE)
			vertex_list.push_back(Insert_point_in_edge(point, lr.sym_edge));
		else if (lr.type == LocateType::VERTEX)
			vertex_list.push_back(lr.sym_edge);
	}
	/*for (size_t vertex = 0; vertex < vertex_list.size() - 1; vertex++)
		insert_segment(vertex_list[vertex], vertex_list[vertex + 1], cref);*/
}

void Mesh::flip_edges(std::stack<SymEdge*>&& edge_indices)
{
	while (edge_indices.size() > 0)
	{
		SymEdge* sym_edge = edge_indices.top()->nxt->sym();
		edge_indices.pop();
		if (sym_edge == nullptr)
			continue;
		SymEdge* sym_edge_sym = sym_edge->sym();

		if (m_edges[sym_edge->edge].constraint_ref.size() == 0 && !is_delaunay(sym_edge))
		{
			edge_indices.push(sym_edge->nxt);
			edge_indices.push(sym_edge->nxt->nxt);

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
	std::vector<SymEdge*> edge_list = get_intersecting_edge_list(v1, v2);

	if (edge_list.size() == 0)
		return;

	std::vector<SymEdge*> vertex_list;
	vertex_list.push_back(v1);
	// For each intersected edge we want to, 1. insert points where contraints cross and 2. get a list of all the vertices that intersects the new constrained edge, that includes existing points.
	for (std::vector<SymEdge*>::iterator edge_it = edge_list.begin(); edge_it != edge_list.end(); edge_it++ )
	{
		if (m_edges[(*edge_it)->edge].constraint_ref.size() > 0)
		{
			glm::ivec2 edge_index = m_edges[(*edge_it)->edge].edge;
			glm::vec2 intersection_point = line_line_intersection_point(m_vertices[edge_index.x].vertice, m_vertices[edge_index.y].vertice, m_vertices[v1->vertex].vertice, m_vertices[v2->vertex].vertice);
			LocateRes point_location = Locate_point(intersection_point);
			if (point_location.type == LocateType::EDGE)
			{
				// If we insert a new point the old edge becomes invalid and needs to be removed
				vertex_list.push_back(Insert_point_in_edge(intersection_point, (*edge_it)));
				edge_it = edge_list.erase(edge_it);
			}
			else if (point_location.type == LocateType::VERTEX)
				vertex_list.push_back(point_location.sym_edge);
		}
	}
	vertex_list.push_back(v2);

	// Step 2
	std::vector<std::vector<SymEdge*>> non_tringulated_faces;
	std::vector<SymEdge*> top_face_points;
	std::vector<SymEdge*> bottom_face_points;
	size_t vertex_list_index = 1;
	// edge_list now only contains uncontrained edges
	top_face_points.push_back(edge_list[0]->nxt->nxt);
	top_face_points.push_back(edge_list[0]->nxt);
	bottom_face_points.push_back(edge_list[0]->nxt->nxt);
	for (size_t ei = 0; ei < edge_list.size(); ei++)
	{
		if (edge_list[ei]->nxt->nxt->vertex == top_face_points.back()->vertex)
			top_face_points.push_back(edge_list[ei]->nxt);
		
		if (edge_list[ei]->nxt->nxt->vertex == bottom_face_points.back()->nxt->vertex)
			bottom_face_points.push_back(edge_list[ei]->nxt->nxt);

		// remove face to the left of the symedge
		remove_face(edge_list[ei]->face);

		// Check if the opposite face contains the next segment point
		if (face_contains_vertex(vertex_list[vertex_list_index]->vertex, edge_list[ei]->sym()->face))
		{
			// Insert the last points
			bottom_face_points.push_back(edge_list[ei]->sym()->nxt);
			bottom_face_points.push_back(edge_list[ei]->sym()->nxt->nxt);
			top_face_points.push_back(edge_list[ei]->sym()->nxt->nxt);
			std::reverse(top_face_points.begin(), top_face_points.end());

			// store symedges that forms a face
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

			// remove face to the right of the symedge
			remove_face(edge_list[ei]->sym()->face);

			vertex_list_index++;
			if (vertex_list_index == vertex_list.size() - 1 && ei == edge_list.size() - 1)
				break; // we have reached the last segment point and checked all intersected edges, we are done.

			while (vertex_list_index < vertex_list.size() - 1)
			{
				if (face_contains_vertex(vertex_list[vertex_list_index]->vertex, edge_list[ei + 1]->face))
				{
					top_face_points.push_back(edge_list[ei + 1]->nxt);
					bottom_face_points.push_back(edge_list[ei + 1]->nxt->nxt);
					break;
				}
				vertex_list_index++;
			}
			//remove_face(edge_list[ei]->sym()->face);
		}
	}

	// the faces that are contains the soon to be removed the edges are removed above.
	for (size_t ei = 0; ei < edge_list.size(); ei++)
	{
		remove_edge(edge_list[ei]->edge);
		delete edge_list[ei];
	}

	// step 3
	for (vertex_list_index = 0; vertex_list_index < vertex_list.size() - 1; vertex_list_index++)
	{
		bool connected = false;
		SymEdge* edge = vertex_list[vertex_list_index];
		while (edge != nullptr && edge->rot != vertex_list[vertex_list_index])
		{
			if (edge_contains_vertex(vertex_list[vertex_list_index + 1]->vertex, edge->edge))
			{
				connected = true;
				break;
			}
			edge = edge->rot;
		}

		if (connected)
			m_edges[edge->edge].constraint_ref.push_back(cref);
		else
		{
			// bottom
			SymEdge* ba = new SymEdge();
			ba->vertex = vertex_list[vertex_list_index + 1]->vertex;
			ba->edge = add_edge({ { vertex_list[vertex_list_index]->vertex, vertex_list[vertex_list_index + 1]->vertex }, {cref} });
			triangulate_pseudopolygon_delaunay(
				non_tringulated_faces[vertex_list_index * 4].data(),
				non_tringulated_faces[vertex_list_index * 4 + 1].data(),
				0, non_tringulated_faces[vertex_list_index * 4].size() - 1, ba);

			// top
			SymEdge* ab = new SymEdge();
			ab->vertex = vertex_list[vertex_list_index]->vertex;
			ab->edge = ba->edge;
			triangulate_pseudopolygon_delaunay(
				non_tringulated_faces[vertex_list_index * 4 + 2].data(),
				non_tringulated_faces[vertex_list_index * 4 + 3].data(),
				0, non_tringulated_faces[vertex_list_index * 4 + 2].size() - 1, ab);

			ab->nxt->rot = ba;
			ba->nxt->rot = ab;
		}
	}
}

std::vector<SymEdge*> Mesh::get_intersecting_edge_list(SymEdge* v1, SymEdge* v2)
{
	SymEdge* triangle = v1;
	std::array<glm::vec2, 2> contraint_edge = { m_vertices[v1->vertex].vertice, m_vertices[v2->vertex].vertice };
	std::vector<SymEdge*> edge_list;
	// find the local starting triangle
	while (true)
	{
		if (face_contains_vertex(v2->vertex, triangle->face))
			return edge_list; // should be empty

		// If not at endpoint, check to which edge we should walk
		std::array<glm::vec2, 2> other_edge = get_edge(triangle->nxt->edge);
		if (line_seg_intersection_ccw(contraint_edge[0], contraint_edge[1], other_edge[0], other_edge[1]))
		{
			edge_list.push_back(triangle->nxt);
			triangle = triangle->nxt->sym()->nxt;
			break;
		}

		if (triangle->rot == v1 || triangle->rot == nullptr)
			return edge_list; // should be empty

		triangle = triangle->rot;
	}

	// walk towards the constraint endpoins while gathering the intersected edges, stop if we reach the triangle that contains the segment endpoint
	while (triangle != nullptr)
	{
		// Check if we have arrived to a triangle that contains the segment endpoint
		if (face_contains_vertex(v2->vertex, triangle->face))
			break;

		int checks = 0;
		while (checks < 3)
		{
			// Checks if the segment intersects a vertex
			if (point_segment_test(m_vertices[triangle->vertex].vertice, contraint_edge[0], contraint_edge[1]))
			{
				triangle = triangle->rot->nxt;
				break;
			}

			// Checks if the segment intersects an edge
			std::array<glm::vec2, 2> other_edge = get_edge(triangle->edge);
			if (line_seg_intersection_ccw(contraint_edge[0], contraint_edge[1], other_edge[0], other_edge[1]))
			{
				edge_list.push_back(triangle);
				triangle = triangle->sym();
				break;
			}

			triangle = triangle->nxt;
			checks++;
		}
	}
	return edge_list;
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

void Mesh::triangulate_pseudopolygon_delaunay(SymEdge** points, SymEdge** syms, int start_i, int end_i, SymEdge* edge_ab)
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
