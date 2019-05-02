#pragma once
#ifndef GPU_MESH_HPP
#define GPU_MESH_HPP

#include "data_structures.hpp"
#include "../trig_functions.hpp"
#include "..//Timer.hpp"
#include <glm/glm.hpp>
#include <utility>

namespace GPU
{
	class GPUMesh
	{
	public:
		GPUMesh(glm::ivec2 screen_res);
		~GPUMesh();
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

		PointBuffers m_point_bufs;
		EdgeBuffers m_edge_bufs;
		SegmentBuffers m_segment_bufs;
		TriangleBuffers m_triangle_bufs;
		Buffer m_sym_edges;
		Buffer m_nr_of_symedges;
		Buffer m_status;

		// CDT shaders
		GLuint m_location_program;
		GLuint m_location_tri_program;
		GLuint m_insertion_program;
		GLuint m_marking_part_one_program;
		GLuint m_marking_part_two_program;
		GLuint m_flip_edges_part_one_program;
		GLuint m_flip_edges_part_two_program;
		GLuint m_flip_edges_part_three_program;

		// LCT shaders
		GLuint m_locate_disturbances_program;
		GLuint m_add_new_points_program;
		GLuint m_locate_point_triangle_program;
		GLuint m_validate_edges_program;
		GLuint m_insert_in_edge_program;
	};
}
#endif
