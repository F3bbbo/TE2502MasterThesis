#version 430
#define FLT_MAX 3.402823466e+38
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

//-----------------------------------------------------------
// Access Functions
//-----------------------------------------------------------
void get_face(in int face_i, out vec2 face_v[3])
{
	face_v[0] = point_positions[sym_edges[tri_symedges[face_i].x].vertex];
	face_v[1] = point_positions[sym_edges[sym_edges[tri_symedges[face_i].x].nxt].vertex];
	face_v[2] = point_positions[sym_edges[sym_edges[sym_edges[tri_symedges[face_i].x].nxt].nxt].vertex];
}
int nxt(int edge)
{
	return sym_edges[edge].nxt;
}

int rot(int edge)
{
	return sym_edges[edge].rot;
}

int sym(int edge)
{
	return rot(nxt(edge));
}

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
bool valid_point_into_face(int face, vec2 p)
{
	int curr_e = tri_symedges[face].x;
	for (int i = 0; i < 3; i++)
	{
		SymEdge curr_sym = sym_edges[curr_e];
		vec2 s1 = point_positions[curr_sym.vertex];
		vec2 s2 = point_positions[sym_edges[nxt(curr_e)].vertex];
		if (point_line_test(p, s1, s2) && edge_label[curr_sym.edge] == 3)
		{
			int e_sym = sym(curr_e);
			if (e_sym > -1)
			{
				vec2 tri_points[3];
				get_face(sym_edges[e_sym].face, tri_points);
				if (point_ray_test(tri_points[0], tri_points[1], tri_points[2]))
				{
					return false;
				}
			}
		}
	}
	return true;
}

// Each thread represents one triangle
void main(void)
{
	uint gid = gl_GlobalInvocationID.x;
	int index = int(gid);
	int num_threads = int(gl_NumWorkGroups.x * gl_WorkGroupSize.x);
	while(index < tri_seg_inters_index.length())
	{
		if (tri_symedges[index].x != -1)
		{
			vec2 tri_points[3];
			get_face(index, tri_points);
			// check so the triangle is not a degenerate triangle
			if (!point_ray_test(tri_points[0], tri_points[1], tri_points[2]))
			{
				// calculate the centroid of the triangle
				vec2 tri_cent = (tri_points[0] + tri_points[1] + tri_points[2]) / 3.0f;
				int point_index = -1;
				float best_dist = FLT_MAX;
				// Figure out which point should be the new point of this triangle
				for (int i = 0; i < point_positions.length(); i++)
				{
					if (point_tri_index[i] == index)
					{
						// Check so it is an uninserted point.
						if (point_inserted[i] == 0)
						{
							vec2 pos = point_positions[i];
							float dist = distance(pos, tri_cent);
							if (dist < best_dist)
							{
								// Check so point is not on an edge with label 3
								if (valid_point_into_face(index, pos))
								{
									best_dist = dist;
									point_index = i;
								}
							}
						}
					}
				}
				tri_ins_point_index[index] = point_index;
			}
		}
		index += num_threads;
	}

}