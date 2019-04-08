#version 430
#define EPSILON 0.0005f
layout(local_size_x = 1, local_size_y= 1) in;

struct SymEdge{
	int nxt;
	int rot;

	int vertex;
	int edge;
	int face;
};

//-----------------------------------------------------------
// Inputs
//-----------------------------------------------------------

layout(std430, binding = 0) buffer Point_position
{
	vec2 point_positions[];
};

layout(std430, binding = 1) buffer Points1
{
	int point_inserted[];
};

layout(std430, binding = 2) buffer Points2
{
	int point_tri_index[];
};

layout(std430, binding = 3) buffer Edge0
{
	int edge_label[];
};

layout(std430, binding = 4) buffer Edge1
{
	int edge_is_constrained[];
};

layout(std430, binding = 5) buffer Seg0
{
	ivec2 seg_endpoint_indices[];
};

layout(std430, binding = 6) buffer Seg1
{
	int seg_inserted[];
};

layout(std430, binding = 7) buffer Tri_buff_0
{
	ivec4 tri_symedges[];
};
layout(std430, binding = 8) buffer Tri_buff_1
{
	int tri_ins_point_index[];
};
layout(std430, binding = 9) buffer Tri_buff_2
{
	int tri_seg_inters_index[];
};
layout(std430, binding = 10) buffer Tri_buff_3
{
	int tri_edge_flip_index[];
};
layout(std430, binding = 11) buffer symedge_buff
{
	SymEdge sym_edges[];
};
layout(std430, binding = 12) buffer status_buff
{
	int status;
};

//-----------------------------------------------------------
// Uniforms
//-----------------------------------------------------------
layout (std140, binding = 0) uniform symedge_size
{
	int symedge_buffer_size;
};
//-----------------------------------------------------------
// Access Functions
//-----------------------------------------------------------
SymEdge get_symedge(int index)
{
	return sym_edges[index];
}

// symedge movement functions
SymEdge nxt(SymEdge sym_edge)
{
	return get_symedge(sym_edge.nxt);
}

SymEdge rot(SymEdge sym_edge)
{
	return get_symedge(sym_edge.rot);
}

SymEdge prev(SymEdge s)
{
	return nxt(nxt(s));
}
SymEdge sym(SymEdge s)
{
	return rot(nxt(s));
}

int get_index(SymEdge s)
{
	return prev(s).nxt;
}

vec2 get_vertex(int index)
{
	return point_positions[index];
}

//-----------------------------------------------------------
// Math Functions
//-----------------------------------------------------------

vec2 project_point_on_line(vec2 point, vec2 a, vec2 b)
{
	vec2 ab = b - a;
	vec2 ap = point - a;
	return a + dot(ap, ab) / dot(ab, ab) * ab;
}

bool point_intersects_line(vec2 p, vec2 a, vec2 b, float epsilon = EPSILON)
{
	float hypotenuse = length(project_point_on_line(p, a, b) - a);
	float adjacent = length(p - a);

	return sqrt(hypotenuse * hypotenuse - adjacent * adjacent) < epsilon;
}

bool adjacent_tri_point_intersects_edge(in SymEdge curr_edge, out int face_index)
{
	// checks if the adjacent triangle wants to insert its point in the provided edge

	// if there exists an adjacent triangle that wants to insert a point
	if (nxt(curr_edge).rot != -1 && tri_ins_point_index[sym(curr_edge).face] != -1)
	{
		SymEdge other_insertion_symedge = sym(curr_edge);

		vec2 other_point = get_vertex(tri_ins_point_index[other_insertion_symedge.face]);

		// Check if the adjacent triangle wants to insert into same edge, if true: let the triangle with the lowest index do its insertion
		if (point_intersects_line(other_point, get_vertex(other_insertion_symedge.vertex), get_vertex(nxt(other_insertion_symedge).vertex)))
		{
			face_index = other_insertion_symedge.face;
			return true;
		}
		else
		{
			face_index = -1;
			return false;
		}
	}
	else
	{
		face_index = -1;
		return false;
	}
}

//-----------------------------------------------------------
// Functions
//-----------------------------------------------------------

// Each thread represents one triangle

void main(void)
{
	uint gid = gl_GlobalInvocationID.x;
	int num_threads = int(gl_NumWorkGroups.x * gl_WorkGroupSize.x);
	int index = int(gid);

	while (index < tri_symedges.length())
	{
		if(tri_ins_point_index[index] != -1)
		{
			// find the symedge which the point should be inserted into

			vec2 p = get_vertex(tri_ins_point_index[index]);
			SymEdge curr_insertion_symedge = get_symedge(tri_symedges[index].x);

			for (int i = 0; i < 3; i++)
			{
				if (point_intersects_line(p, get_vertex(curr_insertion_symedge.vertex), get_vertex(nxt(curr_insertion_symedge).vertex)))
					break;
				curr_insertion_symedge = nxt(curr_insertion_symedge);
			}

			// Check adjacent triangles for if they want to insert their point into one of their edges
			int adjacent_face_index = -1;

			for (int adjacent_triangle = 0; adjacent_triangle < 3; adjacent_triangle++)
			{
				if (adjacent_tri_point_intersects_edge(curr_insertion_symedge, adjacent_face_index) && index > adjacent_face_index)
				{
					tri_ins_point_index[index] = -1;
					return;
				}
				curr_insertion_symedge = nxt(curr_insertion_symedge);
			}

			// check other side
			if (nxt(curr_insertion_symedge).rot != -1)
			{
				curr_insertion_symedge = nxt(sym(curr_insertion_symedge));

				for (int adjacent_triangle = 0; adjacent_triangle < 2; adjacent_triangle++)
				{
					if (adjacent_tri_point_intersects_edge(curr_insertion_symedge, adjacent_face_index) && index > adjacent_face_index)
					{
						tri_ins_point_index[index] = -1;
						return;
					}
					curr_insertion_symedge = nxt(curr_insertion_symedge);
				}
			}
		}
		index += num_threads;
	}
}
