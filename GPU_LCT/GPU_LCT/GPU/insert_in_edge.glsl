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

//-----------------------------------------------------------
// Access Functions
//-----------------------------------------------------------
void get_face(in int face_i, out vec2 face_v[3])
{
	face_v[0] = point_positions[sym_edges[tri_symedges[face_i].x].vertex];
	face_v[1] = point_positions[sym_edges[sym_edges[tri_symedges[face_i].x].nxt].vertex];
	face_v[2] = point_positions[sym_edges[sym_edges[sym_edges[tri_symedges[face_i].x].nxt].nxt].vertex];
}

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

//-----------------------------------------------------------
// Intersection Functions
//-----------------------------------------------------------

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

// input is a point index from tri_ins_point_index

void main(void)
{
	uint gid = gl_GlobalInvocationID.x;
	int num_threads = int(gl_NumWorkGroups.x * gl_WorkGroupSize.x);
	int index = int(gid);

	if (index < tri_seg_inters_index.length() && tri_ins_point_index[index] >= 0)
	{
		int point_index = tri_ins_point_index[index];

		SymEdge segment = get_symedge(tri_symedges[index].x);

		for (int i = 0; i < 3; i++)
		{
			if (point_intersects_line(get_vertex(point_index), get_vertex(segment.vertex), get_vertex(nxt(segment).vertex)))
				break;
			segment = nxt(segment);
		}

		int e1 = get_index(nxt(segment));
		int e2 = get_index(prev(segment));

		int e1_sym = get_index(sym(get_symedge(e1)));
		int e2_sym = get_index(sym(get_symedge(e2)));
		
		ivec2 segment_symedges = ivec2(get_index(segment), nxt(segment).rot);
		int new_symedges[6];

		new_symedges[0] = 36 - 6 * (point_positions.length() - point_index);
		new_symedges[1] = 36 - 6 * (point_positions.length() - point_index) + 1;
		new_symedges[2] = 36 - 6 * (point_positions.length() - point_index) + 2;
		new_symedges[3] = 36 - 6 * (point_positions.length() - point_index) + 3;
		new_symedges[4] = 36 - 6 * (point_positions.length() - point_index) + 4;
		new_symedges[5] = 36 - 6 * (point_positions.length() - point_index) + 5;

		int t0 = get_symedge(segment_symedges[0]).face;
		int t1 = tri_seg_inters_index.length() - 2 * (point_positions.length() - point_index);
		int t2 = tri_seg_inters_index.length() - 2 * (point_positions.length() - point_index) + 1;
		int t3 = get_symedge(segment_symedges[1]).face;

		int edge1 = edge_label.length() - 3 * (point_positions.length() - point_index);
		int edge2 = edge_label.length() - 3 * (point_positions.length() - point_index) + 1;
		int edge3 = edge_label.length() - 3 * (point_positions.length() - point_index) + 2;

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
		tri_symedges[t1] = ivec4(new_symedges[1], e2, new_symedges[0],-1);

		int new_segment_index = seg_inserted.length() - (seg_inserted.length() - point_index);

		seg_endpoint_indices[edge_is_constrained[segment.edge]] = ivec2(point_index, get_symedge(e1).vertex);	// reused segment
		seg_endpoint_indices[new_segment_index] = ivec2(get_symedge(new_symedges[0]).vertex, point_index);		// new segment
		
		edge_is_constrained[edge1] = 1;

		// mark as maybe not non delauney
		edge_label[get_symedge(e1).edge] = 1;
		edge_label[get_symedge(e2).edge] = 1;

		if (nxt(segment).rot != -1)
		{
			int e3 = sym(segment).nxt;
			int e4 = nxt(sym(segment)).nxt;

			int e3_sym = get_index(sym(get_symedge(e3)));
			int e4_sym = get_index(sym(get_symedge(e4)));

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

			// mark as maybe not non delauney
			edge_label[get_symedge(e3).edge] = 1;
			edge_label[get_symedge(e4).edge] = 1;
		}
	}
}
