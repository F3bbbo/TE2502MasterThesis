#version 430
#define EPSILON 0.0001f
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


//-----------------------------------------------------------
// SymEdge funcitons
//-----------------------------------------------------------

vec2 get_vertex(int index)
{
	return point_positions[index];
}

SymEdge get_symedge(int index)
{
	return sym_edges[index];
}

int nxt(int edge)
{
	return sym_edges[edge].nxt;
}

SymEdge nxt(SymEdge s)
{
	return get_symedge(s.nxt);
}

int rot(int edge)
{
	return sym_edges[edge].rot;
}

SymEdge rot(SymEdge s)
{
	return get_symedge(s.rot);
}

int sym(int edge)
{
	return rot(nxt(edge));
}

SymEdge sym(SymEdge s)
{
	return rot(nxt(s));
}

int prev(int edge)
{
	return nxt(nxt(edge));
}

SymEdge prev(SymEdge s)
{
	return nxt(nxt(s));
}

int crot(int edge)
{
	int sym_i = sym(edge);
	return (sym_i != -1) ? nxt(sym_i) : -1;
}

//-----------------------------------------------------------
// Math Functions
//-----------------------------------------------------------


//-----------------------------------------------------------
// Intersection Functions
//-----------------------------------------------------------
vec2 project_point_on_line(vec2 point, vec2 a, vec2 b)
{
	vec2 ab = normalize(b - a);
	vec2 ap = point - a;
	return a + dot(ap, ab) * ab;
}

bool point_ray_test(vec2 p1, vec2 r1, vec2 r2, float epsi = EPSILON)
{
	vec2 dist_vec = project_point_on_line(p1, r1, r2);
	return abs(distance(dist_vec, p1)) < epsi ? true : false;
}


