#version 430
#define FLT_MAX 3.402823466e+38
precision highp float;
layout(local_size_x = 1, local_size_y = 1) in;

struct SymEdge{
	int nxt;
	int rot;

	int vertex;
	int edge;
	int face;
};
struct NewPoint
{
	vec2 pos;
	int index;
	int face_i;
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
layout(std430, binding = 13) buffer Tri_buff_4
{
	NewPoint tri_insert_points[];
};


//-----------------------------------------------------------
// Uniforms
//-----------------------------------------------------------
layout (std140, binding = 1) uniform epsilon_buff
{
	float EPSILON;
};

//-----------------------------------------------------------
// Access funcitons
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

void get_face(in int face_i, out vec2 face_v[3])
{
	face_v[0] = point_positions[sym_edges[tri_symedges[face_i].x].vertex];
	face_v[1] = point_positions[sym_edges[tri_symedges[face_i].y].vertex];
	face_v[2] = point_positions[sym_edges[tri_symedges[face_i].z].vertex];
}

//-----------------------------------------------------------
// Math Functions
//-----------------------------------------------------------


//-----------------------------------------------------------
// Intersection Functions
//-----------------------------------------------------------
float vec2_cross(in vec2 v, in vec2 w)
{
	return v.x*w.y - v.y*w.x;
}
bool line_line_test(vec2 p1, vec2 p2, vec2 q1, vec2 q2, float epsi = EPSILON)
{
	// solution found:
	//https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
	vec2 s = p2 - p1;
	vec2 r = q2 - q1;
	float rs = vec2_cross(s, r);
	vec2 qp = (q1 - p1);
	float qpr = vec2_cross(qp, r);
	if (abs(rs) < epsi && abs(qpr) < epsi) // case 1
	{
		float r2 = dot(r, r);
		float t0 = dot((q1 - p1), r) / r2;
		float sr = dot(s, r);
		float t1 = t0 + (sr / r2);
		if (sr < 0.0f)
		{
			float tmp = t0;
			t0 = t1;
			t1 = tmp;
		}
		if ((t0 < 0.0f && t1 < 0.0f) || t0 > 1.0f && t1 > 1.0f)
			return false;
		else
			return true;
	}
	else if (abs(rs) < epsi && !(abs(qpr) < epsi)) // case 2
	{
		return false;
	}
	else // case 3
	{
		float u = qpr / rs;
		float t = vec2_cross(qp, s) / rs;
		if ((0.0f - epsi) < u && u < (1.0f + epsi) && (0.0f - epsi) < t && t < (1.0f + epsi))
			return true;
	}
	return false;
}

float sign(vec2 p1, vec2 p2, vec2 p3)
{
	return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

bool point_triangle_test(vec2 p1, vec2 t1, vec2 t2, vec2 t3, float epsi = EPSILON)
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

int orientation(vec2 p1, vec2 p2, vec2 p3)
{
	float val = (p2.y - p1.y) * (p3.x - p2.x) - (p2.x - p1.x) * (p3.y - p2.y);

	if (val == 0.0f) return 0;
	return (val > 0.0f) ? 1 : 2; // Clockwise : Counter Clockwise
}

bool line_seg_intersection_ccw(vec2 p1, vec2 q1, vec2 p2, vec2 q2)
{
	int o1 = orientation(p1, q1, p2);
	int o2 = orientation(p1, q1, q2);
	int o3 = orientation(p2, q2, p1);
	int o4 = orientation(p2, q2, q1);

	if (o1 != o2 && o3 != o4)
		return true;

	return false;
}

bool segment_triangle_test(vec2 p1, vec2 p2, vec2 t1, vec2 t2, vec2 t3)
{
	if (point_triangle_test(p1, t1, t2, t3) && point_triangle_test(p2, t1, t2, t3)) {
		// If triangle contains both points of segment return true
		return true;
	}
	if (line_seg_intersection_ccw(p1, p2, t1, t2) ||
		line_seg_intersection_ccw(p1, p2, t2, t3) ||
		line_seg_intersection_ccw(p1, p2, t3, t1)) {
		// If segment intersects any of the edges of the triangle return true
		return true;
	}
	// Otherwise segment is missing the triangle
	return false;
}

vec2 line_line_intersection_point(vec2 a, vec2 b, vec2 c, vec2 d, out bool degenerate_triangle)
{
	// Line AB represented as a1x + b1y = c1 
	float a1 = b.y - a.y;
	float b1 = a.x - b.x;
	float c1 = a1 * a.x + b1 * a.y;

	// Line CD represented as a2x + b2y = c2 
	float a2 = d.y - c.y;
	float b2 = c.x - d.x;
	float c2 = a2 * (c.x) + b2 * (c.y);

	float det = a1 * b2 - a2 * b1;

	if (abs(det) < EPSILON)
	{
		// The lines are parallel
		degenerate_triangle = true;
		return vec2(FLT_MAX);
	}
	else
	{
		degenerate_triangle = false;
		return vec2((b2 * c1 - b1 * c2) / det, (a1 * c2 - a2 * c1) / det);
	}
}

//-----------------------------------------------------------
// Functions
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

bool is_flippable(int e)
{
	int e_sym = sym(e);
	if (e_sym > -1)
	{
		vec2 a = point_positions[sym_edges[e_sym].vertex];
		vec2 d = point_positions[sym_edges[prev(e_sym)].vertex];

		vec2 c = point_positions[sym_edges[e].vertex];
		vec2 b = point_positions[sym_edges[prev(e)].vertex];
		// first check so the new triangles will not be degenerate
		if (point_ray_test(a, d, b) || point_ray_test(c, d, b))
			return false;
		// then check so they will not overlap other triangles
		return line_line_test(a, c, b, d);
	}
	return false;
}

bool is_delaunay(int sym)
{
	int index = rot(nxt(sym));

	if (index != -1)
	{
		vec2 face_vertices[3];
		face_vertices[0] = get_vertex(get_symedge(sym).vertex);
		face_vertices[1] = get_vertex(get_symedge(nxt(sym)).vertex);
		face_vertices[2] = get_vertex(get_symedge(prev(sym)).vertex);

		vec2 ab = face_vertices[1] - face_vertices[0];
		vec2 bc = face_vertices[2] - face_vertices[1];

		vec2 mid_point1 = face_vertices[0] + ab / 2.f;
		vec2 mid_point2 = face_vertices[1] + bc / 2.f;

		// rotate vectors 90 degrees
		vec2 normal1 = vec2(-ab.y, ab.x);
		vec2 normal2 = vec2(-bc.y, bc.x);

		bool degenerate_triangle;
		vec2 circle_center = line_line_intersection_point(mid_point1, mid_point1 + normal1, mid_point2, mid_point2 + normal2, degenerate_triangle);
		if (degenerate_triangle == true)
			return false;

		vec2 other = get_vertex(get_symedge(prev(index)).vertex);
		if (length(circle_center - other) < length(face_vertices[0] - circle_center) - EPSILON)
			return false;

		//mat4x4 mat;

		//vec2 face_vertices[3];
		//face_vertices[2] = get_vertex(get_symedge(sym).vertex);
		//face_vertices[0] = get_vertex(get_symedge(nxt(sym)).vertex);
		//face_vertices[1] = get_vertex(get_symedge(prev(sym)).vertex);

		//for (int i = 0; i < 3; i++)
		//{
		//	mat[0][i] = face_vertices[i].x;
		//	mat[1][i] = face_vertices[i].y;
		//	mat[2][i] = mat[0][i] * mat[0][i] + mat[1][i] * mat[1][i];
		//	mat[3][i] = 1.f;
		//}

		//vec2 other = get_vertex(get_symedge(prev(index)).vertex);

		//mat[0][3] = other.x;
		//mat[1][3] = other.y;
		//mat[2][3] = mat[0][3] * mat[0][3] + mat[1][3] * mat[1][3];
		//mat[3][3] = 1.f;

		//if (determinant(mat) > 0)
		//	return false;
	}
	return true;

}


void main(void)
{
	// Each thread is responsible for a triangle

	uint gid = gl_GlobalInvocationID.x;
	int index = int(gid);
	int num_threads = int(gl_NumWorkGroups.x * gl_WorkGroupSize.x);
	
	while (index < tri_seg_inters_index.length())
	{
		int tri_sym = tri_symedges[index].x;
		if (tri_sym > -1)
		{
			bool no_point_in_edges = edge_label[get_symedge(tri_sym).edge] != 3 &&
				edge_label[get_symedge(nxt(tri_sym)).edge] != 3 &&
				edge_label[get_symedge(prev(tri_sym)).edge] != 3;
			// go through edges checking so edges can be flipped
			// TODO: move this to be done only if there is no edge with label 2 in the triangle
			// Problem: A problem would occur with the flipping when a triangle with a label 2 
			// would discard that label and then would not process the label 1 appropriately, 
			// so now label ones are processed all the time.
			for (int i = 0; i < 3; i++)
			{
				if (edge_label[get_symedge(tri_sym).edge] == 1 && ((!is_flippable(tri_sym) || is_delaunay(tri_sym)) || edge_is_constrained[get_symedge(tri_sym).edge] > -1))
					edge_label[get_symedge(tri_sym).edge] = 0;

				tri_sym = nxt(tri_sym);
			}
			if (no_point_in_edges)
			{
				if (tri_seg_inters_index[index] == -1)
				{
					for (int i = 0; i < 3; i++)
					{
						if (edge_label[get_symedge(tri_sym).edge] == 1 && ((!is_flippable(tri_sym) || is_delaunay(tri_sym)) || edge_is_constrained[get_symedge(tri_sym).edge] > -1))
							edge_label[get_symedge(tri_sym).edge] = 0;

						tri_sym = nxt(tri_sym);
					}
				}
				else
				{
					for (int i = 0; i < 3; i++)
					{
						int adjacent_triangle = get_symedge(nxt(tri_sym)).rot;
						if (adjacent_triangle != -1 && edge_label[get_symedge(tri_sym).edge] == 2)
						{
							vec2 segment_vertices[2];
							segment_vertices[0] = get_vertex(seg_endpoint_indices[tri_seg_inters_index[index]].x);
							segment_vertices[1] = get_vertex(seg_endpoint_indices[tri_seg_inters_index[index]].y);

							vec2 face_vertices[3];
							get_face(get_symedge(rot(nxt(tri_sym))).face, face_vertices);

							if (!segment_triangle_test(segment_vertices[0], segment_vertices[1], face_vertices[0], face_vertices[1], face_vertices[2]) || !is_flippable(tri_sym))
								edge_label[get_symedge(tri_sym).edge] = 0;
						}
						tri_sym = nxt(tri_sym);
					}
				}
			}
		}
		index += num_threads;
	}
}