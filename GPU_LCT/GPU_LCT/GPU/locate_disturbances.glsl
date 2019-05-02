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

struct NewPoint
{
	vec2 pos;
	int index;
	int face_i;
};

uint gid;
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
layout(std430, binding = 13) buffer ref_buff
{
	NewPoint refine_points[];
};
layout(std430, binding = 14) buffer new_points_buff
{
	vec2 new_points[];
};
//-----------------------------------------------------------
// Declare precision
//-----------------------------------------------------------

precision highp float;

//-----------------------------------------------------------
// Uniforms
//-----------------------------------------------------------

//-----------------------------------------------------------
// Math funcitons
//-----------------------------------------------------------

bool point_equal(vec2 p1, vec2 p2)
{
	return abs(distance(p1, p2)) < EPSILON;
}

// Returns the index of the point it equals in the triangle
int point_equal_tri_vert(in vec2 p, in vec2 tri[3])
{
	for(int i = 0; i < 3; i++)
	{
		if(point_equal(p, tri[i]))
			return i;
	}
	return -1;
}

vec2 project_point_on_line(vec2 point, vec2 a, vec2 b)
{
	vec2 ab = normalize(b - a);
	vec2 ap = point - a;
	return a + dot(ap, ab) * ab;
}

vec2 project_point_on_segment(vec2 point, vec2 a, vec2 b, out bool projectable)
{
	vec2 point_proj = project_point_on_line(point, a, b);
	vec2 line = b - a;
	float proj_len = dot(normalize(line), point_proj - a);
	projectable = true;
	if (proj_len < 0.0f)
	{
		projectable = false;
		point_proj = a;
	}
	else if (proj_len * proj_len > dot(line, line))
	{
		projectable = false;
		point_proj = b;
	}
	return point_proj;
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

bool point_triangle_test(vec2 p1, in vec2 tri[3], float epsi = EPSILON)
{
	return point_triangle_test(p1, tri[0], tri[1], tri[2], epsi);
}

vec2 line_line_intersection_point(vec2 a, vec2 b, vec2 c, vec2 d, float epsi = EPSILON)
{
	// Line AB represented as a1x + b1y = c1 
	float a1 = b.y - a.y;
	float b1 = a.x - b.x;
	float c1 = a1 * a.x + b1 * a.y;

	// Line CD represented as a2x + b2y = c2 
	float a2 = d.y - c.y;
	float b2 = c.x - d.x;
	float c2 = a2 * (c.x) + b2 * (c.y);

	float determinant = a1 * b2 - a2 * b1;

	if (abs(determinant) < epsi)
	{
		// The lines are parallel. This is simplified 
		// by returning a pair of FLT_MAX 
		return vec2(FLT_MAX, FLT_MAX);
	}

	float x = (b2 * c1 - b1 * c2) / determinant;
	float y = (a1 * c2 - a2 * c1) / determinant;
	return vec2( x, y );

}

bool edge_intersects_sector(vec2 a, vec2 b, vec2 c, vec2 segment[2])
{
	// Assumes that b is the origin of the sector
	vec2 center_prim = project_point_on_line(b, segment[0], segment[1]);
	float center_prim_length = length(center_prim - b);
	float sector_radius = min(length(a - b), length(c - b));

	if (length(a - b) < length(c - b))
		c = b + normalize(c - b) * sector_radius;
	else
		a = b + normalize(a - b) * sector_radius;

	vec2 tri[3] = { a, b, c };
	bool inside_triangle = point_triangle_test(center_prim, tri);
	bool inside_circle = center_prim_length <= sector_radius;
	vec2 point;
	point = line_line_intersection_point(b, center_prim, a, c, EPSILON);
	if (point_equal(point, vec2(FLT_MAX)))
		return false;

	bool other_side_of_ac = dot(point - b, center_prim - b) > 0 && center_prim_length > length(point - b);
	if (inside_triangle || (inside_circle && other_side_of_ac))
		return true;
	return false;
}

vec2 get_symmetrical_corner(vec2 a, vec2 b, vec2 c)
{
	vec2 ac = c - a;
	vec2 half_ac = a + (ac / 2.f);
	float len = length(half_ac - project_point_on_line(b, a, c));
	return b + 2.f * len * normalize(ac);
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

vec2 circle_center_from_points(vec2 a, vec2 b, vec2 c)
{
	vec2 ab = b - a;
	vec2 bc = c - b;

	vec2 midpoints[2] = { a + ab / 2.f, b + bc / 2.f };
	vec2 normals[2];
	// rotate vectors 90 degrees
	vec3 vec = cross(vec3(ab.x, ab.y, 0.f), vec3(0.f, 0.f, 1.f));
	normals[0] = vec2(vec.x, vec.y);

	vec = cross(vec3(bc.x, bc.y, 0.f), vec3(0.f, 0.f, 1.f));
	normals[1] = vec2(vec.x, vec.y);

	return line_line_intersection_point(midpoints[0], midpoints[0] + normals[0], midpoints[1], midpoints[1] + normals[1]);
}


vec2[2] ray_circle_intersection(vec2 ray0, vec2 ray1, vec2 center, float r, out bool hit)
{
	vec2 result[2];
	vec2 dir = normalize(ray1 - ray0);
	vec2 l = center - ray0;
	float s = dot(l, dir);
	float l_2 = dot(l, l);
	float m_2 = l_2 - (s * s);
	float r_2 = (r*r);
	if (m_2 > r_2)
	{
		hit = false;
		return result;
	}
	float q = sqrt(r_2 - m_2);

	float t[2];
	t[0] = s - q;
	t[1] = s + q;

	for (int i = 0; i < 2; i++)
	{
		result[i] = ray0 + dir * t[i];
	}
	hit = true;
	return result;
}

//-----------------------------------------------------------
// Symedge functions
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

int crot(int edge)
{
	int sym_i = sym(edge);
	return (sym_i != -1) ? nxt(sym_i) : -1;
}
SymEdge prev_symedge(in SymEdge s)
{
	return get_symedge(get_symedge(s.nxt).nxt);
}

int get_index(SymEdge s)
{
	return prev_symedge(s).nxt;
}

//-----------------------------------------------------------
// Access funcitons
//-----------------------------------------------------------
void get_face(int face_i, out vec2 face_v[3])
{
	face_v[0] = point_positions[sym_edges[tri_symedges[face_i].x].vertex];
	face_v[1] = point_positions[sym_edges[sym_edges[tri_symedges[face_i].x].nxt].vertex];
	face_v[2] = point_positions[sym_edges[sym_edges[sym_edges[tri_symedges[face_i].x].nxt].nxt].vertex];
}

	vec2[2] get_segment(int index)
{
	ivec2 seg_edge_i = seg_endpoint_indices[index];
	vec2 s[2];
	s[0] = point_positions[seg_edge_i[0]];
	s[1] = point_positions[seg_edge_i[1]];
	return s;
}
bool face_contains_vertice(int face, int vertex)
{
	SymEdge s = sym_edges[tri_symedges[face].x];
	return s.vertex == vertex || get_symedge(s.nxt).vertex == vertex || prev_symedge(s).vertex == vertex;
}

int get_face_vertex_symedge(int face, int vertex)
{
	SymEdge s = sym_edges[tri_symedges[face].x];
	if (s.vertex == vertex)
		return get_index(s);
	else if (get_symedge(s.nxt).vertex == vertex)
		return s.nxt;
	else if (prev_symedge(s).vertex == vertex)
		return get_index(prev_symedge(s));
	else
		return -1;
}

vec2 get_face_center(int face_i)
{
	vec2 face_v[3];
	face_v[0] = point_positions[sym_edges[tri_symedges[face_i].x].vertex];
	face_v[1] = point_positions[sym_edges[sym_edges[tri_symedges[face_i].x].nxt].vertex];
	face_v[2] = point_positions[sym_edges[sym_edges[sym_edges[tri_symedges[face_i].x].nxt].nxt].vertex];

	return (face_v[0] + face_v[1] + face_v[2]) / 3.f;
}

vec2[2] get_edge(int s_edge)
{
	vec2 edge[2];
	edge[0] = point_positions[sym_edges[s_edge].vertex];
	edge[1] = point_positions[sym_edges[nxt(s_edge)].vertex];
	return edge;
}

//-----------------------------------------------------------
// Locate disturbance functions
//-----------------------------------------------------------
bool possible_disturbance(vec2 a, vec2 b, vec2 c, in vec2 s[2])
{
	vec2 sector_c = project_point_on_line(b, a, c);
	float dist = 2.f * length(sector_c - a);
	sector_c = a + normalize(c - a) * dist;
	if (edge_intersects_sector(a, b, sector_c, s))
		return true;
	vec2 p = get_symmetrical_corner(a, b, c);
	sector_c = c + normalize(a - c) * dist;

	if (edge_intersects_sector(a, b, sector_c, s))
		return true;

	return false;
}

int find_closest_constraint(vec2 a, vec2 b, vec2 c)
{
	float dist = FLT_MAX;
	int ret = -2;
	for (int i = 0; i < seg_endpoint_indices.length(); i++)
	{
		ivec2 seg_i = seg_endpoint_indices[i];
		vec2 s[2];
		s[0] = point_positions[seg_i[0]];
		s[1] = point_positions[seg_i[1]];
		if (possible_disturbance(a, b, c, s))
		{
			bool projectable;
			vec2 b_prim = project_point_on_segment(b, s[0], s[1], projectable);
			if (projectable)
			{
				float b_dist = length(b_prim - b);
				if (b_dist < dist && !(point_equal(b_prim, a) || point_equal(b_prim, c)))
				{
					dist = b_dist;
					ret = i;
				}
			}
		}
	}
	return ret;
}

void oriented_walk_edge(inout int curr_e, vec2 point, out bool on_edge)
{
	bool done = false;
	vec2 goal = point;
	int iter = 0;
	on_edge = false;
	while (!done) {
		on_edge = false;
		// Loop through triangles edges to check if point is on the edge 
		for (int i = 0; i < 3; i++)
		{
			bool hit;
			hit = point_line_test(goal,
				point_positions[sym_edges[curr_e].vertex],
				point_positions[sym_edges[sym_edges[curr_e].nxt].vertex]);
			if (hit)
			{
				on_edge = true;
				return;
			}
			curr_e = sym_edges[curr_e].nxt;
		}
		// calculate triangle centroid
		vec2 tri_points[3];
		get_face(sym_edges[curr_e].face, tri_points);
		vec2 tri_cent;
		tri_cent = (tri_points[0] + tri_points[1] + tri_points[2]) / 3.0f;
		// Loop through edges to see if we should continue through the edge
		// to the neighbouring triangle 
		bool line_line_hit = false;
		for (int i = 0; i < 3; i++)
		{
			line_line_hit = line_line_test(
				tri_cent,
				goal,
				point_positions[sym_edges[curr_e].vertex],
				point_positions[sym_edges[sym_edges[curr_e].nxt].vertex]
			);

			if (line_line_hit)
			{
				break;
			}
			curr_e = sym_edges[curr_e].nxt;
		}

		if (line_line_hit)
		{
			curr_e = sym_edges[sym_edges[curr_e].nxt].rot; // sym
		}
		else
		{
			return;
		}
	}
}

int find_segment_symedge(int start, int segment)
{
	vec2 seg_p[2] = get_segment(segment);
	vec2 goal = (seg_p[0] + seg_p[1]) / 2.0f;
	bool on_edge;
	oriented_walk_edge(start, goal, on_edge);
	if (on_edge)
		return start;
	else
		return -1;
}

bool check_for_sliver_tri(int sym_edge)
{
	// first find the longest edge
	float best_dist = 0.0f;
	int best_i = -1;
	vec2 tri[3];
	get_face(sym_edges[sym_edge].face, tri);
	for (int i = 0; i < 3; i++)
	{
		float dist = distance(tri[i], tri[(i + 1) % 3]);
		if (dist > best_dist)
		{
			best_i = i;
			best_dist = dist;
		}
	}
	// then check if the third point is on the ray of that line.
	vec2 p1 = tri[(best_i + 2) % 3];
	vec2 s1 = tri[best_i];
	vec2 s2 = tri[(best_i + 1) % 3];
	return point_ray_test(p1, s1, s2);
}

int oriented_walk_point(int curr_e, int goal_point_i)
{
	vec2 tri_cent;
	vec2 goal_point = get_vertex(goal_point_i);
	float epsi = EPSILON;
	while (true)
	{
		if (face_contains_vertice(get_symedge(curr_e).face, goal_point_i))
			return get_face_vertex_symedge(get_symedge(curr_e).face, goal_point_i);

		tri_cent = get_face_center(sym_edges[curr_e].face);

		// Loop through edges to see if we should continue through the edge
		// to the neighbouring triangle 
		bool line_line_hit = false;
		for (int i = 0; i < 3; i++)
		{
			if (sym_edges[sym_edges[curr_e].nxt].rot == -1)
			{
				curr_e = sym_edges[curr_e].nxt;
				continue;
			}



			// No degenerate triangles detected
			line_line_hit = line_line_test(
				tri_cent,
				goal_point,
				point_positions[sym_edges[curr_e].vertex],
				point_positions[sym_edges[sym_edges[curr_e].nxt].vertex],
				epsi);

			if (line_line_hit)
			{
				epsi = EPSILON;
				// handle degenerate triangles
				bool not_valid_edge = false;
				SymEdge other_e = prev(sym(sym_edges[curr_e]));

				vec2 p1 = point_positions[get_symedge(sym_edges[curr_e].nxt).vertex];
				vec2 p2 = point_positions[sym_edges[curr_e].vertex];
				//not_valid_edge = point_ray_test(get_vertex(other_e.vertex), p1, p2);
				not_valid_edge = check_for_sliver_tri(other_e.nxt);
				if (not_valid_edge == true)
				{
					//magic = 1;
					//return 0;
					curr_e = sym(curr_e);
					while (not_valid_edge == true)
					{
						// check if face contains vertex
						if (face_contains_vertice(get_symedge(curr_e).face, goal_point_i))
							return get_face_vertex_symedge(get_symedge(curr_e).face, goal_point_i);
						float dir;
						int i = 0;
						do {
							// continue until the edge of triangle is found that is facing the other
							// compared to the intial edge, so we are checking when the edges start going the other
							// direction by doing the dot product between the last edge and next edge, when the dot is 
							// negative it implies that the next edge is facing another direction than the previous ones.
							curr_e = nxt(curr_e);
							vec2 ab = point_positions[sym_edges[prev(curr_e)].vertex] - point_positions[sym_edges[curr_e].vertex];
							vec2 bc = point_positions[sym_edges[curr_e].vertex] - point_positions[sym_edges[nxt(curr_e)].vertex];
							dir = dot(ab, bc);
						} while (dir > 0.0f);
						// check if we are out of the degenerate triangulation
						other_e = prev_symedge(sym(sym_edges[curr_e]));
						//p1 = point_positions[get_symedge(sym_edges[curr_e].nxt).vertex];
						//p2 = point_positions[sym_edges[curr_e].vertex];
						//not_valid_edge = point_ray_test(get_vertex(other_e.vertex), p1, p2);
						not_valid_edge = check_for_sliver_tri(other_e.nxt);
						curr_e = sym(curr_e);
					}
					// move to other triangle
					curr_e = nxt(curr_e);
					break;
				}
				else {
					curr_e = sym_edges[sym_edges[sym_edges[curr_e].nxt].rot].nxt;
					//curr_e = nxt(sym_edges[sym_edges[curr_e].nxt].rot);
					break;
				}
			}
			curr_e = sym_edges[curr_e].nxt;
		}
		epsi *= 2.0f;

	}
	return -1;
}

int find_next_rot(int start, int curr, inout bool reverse)
{
	int edge = curr;
	//Move to nxt edge to check if it is a constraint
	if (reverse) // reverse
	{
		edge = crot(edge);
	}
	else // forward
	{
		edge = rot(edge);
		if (edge == start)
		{
			return -1;
		}
		else if (edge == -1)
		{
			reverse = true;
			edge = start;
			edge = crot(edge);
		}
	}
	return edge;
}

int find_next_rot_constraint(int start, int curr, inout bool reverse)
{
	curr = find_next_rot(start, curr, reverse);
	while (curr != -1)
	{
		if (edge_is_constrained[sym_edges[curr].edge] != -1)
		{
			return curr;
		}
		curr = find_next_rot(start, curr, reverse);
	}
	return curr;
}

bool no_collinear_constraints(int v)
{
	int edge;
	bool reverse = false;
	int curr_constraint = v;
	int last_constraint = -1;
	vec2 point = point_positions[sym_edges[v].vertex];
	// first check if initial value is an constraint
	if (edge_is_constrained[sym_edges[v].edge] != -1)
	{
		curr_constraint = v;
	}
	else {
		curr_constraint = find_next_rot_constraint(v, v, reverse);
	}
	while (curr_constraint != -1)
	{
		vec2 curr_vec = normalize(point_positions[sym_edges[nxt(curr_constraint)].vertex] - point);
		// explore if current constraint is collinear with another constraint
		bool explore_rd = reverse;
		edge = find_next_rot_constraint(v, curr_constraint, explore_rd);
		while (edge != -1)
		{

			//if it is a constraint check if it is collinear with curr_constraint
			vec2 other_vec = normalize(point_positions[sym_edges[nxt(edge)].vertex] - point);
			if (dot(curr_vec, other_vec) < -1 + EPSILON)
			{
				return false;
			}
			// find next constraint
			edge = find_next_rot_constraint(v, edge, explore_rd);
		}

		// find next constraint to explore
		curr_constraint = find_next_rot_constraint(v, curr_constraint, reverse);
	}
	return true;
}

bool is_orthogonally_projectable(vec2 v, vec2 a, vec2 b)
{
	vec2 line = b - a;
	vec2 projected_point = project_point_on_line(v, a, b);

	float projected_length = dot(normalize(line), projected_point - a);

	if (projected_length < 0.f || projected_length * projected_length > dot(line, line))
		return false;

	return true;
}

float local_clearance(vec2 b, in vec2 segment[2])
{
	vec2 b_prim = project_point_on_line(b, segment[0], segment[1]);
	return length(b - b_prim);
}

vec2 find_e_point(inout int v_sym, vec2 v, vec2 v_prim)
{
	vec2 e;
	int edge = v_sym;
	bool reverse = false;
	while (true)
	{
		// Check if edge leads to finding point e
		e = point_positions[sym_edges[nxt(edge)].vertex];
		vec2 d = point_positions[sym_edges[prev(edge)].vertex];
		if (line_line_test(v, v_prim, e, d))
		{
			v_sym = edge;
			return e;
		}

		// Move to next edge
		if (reverse)
		{
			edge = crot(edge);
			if (edge == -1)
			{
				break;
			}
		}
		else
		{
			edge = rot(edge);
			if (edge == v_sym)
			{
				break;
			}
			else if (edge == -1)
			{
				reverse = true;
				edge = v_sym;
			}
		}
	}
	// should not happen, error
	e = vec2(FLT_MAX);
	return e;
}

float is_disturbed(int constraint, int b_sym, inout int v_sym)
{
	// 1
	if (!no_collinear_constraints(v_sym))
		return -1.0f;

	// 2
	vec2 v = point_positions[sym_edges[v_sym].vertex];
	vec2 a = point_positions[sym_edges[prev(b_sym)].vertex];
	vec2 b = point_positions[sym_edges[b_sym].vertex];
	vec2 c = point_positions[sym_edges[nxt(b_sym)].vertex];

	if (!is_orthogonally_projectable(v, a, c))
		return -1.0f;

	// 3
	vec2 c_endpoints[2] = get_edge(constraint);
	vec2 v_prim = project_point_on_line(v, c_endpoints[0], c_endpoints[1]);
	if (!(line_line_test(v, v_prim, a, c) && line_line_test(v, v_prim, b, c)))
		return -1.0f;

	// 4
	float dist_v_segment = length(v_prim - v);
	if (!(dist_v_segment < local_clearance(b, c_endpoints)))
		return -1.0f;

	// 5 
	vec2 e = find_e_point(v_sym, v, v_prim);
	if (!(dist_v_segment < length(v - e)))
		return -1.0f;

	return dist_v_segment;
}

int find_constraint_disturbance(int constraint, int edge_ac, bool right)
{
	vec2 R[3];
	vec2 a;
	// Set variables needed to calculate R
	if (right)
	{
		R[0] = point_positions[sym_edges[edge_ac].vertex];
		R[1] = point_positions[sym_edges[prev(edge_ac)].vertex];
		a = point_positions[sym_edges[nxt(edge_ac)].vertex];
	}
	else {
		R[0] = point_positions[sym_edges[nxt(edge_ac)].vertex];
		R[1] = point_positions[sym_edges[prev(edge_ac)].vertex];
		a = point_positions[sym_edges[edge_ac].vertex];
	}
	// Calculate R
	vec2 ac = R[0] - a;
	vec2 dir = normalize(ac);
	vec2 ab = R[1] - a;
	float b_prim = dot(dir, ab);
	R[2] = R[1] + (dir * (length(ac) - b_prim));
	// Loop through points trying to find disturbance to current traversal
	float best_dist = FLT_MAX;
	int first_disturb = -1;
	float best_dist_b = 0.0f;
	vec2 tri[3];
	get_face(sym_edges[edge_ac].face, tri);
	for (int i = 0; i < point_positions.length(); i++)
	{
		vec2 point = point_positions[i];

		// check if point is inside of R
		if (point_triangle_test(point, R))
		{
			// If point is one of the triangles point continue to next point
			if (point_equal_tri_vert(point, tri) > -1)
				continue;
			// TODO: Change oriented walk to start from last point instead of the constraint
			int v_edge = oriented_walk_point(constraint, i);
			float dist = is_disturbed(constraint, prev(edge_ac), v_edge);
			if (dist > 0.0f)
			{
				if (dist < best_dist)
				{
					first_disturb = v_edge;
					best_dist = dist;
					best_dist_b = distance(point_positions[sym_edges[v_edge].vertex],
						point_positions[sym_edges[prev(edge_ac)].vertex]);
				}
				else if (dist < (best_dist + EPSILON))
				{
					// if new point has the same distance as the previous one
					// check if it is closer to b
					float dist_b = distance(point_positions[sym_edges[v_edge].vertex],
						point_positions[sym_edges[prev(edge_ac)].vertex]);
					if (dist_b < best_dist_b)
					{
						first_disturb = v_edge;
						best_dist = dist;
						best_dist_b = dist_b;
					}
				}
			}

		}
	}

	return first_disturb;
}

vec2 calculate_refinement(int c, int v_sym, out bool success)
	{
		vec2 tri[3];
		get_face(sym_edges[v_sym].face, tri);
		vec2 circle_center = circle_center_from_points(tri[0], tri[1], tri[2]);
		float radius = distance(circle_center, tri[0]);
		vec2 constraint_edge[2] = get_edge(c);
		vec2 inter_points[2] = ray_circle_intersection(constraint_edge[1], constraint_edge[0], circle_center, radius, success);
		if (success)
		{
			return (inter_points[0] + inter_points[1]) / 2.0f;
		}
		else
		{
			return vec2(0.0f);
		}
	}

// Each thread represents one triangle
void main(void)
{
	gid = gl_GlobalInvocationID.x;
	int index = int(gid);
	int num_threads = int(gl_NumWorkGroups.x * gl_WorkGroupSize.x);
	while(index < tri_seg_inters_index.length())
	{
		if (tri_symedges[index].x > -1)
			{
				int num_constraints = 0;
				int c_edge_i[3] = { -1 ,-1,-1 };
				int tri_edge_i[3];
				// loop checking for constraints in triangle
				for (int i = 0; i < 3; i++)
				{
					// Checking if edge of triangle is constrained.
					if (edge_is_constrained[sym_edges[tri_symedges[index][i]].edge] > -1)
					{
						c_edge_i[num_constraints] = tri_symedges[index][i];
						tri_edge_i[num_constraints] = tri_symedges[index][i];
						num_constraints++;
					}
				}
				// if no constraints where found in triangle look for nearby constraints
				if (num_constraints == 0)
				{
					// Loop through edges of the triangles finding the closest constraints
					// to each traversal.
					ivec4 tri_symedge_i = tri_symedges[index];
					vec2 tri[3];
					get_face(sym_edges[tri_symedge_i.x].face, tri);
					//			tri_insert_points[index].pos = tri[0];
					for (int i = 0; i < 3; i++)
					{
						int cc = find_closest_constraint(tri[(i + 1) % 3], tri[(i + 2) % 3], tri[i]);
						// Check if a segment was found
						if (cc > -1)
						{
							cc = find_segment_symedge(tri_symedge_i[i], cc);
							// Check if corresponding constraint to segment was found
							if (cc > -1)
							{
								c_edge_i[num_constraints] = cc;
								tri_edge_i[num_constraints] = tri_symedge_i[i];
								num_constraints++;
							}
						}

					}
				}

				// find disturbances
				for (int i = 0; i < num_constraints; i++)
				{
					if (c_edge_i[i] > -1)
					{
						// test right side
						int disturb = find_constraint_disturbance(c_edge_i[i], tri_edge_i[i], true);
						if (disturb >= 0)
						{
							bool success;
							vec2 calc_pos = calculate_refinement(c_edge_i[i], disturb, success);
							if (success)
							{
								NewPoint tmp;
								tmp.pos = calc_pos;
								//tmp.index = atomicAdd(status, 1);
								tmp.index = status++;
								tmp.face_i = sym_edges[c_edge_i[i]].face;
								refine_points[disturb] = tmp;
							}
						}
						// test left side
						disturb = find_constraint_disturbance(c_edge_i[i], tri_edge_i[i], false);
						if (disturb >= 0)
						{
							bool success;
							vec2 calc_pos = calculate_refinement(c_edge_i[i], disturb, success);
							if (success)
							{
								NewPoint tmp;
								tmp.pos = calc_pos;
								//tmp.index = atomicAdd(status, 1);
								tmp.index = status++;
								tmp.face_i = sym_edges[c_edge_i[i]].face;
								refine_points[disturb] = tmp;
							}
						}
					}

				}
			}
		index += num_threads;
	}
}