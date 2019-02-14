#ifndef MESH_HPP
#define MESH_HPP

#include <vector>
#include <array>
#include <stack>
#include <glm/glm.hpp>

#include "data_structures.hpp"

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
	std::vector<VertexRef> const& get_vertex_list();
	std::vector<Edge> const& get_edge_list();
	std::vector<Face> const& get_face_list();
	std::array<glm::vec2, 2> get_edge(int index);
	std::array<glm::vec2, 3> get_triangle(int index);
	glm::vec2 get_vertex(int index);
	LocateRes Locate_point(glm::vec2 p);
	// returns index to the vertex that was inserted
	int Insert_point_in_edge(glm::vec2 p, SymEdge* e);
	int Insert_point_in_face(glm::vec2 p, SymEdge* e);
private:
	std::vector<VertexRef> m_vertices; // Each vertice keeps track of how many times it is referenced
	std::vector<Edge> m_edges; // Edges keeps track of the constraints it represents
	std::vector<Face> m_faces; // Indices to vertices that makes up each face
	unsigned long int m_iter_id = 0; //Number indication which iteration id the mesh is currently at
	void next_iter();

	LocateRes Oriented_walk(SymEdge* start_edge, const glm::vec2& p);
	LocateRes Standard_walk(SymEdge* start_edge, const glm::vec2& p);
	LocateRes Epsilon_walk(SymEdge* current_edge, const glm::vec2& p);

	void flip_edges(std::stack<SymEdge*>&& edge_indices);
	bool is_delaunay(SymEdge* edge);
};

#endif