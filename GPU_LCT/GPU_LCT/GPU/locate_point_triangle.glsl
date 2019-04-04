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

vec2 get_face_center(int face_i)
{
	vec2 face_v[3];
	face_v[0] = point_positions[sym_edges[tri_symedges[face_i].x].vertex];
	face_v[1] = point_positions[sym_edges[sym_edges[tri_symedges[face_i].x].nxt].vertex];
	face_v[2] = point_positions[sym_edges[sym_edges[sym_edges[tri_symedges[face_i].x].nxt].nxt].vertex];

	return (face_v[0] + face_v[1] + face_v[2]) / 3.f; 
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
	float dist = length(project_point_on_line(p, a, b) - p);
	return abs(dist) < epsilon;
}

bool orientation(in vec2 p1 , in vec2 p2 , in vec2 p3)
{
	float val = (p2.y - p1.y) * (p3.x - p2.x) - (p2.x - p1.x) * (p3.y - p2.y);
	return (val > 0.0f) ? true : false;
}

bool line_line_test(in vec2 s1p1 , in vec2 s1p2, in vec2 s2p1, in vec2 s2p2)
{
	bool hit = false;
	bool o1 = orientation(s1p1, s1p2, s2p1);
	bool o2 = orientation(s1p1, s1p2, s2p2);
	bool o3 = orientation(s2p1, s2p2, s1p1);
	bool o4 = orientation(s2p1, s2p2, s1p2);

	if (o1 != o2 && o3 != o4)
		hit = true;
	else
		hit = false;

	return hit;
}

//-----------------------------------------------------------
// Functions
//-----------------------------------------------------------

bool face_edges_contains_vertice(int face, int vertex)
{
	SymEdge s = get_symedge(tri_symedges[face].x);
	vec2 p = get_vertex(vertex);

	vec2 t0 = get_vertex(s.vertex);
	vec2 t1 = get_vertex(nxt(s).vertex);
	vec2 t2 = get_vertex(prev(s).vertex);

	if (point_intersects_line(p, t0, t1) || point_intersects_line(p, t1, t2) || point_intersects_line(p, t2, t0))
	{
		tri_ins_point_index[face] = vertex;
		return true;
	}

	return false;
}

void oriented_walk_point(int curr_e, int goal_point_i)
{
	vec2 tri_cent;
	vec2 goal_point = get_vertex(goal_point_i);
	int i = 0;
	while (i != 3)
	{
		if (face_edges_contains_vertice(get_symedge(curr_e).face, goal_point_i))
			return;
		
		tri_cent = get_face_center(sym_edges[curr_e].face);
		
		// Loop through edges to see if we should continue through the edge
		// to the neighbouring triangle 
		bool line_line_hit = false;
		for (i = 0; i < 3; i++)
		{
			if (sym_edges[sym_edges[curr_e].nxt].rot == -1)
			{
				curr_e = sym_edges[curr_e].nxt;
				continue;
			}

			line_line_hit = line_line_test(
				tri_cent,
				goal_point,
				point_positions[sym_edges[curr_e].vertex],
				point_positions[sym_edges[sym_edges[curr_e].nxt].vertex]);

			if (line_line_hit)
			{
				curr_e = sym_edges[sym_edges[curr_e].nxt].rot;
				break;
			}
			curr_e = sym_edges[curr_e].nxt;
		}
	}
	return;
}

// Each thread represents one point

void main(void)
{
	uint gid = gl_GlobalInvocationID.x;
	int num_threads = int(gl_NumWorkGroups.x * gl_WorkGroupSize.x);
	int index = int(gid);

	if (index < point_positions.length() && point_inserted[index] == 0)
		oriented_walk_point(0, index);
}
