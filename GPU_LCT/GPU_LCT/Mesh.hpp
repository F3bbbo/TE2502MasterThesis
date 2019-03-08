#ifndef MESH_HPP
#define MESH_HPP

#include <vector>
#include <array>
#include <stack>
#include <deque>
#include <glm/glm.hpp>

#include "data_structures.hpp"
#include "trig_functions.hpp"
#include "Log.hpp"

namespace CPU
{

	class Mesh
	{
	public:
		SymEdge* first;
		Mesh();
		~Mesh();
		void initialize_as_quad(glm::vec2 scale, glm::vec2 translate);
		std::vector<VertexRef> get_vertex_list();
		std::vector<Edge> get_edge_list();
		std::vector<Face> get_face_list();
		std::array<glm::vec2, 2> get_edge(int index);
		glm::vec2 get_other_edge_vertex(int eindex, int vindex);
		std::array<glm::vec2, 3> get_triangle(int index);
		glm::vec2 get_vertex(int index);
		int locate_face(glm::vec2 p);
		LocateRes Locate_point(glm::vec2 p);
		// returns index to the vertex that was inserted
		SymEdge* insert_point_in_edge(glm::vec2 p, SymEdge* e);
		SymEdge* insert_point_in_face(glm::vec2 p, SymEdge* e);

		void insert_constraint(std::vector<glm::vec2>&& points, int cref);

		void transform_into_LCT();
	private:
		std::vector<VertexRef> m_vertices; // Each vertice keeps track of how many times it is referenced
		std::deque<int> m_free_verts;
		std::vector<Edge> m_edges; // Edges keeps track of the constraints it represents
		std::deque<int> m_free_edges;
		std::vector<Face> m_faces; // Indices to vertices that makes up each face
		std::deque<int> m_free_faces;
		unsigned long int m_iter_id = 0; //Number indication which iteration id the mesh is currently at
		void next_iter();

		LocateRes oriented_walk(SymEdge* start_edge, const glm::vec2& p);
		LocateRes standard_walk(SymEdge* start_edge, const glm::vec2& p);
		LocateRes epsilon_walk(SymEdge* current_edge, const glm::vec2& p);

		void flip_edges(std::stack<SymEdge*>&& edge_indices);
		bool is_delaunay(SymEdge* edge);

		void insert_segment(SymEdge* v1, SymEdge* v2, int cref);
		std::vector<SymEdge*> get_intersecting_edge_list(SymEdge* v1, SymEdge* v2, std::vector<int>& vertex_list);
		bool face_contains_vertex(int vertex, int face);
		bool edge_contains_vertex(int vertex, int edge);
		bool is_constrained(int edge);

		// returns the 
		void triangulate_pseudopolygon_delaunay(SymEdge** points, SymEdge** syms, unsigned int start_i, unsigned int end_i, SymEdge* edge_ab);

		// functions to insert and remove objects from the struct vectors
		int add_vert(glm::vec2 v);
		void remove_vert(int index);
		int add_edge(glm::ivec2 e);
		int add_edge(Edge e);
		void remove_edge(int index);
		int add_face(glm::ivec3 f);
		void remove_face(int index);

		//---------------------------------------------
		// LCT
		//---------------------------------------------

		bool possible_disturbance(SymEdge* b, SymEdge* segment);
		float is_disturbed(SymEdge* constraint, SymEdge* b_sym, SymEdge* v_sym, glm::vec2 e);
		float local_clearance(SymEdge* b, SymEdge* segment);
		bool no_colliniear_constraints(SymEdge* v);
		// Returns the pRefs from the linear pass that needs to 
		// be inserted into the mesh, 
		// if the list is empty no disturbances where found.
		std::vector<std::pair<glm::vec2, SymEdge*>> disturbance_linear_pass(SymEdge* start_edge);
		void find_triangle_disturbances(SymEdge* tri, std::vector<std::pair<glm::vec2, SymEdge*>> &prefs);
		SymEdge* find_constraint_disturbance(SymEdge* constraint, SymEdge* edge_ac, bool right);
		bool is_orthogonally_projectable(glm::vec2 v, glm::vec2 a, glm::vec2 b);
		bool edge_intersects_sector(glm::vec2 a, glm::vec2 b, glm::vec2 c, std::array<glm::vec2, 2> segment);
		SymEdge* find_closest_constraint(SymEdge* b);
		glm::vec2 calculate_pref(SymEdge* c, SymEdge* d);// c = constraint, d = disturbance
	};
}
#endif