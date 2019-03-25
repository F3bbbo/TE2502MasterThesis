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

SymEdge sym_symedge(SymEdge s)
{
	return get_symedge(get_symedge(s.nxt).rot);
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
	vec2 pad;
};

//-----------------------------------------------------------
// Math Functions
//-----------------------------------------------------------

float sign(vec2 p1, vec2 p2, vec2 p3)
{
	return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

bool point_triangle_test(vec2 p1, vec2 t1, vec2 t2, vec2 t3)
{
	float d1, d2, d3;
	bool has_neg, has_pos;

	d1 = sign(p1, t1, t2);
	d2 = sign(p1, t2, t3);
	d3 = sign(p1, t3, t1);

	has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
	has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

	return !(has_neg && has_pos);
}

#define COLINEAR 0
#define CLOCKWISE 1
#define COUNTER_CLOCKWISE 2

int orientation(vec2 p1, vec2 p2, vec2 p3)
{
	float val = (p2.y - p1.y) * (p3.x - p2.x) - (p2.x - p1.x) * (p3.y - p2.y);

	int ret_val = val == 0.f ? COLINEAR : -1;
	ret_val = val > 0.f ? CLOCKWISE : COUNTER_CLOCKWISE;
	return ret_val;
}

bool line_seg_intersection_ccw(vec2 p1, vec2 q1, vec2 p2, vec2 q2)
{
	int o1 = orientation(p1, q1, p2);
	int o2 = orientation(p1, q1, q2);
	int o3 = orientation(p2, q2, p1);
	int o4 = orientation(p2, q2, q1);

	return o1 != o2 && o3 != o4 ? true : false;
}

//-----------------------------------------------------------
// Intersection Functions
//-----------------------------------------------------------

bool segment_triangle_test(vec2 p1, vec2 p2, vec2 t1, vec2 t2, vec2 t3)
{
	bool test_one, test_two = false;

	test_one = point_triangle_test(p1, t1, t2, t3) && point_triangle_test(p2, t1, t2, t3);
		
	test_two = line_seg_intersection_ccw(p1, p2, t1, t2) ||
		line_seg_intersection_ccw(p1, p2, t2, t3) ||
		line_seg_intersection_ccw(p1, p2, t3, t1);

	return test_one || test_two;
}

//-----------------------------------------------------------
// Functions
//-----------------------------------------------------------

bool is_delaunay(SymEdge sym)
{
	int index = get_symedge(sym.nxt).rot;
	if (index != -1)
	{
		vec2 point1 = get_vertex(prev_symedge(sym).vertex);
		vec2 point2 = get_vertex(prev_symedge(sym_symedge(sym)).vertex);
		vec2 center = (point1 + point2) / 2.f;

		float len = length(get_vertex(sym.vertex) - get_vertex(sym_symedge(sym).vertex)) / 2.f;
		if (length(center - point1) <= len && length(center - point2) <= len)
			return true;
	}
	return true;

//	int index = get_symedge(sym.nxt).rot;
//	if (index != -1)
//	{
//		mat4x4 mat;
//
//		vec2 face_vertices[3];
//		get_face(sym.face, face_vertices);
//
//		for (int i = 0; i < 3; i++)
//		{
//			mat[0][i] = face_vertices[i].x;
//			mat[1][i] = face_vertices[i].y;
//			mat[2][i] = mat[0][i] * mat[0][i] + mat[1][i] * mat[1][i];
//			mat[3][i] = 1.f;
//		}
//
//		vec2 other = get_vertex(prev_symedge(get_symedge(index)).vertex);
//		mat[0][3] = other.x;
//		mat[1][3] = other.y;
//		mat[2][3] = mat[0][3] * mat[0][3] + mat[1][3] * mat[1][3];
//		mat[3][3] = 1.f;
//
//		if (determinant(mat) > 0)
//			return false;
//	}
//	return true;
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
						edge_label[tri_sym.edge] = 0;
					tri_sym = get_symedge(tri_sym.nxt);
				}
			}
			else
			{
				for (int i = 0; i < 3; i++)
				{
					int adjacent_triangle = get_symedge(tri_sym.nxt).rot;
					if (adjacent_triangle != -1 && edge_label[tri_sym.edge] == 2)
					{
						vec2 segment_vertices[2];
						segment_vertices[0] = get_vertex(seg_endpoint_indices[tri_seg_inters_index[index]].x);
						segment_vertices[1] = get_vertex(seg_endpoint_indices[tri_seg_inters_index[index]].y);

						vec2 face_vertices[3];
						get_face(sym_symedge(tri_sym).face, face_vertices);

						if (!segment_triangle_test(segment_vertices[0], segment_vertices[1], face_vertices[0], face_vertices[1], face_vertices[2]))
							edge_label[tri_sym.edge] = 0;
					}
					tri_sym = get_symedge(tri_sym.nxt);
				}
			}
		}
	}
}