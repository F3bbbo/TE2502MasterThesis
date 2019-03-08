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
	private:
		PointBuffers m_point_bufs;
		EdgeBuffers m_edge_bufs;
		SegmentBuffers m_segment_bufs;
		TriangleBuffers m_triangle_bufs;
	};
}
#endif
