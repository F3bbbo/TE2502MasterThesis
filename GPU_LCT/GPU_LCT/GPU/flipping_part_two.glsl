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

//-----------------------------------------------------------
// Uniforms
//-----------------------------------------------------------

//-----------------------------------------------------------
// Functions
//-----------------------------------------------------------

void set_quad_edges_label(int label, SymEdge edge)
{
	edge_label[nxt(edge).edge] = edge_is_constrained[nxt(edge).edge] == -1 ? max(label, edge_label[nxt(edge).edge]) : edge_label[nxt(edge).edge];
	edge_label[prev(edge).edge] = edge_is_constrained[prev(edge).edge] == -1 ? max(label, edge_label[prev(edge).edge]) : edge_label[prev(edge).edge];

	edge = sym(edge);

	edge_label[nxt(edge).edge] = edge_is_constrained[nxt(edge).edge] == -1 ? max(label, edge_label[nxt(edge).edge]) : edge_label[nxt(edge).edge];
	edge_label[prev(edge).edge] = edge_is_constrained[prev(edge).edge] == -1 ? max(label, edge_label[prev(edge).edge]) : edge_label[prev(edge).edge];
}

// Each thread represents one triangle
void main(void)
{
	int index = int(gl_GlobalInvocationID.x);
	int num_threads = int(gl_NumWorkGroups.x * gl_WorkGroupSize.x);
	while (index < tri_seg_inters_index.length())
	{	
		if (tri_symedges[index].x > -1)
			{
				if (tri_edge_flip_index[index] == -1)
				{
					SymEdge edge_sym = get_symedge(tri_symedges[index].x);
					for (int i = 0; i < 3; i++)
					{
						int sym = nxt(edge_sym).rot;
						if (sym != -1 && tri_edge_flip_index[get_symedge(sym).face] == edge_sym.edge)
						{
							// This feels like bullshit
							// tri_edge_flip_index[index] = edge_label[edge_sym.edge];
							set_quad_edges_label(1, edge_sym);
							break;
						}
						edge_sym = nxt(edge_sym);
					}
				}
			}
		index += num_threads;
	}
}