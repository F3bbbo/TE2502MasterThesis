#pragma once

#ifndef GPU_DATA_STRUCTURES_HPP
#define GPU_DATA_STRUCTURES_HPP

#include <glm/glm.hpp>
#include <glad/glad.h>
#include "../Buffer.hpp"

namespace GPU
{
	// n + 1 points
	// m segments
	// 2n triangles

	struct SymEdge
	{
		int nxt = -1;
		int rot = -1;

		int vertex = -1;
		int edge = -1;
		int face = -1;
	};

	struct PointBuffers
	{
		Buffer positions;
		Buffer inserted;
		Buffer tri_index;
	};

	struct EdgeBuffers
	{
		Buffer label;
		Buffer is_constrained;
	};

	struct SegmentBuffers
	{
		Buffer endpoint_indices;
		Buffer inserted;
	};

	struct TriangleBuffers
	{
		Buffer vertex_indices;
		Buffer symedge_indices;
		Buffer ins_point_index;
		Buffer seg_inters_index;
		Buffer edge_flip_index;
	};

	struct Edge
	{

		glm::ivec2 edge;
		bool inserted;
		bool constraint_ref;
	};

	struct Face
	{
		glm::ivec3 vert_i;
	};

	struct BufferSizes
	{
		int num_tris;
		int num_points;
	};

}

#endif
