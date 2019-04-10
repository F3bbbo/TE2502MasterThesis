#pragma once
#ifndef GPU_CPU_MESH_HPP
#define GPU_CPU_MESH_HPP

#include "data_structures.hpp"
#include "../trig_functions.hpp"
#include "..//Timer.hpp"
#include <glm/glm.hpp>
#include <utility>
using namespace glm;
namespace GPU
{
	class GCMesh
	{
	public:
		GCMesh(glm::ivec2 screen_res);
		~GCMesh();
		void initiate_buffers(glm::vec2 scale = { 1.f, 1.f });
		void build_CDT(std::vector<glm::vec2> points, std::vector<glm::ivec2> segments);
		void refine_LCT();
		std::vector<glm::vec2> get_vertices();
		glm::vec2 get_vertex(int index);
		SymEdge get_symedge(int index);
		std::vector<std::pair<glm::ivec2, bool>> get_edges();
		std::vector<glm::ivec3> get_faces();
		int locate_face(glm::vec2 p);
	private:
		void setup_compute_shaders();
		void compile_cs(GLuint& program, const char* path, int work_group_size = 64);

		// point buffers
		std::vector<glm::vec2> point_positions;
		std::vector<int> point_inserted;
		std::vector<int> point_tri_index;
		// edge buffers
		std::vector<int> edge_label;
		std::vector<int> edge_is_constrained;
		// segment buffers
		std::vector<glm::ivec2> seg_endpoint_indices;
		std::vector<int> seg_inserted;
		// triangle buffers
		std::vector<glm::ivec4> tri_symedges;
		std::vector<int> tri_ins_point_index;
		std::vector<int> tri_seg_inters_index;
		std::vector<int> tri_edge_flip_index;
		std::vector<NewPoint> tri_insert_points;
		// symedges
		std::vector<SymEdge> sym_edges;
		int symedge_buffer_size;
		int status;

		// sym edge functions
		int nxt(int edge);
		int rot(int edge);
		int sym(int edge);
		int prev(int edge);
		int crot(int edge);

		// CDT shaders
		//GLuint m_location_program;
		void location_program();
		//GLuint m_location_tri_program;
		void location_tri_program();
		//GLuint m_insertion_program;
		void insertion_program();
		//GLuint m_marking_part_one_program;
		void marking_part_one_program();
		//GLuint m_marking_part_two_program;
		void marking_part_two_program();
		//GLuint m_flip_edges_part_one_program;
		void flip_edges_part_one_program();
		//GLuint m_flip_edges_part_two_program;
		void flip_edges_part_two_program();
		//GLuint m_flip_edges_part_three_program;
		void flip_edges_part_three_program();

		// LCT shaders
		//GLuint m_locate_disturbances_program;
		void locate_disturbances_program();
		//GLuint m_add_new_points_program;
		void add_new_points_program();
		//GLuint m_insert_in_edge_program;
		void insert_in_edge_program();
		//GLuint m_locate_point_triangle_program;
		void locate_point_triangle_program();
		//GLuint m_validate_edges_program;
		void validate_edges_program();

		// shader functions 
		void oriented_walk(int &curr_e, int point_i, bool &on_edge, vec2 &tri_cent);
		void get_face(int face_i, std::array<vec2, 3> &face_v);
	};
}
#endif