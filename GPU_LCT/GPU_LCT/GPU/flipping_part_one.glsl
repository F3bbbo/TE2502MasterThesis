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
layout (std140, binding = 1) uniform epsilon_buff
{
	float EPSILON;
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


// Each thread represents one triangle
void main(void)
{
	uint gid = gl_GlobalInvocationID.x;
	int num_threads = int(gl_NumWorkGroups.x * gl_WorkGroupSize.x);
	int index = int(gid);
	while (index < tri_seg_inters_index.length())
	{	
		if (tri_symedges[index].x > -1)
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

			if (h > 0)
			{
				// calculate nh to know how many other edges the that can be tested
				int nh = 0;
				for (int i = 0; i < 3; i++)
				{
					nh = h == edge_label[edge_sym.edge] ? nh + 1 : nh;
					edge_sym = nxt(edge_sym);
				}
				int num_iter = nh;
				while (num_iter > 0)
				{
					int sym_symedge = nxt(get_symedge(highest_priority_s_edge)).rot;
					if (sym_symedge != -1)
					{
						int o_label1 = edge_label[nxt(get_symedge(sym_symedge)).edge];
						int o_label2 = edge_label[prev_symedge(get_symedge(sym_symedge)).edge];

						if (o_label1 != h && o_label2 != h && o_label1 < h && o_label2 < h)
						{
							if (nh >= 2 || (nh == 1 && index < get_symedge(sym_symedge).face))
								tri_edge_flip_index[index] = get_symedge(highest_priority_s_edge).edge;
							else
								tri_edge_flip_index[index] = -1;
						}
					}
					// find next edge with label h
					edge_sym = sym_edges[highest_priority_s_edge];
					for (int i = 0; i < 3; i++)
					{
						edge_sym = nxt(edge_sym);
						if (h == edge_label[edge_sym.edge])
						{
							highest_priority_s_edge = get_index(edge_sym);
							break;
						}
					}
					num_iter--;
				}
			}
		}
		index += num_threads;
	}
}