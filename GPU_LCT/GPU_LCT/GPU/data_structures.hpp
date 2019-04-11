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
		bool operator==(SymEdge& o) {
			return (
				nxt == o.nxt &&
				rot == o.rot &&
				vertex == o.vertex &&
				edge == o.edge &&
				face == o.face);
		};
	};

	struct NewPoint
	{
		glm::vec2 pos;
		int index = -1;
		int face_i = -1;
	};

	struct PointBuffers
	{
		Buffer positions;
		Buffer inserted;
		Buffer tri_index;
	};

	struct PointBuffersCPU
	{
		std::vector<glm::vec2> positions;
		std::vector<int> inserted;
		std::vector<int> tri_index;
	};

	struct EdgeBuffers
	{
		Buffer label;
		Buffer is_constrained;
	};

	struct EdgeBuffersCPU
	{
		std::vector<int> label;
		std::vector<int> is_constrained;
	};

	struct SegmentBuffers
	{
		Buffer endpoint_indices;
		Buffer inserted;
	};

	struct SegmentBuffersCPU
	{
		std::vector<glm::ivec2> endpoint_indices;
		std::vector<int> inserted;
	};

	struct TriangleBuffers
	{
		Buffer symedge_indices;
		Buffer ins_point_index;
		Buffer seg_inters_index;
		Buffer edge_flip_index;
		Buffer new_points;
	};

	struct TriangleBuffersCPU
	{
		std::vector<glm::ivec4> symedge_indices;
		std::vector<int> ins_point_index;
		std::vector<int> seg_inters_index;
		std::vector<int> edge_flip_index;
		std::vector<NewPoint> new_points;
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

	template <typename Data>
	void append_vec(std::vector<Data> &goal, std::vector<Data> data)
	{
		goal.insert(goal.end(), data.begin(), data.end());
	};

}

#endif
