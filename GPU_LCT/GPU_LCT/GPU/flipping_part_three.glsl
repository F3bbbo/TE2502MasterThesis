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
// Access Functions
//-----------------------------------------------------------

SymEdge get_symedge(int index)
{
	return sym_edges[index];
}

SymEdge nxt(SymEdge s)
{
	return get_symedge(s.nxt);
}

SymEdge prev(SymEdge s)
{
	return nxt(nxt(s));
}

SymEdge rot(SymEdge s)
{
	return get_symedge(s.rot);
}

SymEdge sym(SymEdge s)
{
	return rot(nxt(s));
}

int get_index(SymEdge s)
{
	return prev(s).nxt;
}

int get_vertex(int sym_edge_i)
{
	return get_symedge(sym_edge_i).vertex;
}

//-----------------------------------------------------------
// Uniforms
//-----------------------------------------------------------

//-----------------------------------------------------------
// Functions
//-----------------------------------------------------------

void flip_edge(SymEdge edge)
{
	// flips clockwise according to figure 8 in the paper.

	int t_prim = edge.face;
	int t = sym(edge).face;

	int curr = get_index(edge);
	int curr_sym = get_index(sym(edge));

	int e1 = edge.nxt;
	int e2 = nxt(edge).nxt;
	int e3 = sym(edge).nxt;
	int e4 = nxt(sym(edge)).nxt;

	int e1_sym = nxt(get_symedge(e1)).rot;
	int e2_sym = nxt(get_symedge(e2)).rot;
	int e3_sym = nxt(get_symedge(e3)).rot;
	int e4_sym = nxt(get_symedge(e4)).rot;

	//e1
	sym_edges[e1].nxt = curr;
	sym_edges[e1].rot = e4_sym;

	sym_edges[e1].face = t_prim;

	//e2
	sym_edges[e2].nxt = e3;
	sym_edges[e2].rot = curr;

	sym_edges[e2].face = t;
	
	//e3
	sym_edges[e3].nxt = curr_sym;
	sym_edges[e3].rot = e2_sym;

	sym_edges[e3].face = t;
	
	//e4
	sym_edges[e4].nxt = e1;
	sym_edges[e4].rot = curr_sym;

	sym_edges[e4].face = t_prim;

	// curr
	sym_edges[curr].nxt = e4;
	sym_edges[curr].rot = e1_sym;

	sym_edges[curr].vertex = get_symedge(e2).vertex;
	sym_edges[curr].face = t_prim;

	// curr_sym
	sym_edges[curr_sym].nxt = e2;
	sym_edges[curr_sym].rot = e3_sym;

	sym_edges[curr_sym].vertex = get_symedge(e4).vertex;
	sym_edges[curr_sym].face = t;

	// update face symedges
	tri_symedges[t_prim] = ivec4(curr, e4, e1, -1);
	tri_symedges[t] = ivec4(curr_sym, e2, e3, -1);

	// reset
	tri_seg_inters_index[t_prim] = -1;
	tri_seg_inters_index[t] = -1;
}

// Each thread represents one triangle
void main(void)
{
	int index = int(gl_GlobalInvocationID.x);
	if (index < tri_seg_inters_index.length() && tri_edge_flip_index[index] != -1 && edge_label[tri_edge_flip_index[index]] != -1)
	{
		status = 1;

		// find the symedge that constains the edge that should get flipped
		SymEdge edge_to_be_flipped = get_symedge(tri_symedges[index].x);
		SymEdge cur_edge = edge_to_be_flipped;
		for (int i = 0; i < 3; i++)
		{
			edge_to_be_flipped = cur_edge.edge == tri_edge_flip_index[index] ? cur_edge : edge_to_be_flipped;
			cur_edge = nxt(cur_edge);
		}

		edge_label[edge_to_be_flipped.edge] = 0;
		tri_edge_flip_index[index] = -1;
		flip_edge(edge_to_be_flipped);
	}
}