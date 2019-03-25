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

SymEdge prev_symedge(SymEdge s)
{
	return nxt(nxt(s));
}

int get_index(SymEdge s)
{
	return prev_symedge(s).nxt;
}

//-----------------------------------------------------------
// Uniforms
//-----------------------------------------------------------
layout (std140, binding = 0) uniform Sizes
{
	int num_tris;
	int num_points;
	vec2 pad;
};

// Each thread represents one triangle
void main(void)
{
	uint gid = gl_GlobalInvocationID.x;
	int num_threads = int(gl_NumWorkGroups.x * gl_WorkGroupSize.x);
	int index = int(gid);
	if (index < num_tris)
	{	
		int highest_priority_s_edge = -1;
		int h = -1;

		SymEdge edge_sym = get_symedge(tri_symedges[index].x);
		for (int i = 0; i < 3; i++)
		{
			highest_priority_s_edge = h < edge_label[edge_sym.edge] ? get_index(edge_sym) : highest_priority_s_edge;
			h = max(edge_label[edge_sym.edge], h);
			edge_sym = nxt(edge_sym);
		}

		int sym_symedge = nxt(get_symedge(highest_priority_s_edge)).rot;
		if (h > 0 && sym_symedge != -1)
		{
			int nh = 0;
			for (int i = 0; i < 3; i++)
			{
				nh = h == edge_label[edge_sym.edge] ? nh + 1 : nh;
				edge_sym = nxt(edge_sym);
			}

			if (nh >= 2 || (nh == 1 && index < get_symedge(sym_symedge).face))
				tri_edge_flip_index[index] = get_symedge(highest_priority_s_edge).edge;
			else
				tri_edge_flip_index[index] = -1;
		}
	}
}