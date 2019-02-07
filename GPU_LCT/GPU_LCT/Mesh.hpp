#ifndef MESH_HPP
#define MESH_HPP

#include <vector>
#include <array>
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
	NONE
};

struct LocateRes {
	int hit_index = -1;
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
private:
	std::vector<VertexRef> m_vertices; // Each vertice keeps track of how many times it is referenced
	std::vector<Edge> m_edges; // Edges keeps track of the constraints it represents
	std::vector<Face> m_faces; // Indices to vertices that makes up each face
	unsigned int m_iter_id = 0; //Number indication which iteration id the mesh is currently at
	LocateRes Locate_point(glm::vec2 p);
	LocateRes Oriented_walk(SymEdge* start_edge, glm::vec2 p);
};

#endif