bool point_line_test(in vec2 p1, in vec2 s1, in vec2 s2, float epsi = EPSILON)
{
	vec2 dist_vec = project_point_on_line(p1, s1, s2);
	if (!point_ray_test(p1, s1, s2, epsi))
		return false;
	vec2 v1 = s1 - p1;
	vec2 v2 = s1 - s2;
	float dot_p = dot(v1, v2);
	if (dot_p < epsi * epsi)
		return false;
	if (dot_p > (dot(v2, v2) - epsi * epsi))
		return false;

	return true;
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

	while (index < tri_seg_inters_index.length())
	{
		if (tri_ins_point_index[index] >= 0)
		{
			status = 1;
			int point_index = tri_ins_point_index[index];
			point_inserted[point_index] = 1;
			tri_ins_point_index[index] = -1;

			int segment = tri_symedges[index].x;

			for (int i = 0; i < 2; i++)
			{
				if (point_line_test(get_vertex(point_index),
					get_vertex(get_symedge(segment).vertex),
					get_vertex(get_symedge(nxt(segment)).vertex)))
					break;
				segment = nxt(segment);
			}

			int e1 = nxt(segment);
			int e2 = prev(segment);

			int e1_sym = sym(e1);
			int e2_sym = sym(e2);

			ivec2 segment_symedges = ivec2(segment, rot(nxt(segment)));
			int new_symedges[6];

			new_symedges[0] = symedge_buffer_size - 6 * (point_positions.length() - point_index);
			new_symedges[1] = symedge_buffer_size - 6 * (point_positions.length() - point_index) + 1;
			new_symedges[2] = symedge_buffer_size - 6 * (point_positions.length() - point_index) + 2;
			new_symedges[3] = symedge_buffer_size - 6 * (point_positions.length() - point_index) + 3;
			new_symedges[4] = symedge_buffer_size - 6 * (point_positions.length() - point_index) + 4;
			new_symedges[5] = symedge_buffer_size - 6 * (point_positions.length() - point_index) + 5;

			int t0 = get_symedge(segment_symedges[0]).face;
			int t1 = tri_seg_inters_index.length() - 2 * (point_positions.length() - point_index);
			int t2 = tri_seg_inters_index.length() - 2 * (point_positions.length() - point_index) + 1;
			int t3 = get_symedge(segment_symedges[1]).face;

			int edge1 = edge_label.length() - 3 * (point_positions.length() - point_index);
			int edge2 = edge_label.length() - 3 * (point_positions.length() - point_index) + 1;
			int edge3 = edge_label.length() - 3 * (point_positions.length() - point_index) + 2;

			edge_label[edge1] = 0;
			edge_label[edge2] = 0;
			edge_label[edge3] = 0;

			// e1
			sym_edges[e1].nxt = new_symedges[2];
			sym_edges[e1].rot = segment_symedges[1];

			// e2
			sym_edges[e2].nxt = new_symedges[0];
			sym_edges[e2].rot = new_symedges[2];
			sym_edges[e2].face = t1;

			// new 0
			sym_edges[new_symedges[0]].nxt = new_symedges[1];
			sym_edges[new_symedges[0]].rot = e2_sym;
			sym_edges[new_symedges[0]].vertex = get_symedge(segment_symedges[0]).vertex;
			sym_edges[new_symedges[0]].edge = edge1;
			sym_edges[new_symedges[0]].face = t1;

			// new 1
			sym_edges[new_symedges[1]].nxt = e2;
			sym_edges[new_symedges[1]].rot = -1; // this will be fixed later if sym(seg) is not null
			sym_edges[new_symedges[1]].vertex = point_index;
			sym_edges[new_symedges[1]].edge = edge2;
			sym_edges[new_symedges[1]].face = t1;

			// new 2
			sym_edges[new_symedges[2]].nxt = segment_symedges[0];
			sym_edges[new_symedges[2]].rot = e1_sym;
			sym_edges[new_symedges[2]].vertex = get_symedge(e2).vertex;
			sym_edges[new_symedges[2]].edge = edge2;
			sym_edges[new_symedges[2]].face = t0;

			// old segment 0
			sym_edges[segment_symedges[0]].nxt = e1;
			sym_edges[segment_symedges[0]].rot = new_symedges[1];
			sym_edges[segment_symedges[0]].vertex = point_index;

			tri_symedges[t0] = ivec4(segment_symedges[0], e1, new_symedges[2], -1);
			tri_symedges[t1] = ivec4(new_symedges[1], e2, new_symedges[0], -1);

			int new_segment_index = seg_inserted.length() - (seg_inserted.length() - point_index);

			seg_endpoint_indices[edge_is_constrained[get_symedge(segment).edge]] = ivec2(point_index, get_symedge(e1).vertex);	// reused segment
			seg_endpoint_indices[new_segment_index] = ivec2(get_symedge(new_symedges[0]).vertex, point_index);		// new segment

			edge_is_constrained[edge1] = new_segment_index;

			// mark as maybe non delauney

			edge_label[get_symedge(e1).edge] = edge_is_constrained[get_symedge(e1).edge] == -1 ? 1 : edge_label[get_symedge(e1).edge];
			edge_label[get_symedge(e2).edge] = edge_is_constrained[get_symedge(e2).edge] == -1 ? 1 : edge_label[get_symedge(e2).edge];

			if (rot(nxt(segment)) != -1)
			{
				int e3 = nxt(sym(segment));
				int e4 = nxt(nxt(sym(segment)));

				int e3_sym = sym(e3);
				int e4_sym = sym(e4);

				// new 1 fix
				sym_edges[new_symedges[1]].rot = new_symedges[5];

				// e3
				sym_edges[e3].nxt = new_symedges[4];
				sym_edges[e3].rot = new_symedges[0];
				sym_edges[e3].face = t2;

				// e4
				sym_edges[e4].nxt = segment_symedges[1];
				sym_edges[e4].rot = new_symedges[4];

				// new 3
				sym_edges[new_symedges[3]].nxt = e4;
				sym_edges[new_symedges[3]].rot = segment_symedges[0];
				sym_edges[new_symedges[3]].vertex = point_index;
				sym_edges[new_symedges[3]].edge = edge3;
				sym_edges[new_symedges[3]].face = t3;

				// new 4
				sym_edges[new_symedges[4]].nxt = new_symedges[5];
				sym_edges[new_symedges[4]].rot = e3_sym;
				sym_edges[new_symedges[4]].vertex = get_symedge(e4).vertex;
				sym_edges[new_symedges[4]].edge = edge3;
				sym_edges[new_symedges[4]].face = t2;

				// new 5
				sym_edges[new_symedges[5]].nxt = e3;
				sym_edges[new_symedges[5]].rot = new_symedges[3];
				sym_edges[new_symedges[5]].vertex = point_index;
				sym_edges[new_symedges[5]].edge = edge1;
				sym_edges[new_symedges[5]].face = t2;

				// old segment 1
				sym_edges[segment_symedges[1]].nxt = new_symedges[3];
				sym_edges[segment_symedges[1]].rot = e4_sym;

				tri_symedges[t2] = ivec4(new_symedges[5], e3, new_symedges[4], -1);
				tri_symedges[t3] = ivec4(new_symedges[3], e4, segment_symedges[1], -1);

				seg_inserted[new_segment_index] = 1;

				// mark as maybe non delauney
				edge_label[get_symedge(e3).edge] = edge_is_constrained[get_symedge(e3).edge] == -1 ? 1 : edge_label[get_symedge(e3).edge];
				edge_label[get_symedge(e4).edge] = edge_is_constrained[get_symedge(e4).edge] == -1 ? 1 : edge_label[get_symedge(e4).edge];
			}
		}
		index += num_threads;
	}
}
