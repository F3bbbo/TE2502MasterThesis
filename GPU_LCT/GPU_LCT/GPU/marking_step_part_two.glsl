#version 430

layout(local_size_x = 1, local_size_y = 1) in;

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

layout(std140, binding = 0) buffer Point_position
{
	vec2 point_positions[];
};

layout(std140, binding = 1) buffer Points1
{
	int point_inserted[];
};

layout(std140, binding = 2) buffer Points2
{
	int point_tri_index[];
};

layout(std140, binding = 3) buffer Edge0
{
	int edge_label[];
};

layout(std140, binding = 4) buffer Edge1
{
	int edge_is_constrained[];
};

layout(std140, binding = 5) buffer Seg0
{
	int seg_endpoint_indices[];
};

layout(std140, binding = 6) buffer Seg1
{
	int seg_inserted[];
};

layout(std140, binding = 7) buffer Tri_buff_0
{
	ivec3 tri_vertex_indices[];
};
layout(std140, binding = 8) buffer Tri_buff_1
{
	ivec3 tri_symedges[];
};
layout(std140, binding = 9) buffer Tri_buff_2
{
	int tri_ins_point_index[];
};
layout(std140, binding = 10) buffer Tri_buff_3
{
	int tri_seg_inters_index[];
};
layout(std140, binding = 11) buffer Tri_buff_4
{
	int tri_edge_flip_index[];
};
layout(std140, binding = 12) buffer symedge_buff
{
	SymEdge sym_edges[];
};

//-----------------------------------------------------------
// Access funcitons
//-----------------------------------------------------------

SymEdge get_symedge(int index)
{
	return sym_edges[index];
}

SymEdge prev_symedge(SymEdge s)
{
	return get_symedge(get_symedge(s.nxt).nxt);
}

vec2 get_vertex(int index)
{
	return point_positions[index];
}

void get_face(in int face_i, out vec2 face_v[3])
{
	face_v[0] = point_positions[sym_edges[tri_symedges[face_i].x].vertex];
	face_v[1] = point_positions[sym_edges[sym_edges[tri_symedges[face_i].x].nxt].vertex];
	face_v[2] = point_positions[sym_edges[sym_edges[sym_edges[tri_symedges[face_i].x].nxt].nxt].vertex];
}

//-----------------------------------------------------------
// Uniforms
//-----------------------------------------------------------

layout (std140, binding = 0) uniform Sizes
{
	int num_tris;
	int num_points;
};

//-----------------------------------------------------------
// Functions
//-----------------------------------------------------------

bool is_delaunay(SymEdge sym)
{
	int index = get_symedge(sym.nxt).rot;
	if (index != -1)
		{
			mat4x4 mat;

			vec2 face_vertices[3];
			get_face(sym.face, face_vertices);

			for (int i = 0; i < 3; i++)
			{
				mat[0][i] = face_vertices[i].x;
				mat[1][i] = face_vertices[i].y;
				mat[2][i] = mat[0][i] * mat[0][i] + mat[1][i] * mat[1][i];
				mat[3][i] = 1.f;
			}

			vec2 other = get_vertex(prev_symedge(get_symedge(index)).vertex);
			mat[0][3] = other.x;
			mat[1][3] = other.y;
			mat[2][3] = mat[0][3] * mat[0][3] + mat[1][3] * mat[1][3];
			mat[3][3] = 1.f;

			if (determinant(mat) > 0)
				return false;
		}
		return true;
}

void main(void)
{
	// Each thread is responsible for a triangle

	uint gid = gl_GlobalInvocationID.x;
	int index = int(gid);
	
	if (index < num_tris)
	{
		SymEdge tri_sym = get_symedge(tri_symedges[index].x);
		bool no_point_in_edges = edge_label[tri_sym.edge] != 3 &&
									edge_label[get_symedge(tri_sym.nxt).edge] != 3 &&
									edge_label[prev_symedge(tri_sym).edge] != 3;
		if (no_point_in_edges)
		{
			if (tri_seg_inters_index[index] == -1)
			{
				for (int i = 0; i < 3; i++)
				{
					if (edge_label[tri_sym.edge] == 1 && is_delaunay(tri_sym))
						edge_label[tri_sym.edge] == 0;
					tri_sym = get_symedge(tri_sym.nxt);
				}
			}
			else
			{
				for (int i = 0; i < 3; i++)
				{
					int sym_index = get_symedge(tri_sym.nxt).rot;
					if (sym_index != -1)
					{
						if (edge_label[tri_sym.edge] == 2)
							tri_seg_inters_index[get_symedge(sym_index).face] == tri_seg_inters_index[index];
						else
							edge_label[tri_sym.edge] = 0;
					}
				}
			}
		}
	}
}