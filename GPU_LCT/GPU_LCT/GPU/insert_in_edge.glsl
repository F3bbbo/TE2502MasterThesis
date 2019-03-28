#version 430
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

// Each thread represents one segment
void main(void)
{
	uint gid = gl_GlobalInvocationID.x;
	int num_threads = int(gl_NumWorkGroups.x * gl_WorkGroupSize.x);
	int index = int(gid);

	int point_index = 8;
	int segment_index = 9;
	SymEdge segment = get_symedge(22);

//	if (index < tri_seg_inters_index.length())
	if (index == 0)
	{
		int e1 = get_index(nxt(segment));
		int e2 = get_index(prev(segment));
		int e3 = get_index(nxt(sym(segment)));
		int e4 = get_index(prev(sym(segment)));

		int e1_sym = get_index(sym(get_symedge(e1)));
		int e2_sym = get_index(sym(get_symedge(e2)));
		int e3_sym = get_index(sym(get_symedge(e3)));
		int e4_sym = get_index(sym(get_symedge(e4)));
		
		ivec2 segment_symedges = ivec2(get_index(segment), get_index(sym(segment)));
		int new_symedges[6];

		new_symedges[0] = 30;
		new_symedges[1] = 31;
		new_symedges[2] = 32;
		new_symedges[3] = 33;
		new_symedges[4] = 34;
		new_symedges[5] = 35;

		int t0 = get_symedge(segment_symedges[0]).face;
		int t1 = get_symedge(segment_symedges[1]).face;
		int t2 = 10; // ???
		int t3 = 11; // ???

		int edge1 = 17;
		int edge2 = 18;
		int edge3 = 19;

		// e1
		sym_edges[e1].nxt = new_symedges[1];
		sym_edges[e1].rot = new_symedges[3];
		sym_edges[e1].face = t2;

		// e2
		sym_edges[e2].rot = new_symedges[1];

		// e3
		sym_edges[e3].nxt = new_symedges[5];

		// e4
		sym_edges[e4].nxt = new_symedges[3];
		sym_edges[e4].rot = new_symedges[5];
		sym_edges[e4].face = t3;

		// new 0
		sym_edges[new_symedges[0]].nxt = e2;
		sym_edges[new_symedges[0]].rot = segment_symedges[1];
		sym_edges[new_symedges[0]].vertex = point_index;
		sym_edges[new_symedges[0]].edge = edge1;
		sym_edges[new_symedges[0]].face = t0;

		// new 1
		sym_edges[new_symedges[1]].nxt = new_symedges[2];
		sym_edges[new_symedges[1]].rot = e1_sym;
		sym_edges[new_symedges[1]].vertex = get_symedge(e2).vertex;
		sym_edges[new_symedges[1]].edge = edge1;
		sym_edges[new_symedges[1]].face = t2;

		// new 2
		sym_edges[new_symedges[2]].nxt = e1;
		sym_edges[new_symedges[2]].rot = new_symedges[0];
		sym_edges[new_symedges[2]].vertex = point_index;
		sym_edges[new_symedges[2]].edge = edge2;
		sym_edges[new_symedges[2]].face = t2;

		// new 3
		sym_edges[new_symedges[3]].nxt = new_symedges[4];
		sym_edges[new_symedges[3]].rot = e4_sym;
		sym_edges[new_symedges[3]].vertex = get_symedge(e1).vertex;;
		sym_edges[new_symedges[3]].edge = edge2;
		sym_edges[new_symedges[3]].face = t3;

		// new 4
		sym_edges[new_symedges[4]].nxt = e4;
		sym_edges[new_symedges[4]].rot = new_symedges[2];
		sym_edges[new_symedges[4]].vertex = point_index;
		sym_edges[new_symedges[4]].edge = edge3;
		sym_edges[new_symedges[4]].face = t3;

		// new 5
		sym_edges[new_symedges[5]].nxt = segment_symedges[1];
		sym_edges[new_symedges[5]].rot = e3_sym;
		sym_edges[new_symedges[5]].vertex = get_symedge(e4).vertex;;
		sym_edges[new_symedges[5]].edge = edge3;
		sym_edges[new_symedges[5]].face = t1;

		// old segment 0
		sym_edges[segment_symedges[0]].nxt = new_symedges[0];

		// old segment 1
		sym_edges[segment_symedges[1]].rot = new_symedges[4];
		sym_edges[segment_symedges[1]].vertex = point_index;

		tri_symedges[t0] = ivec4(segment_symedges[0], new_symedges[0], e2, -1);
		tri_symedges[t1] = ivec4(segment_symedges[1], e3, new_symedges[5], -1);
		tri_symedges[t2] = ivec4(new_symedges[2], e1, new_symedges[1], -1);
		tri_symedges[t3] = ivec4(new_symedges[3], new_symedges[4], e4, -1);

		seg_endpoint_indices[segment_index] = ivec2(point_index, get_symedge(e2).vertex);
		seg_inserted[segment_index] = 1;

		edge_is_constrained[edge2] = 1;
	}

}
