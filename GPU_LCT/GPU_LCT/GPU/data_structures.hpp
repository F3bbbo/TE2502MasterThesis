#pragma once

#ifndef GPU_DATA_STRUCTURES_HPP
#define GPU_DATA_STRUCTURES_HPP

#include <glm/glm.hpp>
#include <glad/glad.h>

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
		GLuint positions = 0;
		GLuint inserted = 0;
		GLuint tri_index = 0;
	};

	struct EdgeBuffers
	{
		GLuint label = 0;
		GLuint is_constrained = 0;
	};

	struct SegmentBuffers
	{
		GLuint endpoint_indices = 0;
		GLuint inserted = 0;
	};

	struct TriangleBuffers
	{
		GLuint vertex_indices = 0;
		GLuint symedges = 0;
		GLuint ins_point_index = 0;
		GLuint seg_inters_index = 0;
		GLuint edge_flip_index = 0;
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
}

#endif
