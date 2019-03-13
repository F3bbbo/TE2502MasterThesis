#pragma once
#ifndef GPU_MESH_HPP
#define GPU_MESH_HPP

#include "data_structures.hpp"
#include <glm/glm.hpp>

namespace GPU
{
	class GPUMesh
	{
	public:
		GPUMesh();
		~GPUMesh();
		void initiate_buffers();
		void build_CDT(std::vector<glm::vec2> points, std::vector<glm::ivec2> segments);
	private:
		void setup_compute_shaders();
		void compile_cs(GLuint& program, const char* path);

		PointBuffers m_point_bufs;
		EdgeBuffers m_edge_bufs;
		SegmentBuffers m_segment_bufs;
		TriangleBuffers m_triangle_bufs;
		Buffer m_sym_edges;

		GLuint m_location_program;
		GLuint m_insertion_program;
		GLuint m_marking_program;
		GLuint m_flip_edges_program;
	};
}
#endif
