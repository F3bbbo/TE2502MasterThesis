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
struct VertexRef
{
	glm::vec2 vertice;
	int ref_counter;
};

struct Edge
{
	glm::ivec2 edge;
	std::vector<int> constraint_ref;
};

struct Face
{
	glm::ivec3 vert_i;
	unsigned int explored = 0; //number indicating last iteration being explored
};

enum class LocateType {
	VERTEX,
	EDGE,
	FACE,
	NEXT,
	NONE
};

struct LocateRes {
	int hit_index = -1;
	SymEdge* sym_edge = nullptr;
	LocateType type = LocateType::NONE;
};

class Mesh
{
public:
	SymEdge* first;
	Mesh();
	~Mesh();
	void Initialize_as_quad(glm::vec2 scale, glm::vec2 translate);
	std::vector<VertexRef> get_vertex_list();
	std::vector<Edge> get_edge_list();
	std::vector<Face> get_face_list();
	std::array<glm::vec2, 2> get_edge(int index);
	std::array<glm::vec2, 3> get_triangle(int index);
	glm::vec2 get_vertex(int index);
	LocateRes Locate_point(glm::vec2 p);
	// returns index to the vertex that was inserted
	SymEdge* Insert_point_in_edge(glm::vec2 p, SymEdge* e);
	SymEdge* Insert_point_in_face(glm::vec2 p, SymEdge* e);

	void insert_constraint(std::vector<glm::vec2>&& points, int cref);
private:
	std::vector<VertexRef> m_vertices; // Each vertice keeps track of how many times it is referenced
	std::deque<int> m_free_verts;
	std::vector<Edge> m_edges; // Edges keeps track of the constraints it represents
	std::deque<int> m_free_edges;
	std::vector<Face> m_faces; // Indices to vertices that makes up each face
	std::deque<int> m_free_faces;
	unsigned long int m_iter_id = 0; //Number indication which iteration id the mesh is currently at
	void next_iter();

	LocateRes Oriented_walk(SymEdge* start_edge, const glm::vec2& p);
	LocateRes Standard_walk(SymEdge* start_edge, const glm::vec2& p);
	LocateRes Epsilon_walk(SymEdge* current_edge, const glm::vec2& p);

	void flip_edges(std::stack<SymEdge*>&& edge_indices);
	bool is_delaunay(SymEdge* edge);

	void insert_segment(SymEdge* v1, SymEdge* v2, int cref);
	std::vector<SymEdge*> get_intersecting_edge_list(SymEdge* v1, SymEdge* v2);
	bool face_contains_vertex(int vertex, int face);
	bool edge_contains_vertex(int vertex, int edge);

	// returns the 
	void triangulate_pseudopolygon_delaunay(SymEdge** points, SymEdge** syms, int start_i, int end_i, SymEdge* edge_ab);

	// functions to insert and remove objects from the struct vectors
	int add_vert(glm::vec2 v);
	void remove_vert(int index);
	int add_edge(glm::ivec2 e);
	int add_edge(Edge e);
	void remove_edge(int index);
	int add_face(glm::ivec3 f);
	void remove_face(int index);
};

#endif