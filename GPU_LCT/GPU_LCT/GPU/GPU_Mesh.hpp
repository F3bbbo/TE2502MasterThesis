#pragma once

#ifndef GPU_MESH_HPP
#define GPU_MESH_HPP
#include "data_structures.hpp"

namespace GPU
{
	class GPUMesh
	{
	public:
		GPUMesh();
		~GPUMesh();
		void initiate_buffers(std::vector<glm::vec2> vertices, std::vector<glm::ivec2> constraints_indices, std::vector<glm::ivec3> triangle_indices);
	private:
		PointBuffers m_point_bufs;
		EdgeBuffers m_edge_bufs;
		SegmentBuffers m_segment_bufs;
		TriangleBuffers m_triangle_bufs;
	};
}
#endif
