#pragma once
#ifndef GPU_MESH_HPP
#define GPU_MESH_HPP

#include "data_structures.hpp"
#include <glm/glm.hpp>
#include <utility>

namespace GPU
{
	class GPUMesh
	{
	public:
		GPUMesh(glm::ivec2 screen_res);
		~GPUMesh();
		void initiate_buffers(glm::vec2 scale = {1.f, 1.f});
		void build_CDT(std::vector<glm::vec2> points, std::vector<glm::ivec2> segments);
		std::vector<glm::vec2> get_vertices();
		glm::vec2 get_vertex(int index);
		SymEdge get_symedge(int index);
		std::vector<std::pair<glm::ivec2, bool>> get_edges();
		std::vector<glm::ivec3> get_faces();
	private:
		void setup_compute_shaders();
		void compile_cs(GLuint& program, const char* path);

		PointBuffers m_point_bufs;
		EdgeBuffers m_edge_bufs;
		SegmentBuffers m_segment_bufs;
		TriangleBuffers m_triangle_bufs;
		Buffer m_sym_edges;
		Buffer m_sizes;

		GLuint m_location_program;
		GLuint m_insertion_program;
		GLuint m_marking_program;
		GLuint m_flip_edges_program;
	};
}
#endif
