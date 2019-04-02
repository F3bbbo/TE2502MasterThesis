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

//-----------------------------------------------------------
// Functions
//-----------------------------------------------------------

// Each thread represents one triangle

void main(void)
{
	uint gid = gl_GlobalInvocationID.x;
	int num_threads = int(gl_NumWorkGroups.x * gl_WorkGroupSize.x);
	int index = int(gid);

	if (index < tri_symedges.length() && tri_ins_point_index[index] != -1)
	{
		// Triangles can only insert a point if the neighbouring triangle does not have a point that it should insert
		SymEdge s = get_symedge(tri_symedges[index].x);
		vec2 p = get_vertex(tri_ins_point_index[index]);

		vec2 t0 = get_vertex(s.vertex);
		vec2 t1 = get_vertex(nxt(s).vertex);
		vec2 t2 = get_vertex(prev(s).vertex);
		
		if (point_intersects_line(p, t0, t1))
		{
			edge_is_constrained[s.edge] = get_index(s);
			tri_ins_point_index[index] = nxt(s).rot != -1 && tri_ins_point_index[rot(nxt(s)).face] != -1 ? -1 : tri_ins_point_index[index];
		}
		else if (point_intersects_line(p, t1, t2))
		{
			edge_is_constrained[nxt(s).edge] = get_index(nxt(s));
			tri_ins_point_index[index] = prev(s).rot != -1 && tri_ins_point_index[rot(prev(s)).face] != -1 ? -1 : tri_ins_point_index[index];
		}
		else if (point_intersects_line(p, t2, t0))
		{
			edge_is_constrained[prev(s).edge] = get_index(prev(s));
			tri_ins_point_index[index] = s.rot != -1 && tri_ins_point_index[rot(s).face] != -1 ? -1 : tri_ins_point_index[index];
		}
	}
}
