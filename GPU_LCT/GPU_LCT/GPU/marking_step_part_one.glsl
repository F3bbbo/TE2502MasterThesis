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
// Access funcitons
//-----------------------------------------------------------
vec2 get_vertex(int index)
{
	return point_positions[index];
}

SymEdge get_symedge(int index)
{
	return sym_edges[index];
}


//-----------------------------------------------------------
// SymEdge funcitons
//-----------------------------------------------------------

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

SymEdge sym_symedge(SymEdge s)
{
	return get_symedge(get_symedge(s.nxt).rot);
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

bool face_contains_vertice(int face, int vertex)
{
	int sym_x = tri_symedges[face].x;
	return sym_edges[sym_x].vertex == vertex || sym_edges[nxt(sym_x)].vertex == vertex || sym_edges[prev(sym_x)].vertex == vertex;
}
int get_index(SymEdge s)
{
	return prev_symedge(s).nxt;
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

int crot_symedge_i(SymEdge s)
{
	int index = get_symedge(s.nxt).rot;
	return index != -1 ? get_symedge(index).nxt : -1;
}
int get_label(int index)
{
	return edge_label[index];
}

void get_face(int face_i, out vec2 face_v[3])
{
	face_v[0] = point_positions[sym_edges[tri_symedges[face_i].x].vertex];
	face_v[1] = point_positions[sym_edges[sym_edges[tri_symedges[face_i].x].nxt].vertex];
	face_v[2] = point_positions[sym_edges[sym_edges[sym_edges[tri_symedges[face_i].x].nxt].nxt].vertex];
}

bool face_contains_vertex(int vert, SymEdge s_triangle)
{
	ivec3 tri = ivec3(s_triangle.vertex, get_symedge(s_triangle.nxt).vertex, prev_symedge(s_triangle).vertex);
	return vert == tri.x || vert == tri.y || vert == tri.z;
}





//-----------------------------------------------------------
// Math Functions
//-----------------------------------------------------------
vec2 get_face_center(int face_i)
{
	vec2 face_v[3];
	face_v[0] = point_positions[sym_edges[tri_symedges[face_i].x].vertex];
	face_v[1] = point_positions[sym_edges[sym_edges[tri_symedges[face_i].x].nxt].vertex];
	face_v[2] = point_positions[sym_edges[sym_edges[sym_edges[tri_symedges[face_i].x].nxt].nxt].vertex];

	return (face_v[0] + face_v[1] + face_v[2]) / 3.f;
}

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

float vec2_cross(in vec2 v, in vec2 w)
{
	return v.x*w.y - v.y*w.x;
}

bool check_for_sliver_tri(int sym_edge, float epsi = EPSILON)
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

	return point_ray_test(p1, s1, s2, epsi);
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
		float l_epsi = 0.0001f;
		float u = qpr / rs;
		float t = vec2_cross(qp, s) / rs;
		if ((0.0f - l_epsi) <= u && u <= (1.0f + l_epsi) && (0.0f - l_epsi) <= t && t <= (1.0f + l_epsi))
			return true;
	}
	return false;
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

//-----------------------------------------------------------
// Functions
//-----------------------------------------------------------
int oriented_walk_point(int start_e, int goal_point_i)
{
	int curr_e = start_e;
	vec2 tri_cent;
	vec2 goal_point = get_vertex(goal_point_i);
	float epsi = EPSILON;
	int counter = 0;
	while (true)
	{
		counter++;
		//if (counter > 1001)
		//{
		//	edge_label[sym_edges[curr_e].edge] = -2;
		//	edge_is_constrained[sym_edges[curr_e].edge] = goal_point_i;
		//	return -1;
		//}
		if (face_contains_vertice(sym_edges[curr_e].face, goal_point_i))
			return get_face_vertex_symedge(sym_edges[curr_e].face, goal_point_i);
		//check if current triangle is a sliver triangle
		bool sliver_tri = check_for_sliver_tri(curr_e);
		if (sliver_tri)
		{
			float dir;
			do {
				// continue until the edge of triangle is found that is facing the other
				// compared to the intial edge, so we are checking when the edges start going the other
				// direction by doing the dot product between the last edge and next edge, when the dot is 
				// negative it implies that the next edge is facing another direction than the previous ones.
				curr_e = nxt(curr_e);
				vec2 ab = point_positions[sym_edges[curr_e].vertex] - point_positions[sym_edges[prev(curr_e)].vertex];
				vec2 bc = point_positions[sym_edges[nxt(curr_e)].vertex] - point_positions[sym_edges[curr_e].vertex];
				dir = dot(ab, bc);
			} while (dir > 0.0f);
			int last_e = curr_e;
			do
			{
				curr_e = sym(last_e);
				last_e = nxt(last_e);
			} while (curr_e < 0);
		}
		else
		{
			curr_e = nxt(curr_e);
			// No degenerate triangles detected
			tri_cent = get_face_center(sym_edges[curr_e].face);

			for (int i = 0; i < 3; i++)
			{
				if (sym(curr_e) < 0)
				{
					curr_e = nxt(curr_e);
					continue;
				}
				vec2 s1 = point_positions[sym_edges[curr_e].vertex];
				vec2 s2 = point_positions[sym_edges[sym_edges[curr_e].nxt].vertex];


				bool line_line_hit = point_line_test(goal_point, s1, s2) || line_line_test(
					tri_cent,
					goal_point,
					s1,
					s2);
				if (line_line_hit)
				{
					curr_e = sym(curr_e);
					break;
				}
				else
				{
					curr_e = nxt(curr_e);
				}
			}
		}
	}
	return -1;
}

int points_connected(int e1, int other_vertex)
{
	int curr_e = e1;
	bool reverse_direction = false;

	while (true)
	{
		if (get_symedge(sym_edges[curr_e].nxt).vertex == other_vertex)
			return sym_edges[curr_e].edge;

		else
		{
			if (!reverse_direction)
			{
				if (sym_edges[curr_e].rot == e1)
					return -1;

				if (sym_edges[curr_e].rot != -1)
					curr_e = get_symedge(curr_e).rot;
				else
				{
					reverse_direction = true;
					curr_e = crot_symedge_i(get_symedge(e1));
					if (curr_e == -1)
						return -1;
				}
			}
			else
			{
				curr_e = crot_symedge_i(get_symedge(curr_e));
				if (curr_e == -1)
					return -1;
			}
		}
	}
}

void process_triangle(int segment_index, SymEdge triangle)
{
	// Assumes that the provided symedge is the symedge between the trianlges T, T+1
	if (get_label(triangle.edge) != 3 &&
		get_label(get_symedge(triangle.nxt).edge) != 3 &&
		get_label(prev_symedge(triangle).edge) != 3 &&
		(tri_seg_inters_index[triangle.face] > segment_index ||
			tri_seg_inters_index[triangle.face] == -1))
		tri_seg_inters_index[triangle.face] = segment_index;
}

bool check_side(vec2 direction, vec2 other)
{
	return dot(direction, other) >= 0.f ? true : false;
}

bool polygonal_is_strictly_convex(int num, vec2 p1, vec2 p2, vec2 p3, vec2 p4, vec2 p5 = vec2(0.f, 0.f))
{
	// Definition of a stricly convex set from wikipedia
	// https://en.wikipedia.org/wiki/Convex_set

	// Let S be a vector space over the real numbers, or, more generally, over some ordered field. This includes Euclidean spaces. 
	// A set C in S is said to be convex if, for all x and y in C and all t in the interval (0, 1), the point (1 − t)x + ty also belongs to C.
	// In other words, every point on the line segment connecting x and y is in C
	// This implies that a convex set in a real or complex topological vector space is path-connected, thus connected.
	// Furthermore, C is strictly convex if every point on the line segment connecting x and y other than the endpoints is inside the interior of C.

	// My made up solution
	const vec2 point_array[5] = { p1, p2, p3, p4, p5 };
	
	for (int i = 0; i < num; i++)
	{
		vec2 line = point_array[(i + 1) % num] - point_array[i];

		// rotate vector by 90 degrees
		{
			float tmp = line.x;
			line.x = line.y;
			line.y = -tmp;
		}

		for (int j = 0; j < num - 2; j++)
		{
			if (!check_side(line, point_array[(i + 2 + j) % num] - point_array[(i + 1) % num]))
				return false;
		}
	}
	return true;
}

bool pre_candidate_check(SymEdge s)
{
	vec2 p1 = get_vertex(get_symedge(get_symedge(s.nxt).nxt).vertex);
	vec2 e1 = get_vertex(s.vertex);
	vec2 e2 = get_vertex(get_symedge(s.nxt).vertex);
	vec2 p2 = get_vertex(get_symedge(get_symedge(sym_symedge(s).nxt).nxt).vertex);
	return polygonal_is_strictly_convex(4, p1, e1, p2, e2);
}
bool will_be_flipped(int segment_index, SymEdge triangle)
{
	if (tri_seg_inters_index[triangle.face] == segment_index && tri_seg_inters_index[sym_symedge(triangle).face] == segment_index && edge_label[triangle.edge] < 3)
	{
		edge_label[triangle.edge] = 2;
		return true;
	}

	return false;
}

bool first_candidate_check(vec2 s1, vec2 s2, SymEdge ei)
{
	vec2 p1 = get_vertex(prev_symedge(ei).vertex);
	vec2 e1 = get_vertex(ei.vertex);
	vec2 e2 = get_vertex(get_symedge(ei.nxt).vertex);
	vec2 p2 = get_vertex(prev_symedge(sym_symedge(ei)).vertex);

	bool intersects_first = segment_triangle_test(s1, s2, p1, e1, p2);
	bool intersects_second = segment_triangle_test(s1, s2, p1, p2, e2);
	return intersects_first ^^ intersects_second;
}

bool second_candidate_check(vec2 p1, vec2 p2, vec2 p3, vec2 p4, vec2 p5)
{
	// input parameters forms a pentagonal polygonal
	return polygonal_is_strictly_convex(5, p1, p2, p3, p4, p5);
}

bool third_candidate_check(bool edges_connected, vec2 p1, vec2 p2, vec2 p3, vec2 p4, vec2 p5)
{
	if (edges_connected)
		return !polygonal_is_strictly_convex(4, p1, p2, p4, p5) && polygonal_is_strictly_convex(4, p1, p2, p3, p5) == true ? true : false;
	else
		return !polygonal_is_strictly_convex(4, p1, p2, p3, p5) && polygonal_is_strictly_convex(4, p1, p2, p4, p5) == true ? true : false;
}

bool Qi_check(int segment_index, SymEdge ei1, SymEdge ei)
{
	vec2 s1 = get_vertex(seg_endpoint_indices[segment_index].x);
	vec2 s2 = get_vertex(seg_endpoint_indices[segment_index].y);

	vec2 vertices[5];

	vertices[0] = get_vertex(prev_symedge(ei1).vertex);
	bool edges_connected = ei1.vertex == ei.vertex;
	if (edges_connected)
	{
		vertices[1] = get_vertex(ei.vertex);
		vertices[2] = get_vertex(prev_symedge(sym_symedge(ei)).vertex);
		vertices[3] = get_vertex(get_symedge(ei.nxt).vertex);
		vertices[4] = get_vertex(get_symedge(ei1.nxt).vertex);
	}
	else
	{
		vertices[1] = get_vertex(ei1.vertex);
		vertices[2] = get_vertex(ei.vertex);
		vertices[3] = get_vertex(prev_symedge(sym_symedge(ei)).vertex);
		vertices[4] = get_vertex(get_symedge(ei.nxt).vertex);
	}

	if (polygonal_is_strictly_convex(4, vertices[1], vertices[2], vertices[3], vertices[4]))
	{
		if (first_candidate_check(s1, s2, ei) ||
			second_candidate_check(vertices[0], vertices[1], vertices[2], vertices[3], vertices[4]) ||
			third_candidate_check(edges_connected, vertices[0], vertices[1], vertices[2], vertices[3], vertices[4]))
		{
			return true;
		}
		return false;
	}
	return false;
}

void straight_walk(inout int segment_index, SymEdge s_starting_point, int ending_point_i)
{
	SymEdge cur_edge = s_starting_point;
	SymEdge prev_intersecting_edge = s_starting_point;
	vec2 constraint_edge[2];
	constraint_edge[0] = get_vertex(s_starting_point.vertex);
	constraint_edge[1] = get_vertex(ending_point_i);
	vec2 normalized_constrained_edge = normalize(constraint_edge[1] - constraint_edge[0]);
	int counter = 0;
	while (true)
	{

		vec2 v0 = get_vertex(get_symedge(cur_edge.nxt).vertex);
		vec2 v1 = get_vertex(prev_symedge(cur_edge).vertex);
		if (line_seg_intersection_ccw(constraint_edge[0], constraint_edge[1], v0, v1))
		{
			cur_edge = get_symedge(cur_edge.nxt);

			// Check if the sym triangle contains the segment endpoint
			if (face_contains_vertex(ending_point_i, sym_symedge(cur_edge)))
			{
				process_triangle(segment_index, cur_edge);
				process_triangle(segment_index, sym_symedge(cur_edge));

				// check if the edge satisfies to conditions and mark it to be flipped if needed, should always return here.
				if (pre_candidate_check(cur_edge))
					will_be_flipped(segment_index, cur_edge);
				return;
			}

			prev_intersecting_edge = cur_edge;
			process_triangle(segment_index, cur_edge);
			if (pre_candidate_check(cur_edge) && will_be_flipped(segment_index, cur_edge))
				return;
			cur_edge = get_symedge(sym_symedge(cur_edge).nxt);
			break;
		}
		cur_edge = get_symedge(cur_edge.rot);
		if (cur_edge == s_starting_point)
			return;
	}

	// walk towards the constraint endpoins and stop if we reach the triangle that contains the segment endpoint
	while (true)
	{
		counter++;
		if (counter > 500)
		{
			edge_label[cur_edge.edge] = -3;
			segment_index = -1;
			return;
		}
		int checks;
		for (checks = 0; checks < 2; checks++)
		{
			// Check if the sym triangle contains the segment endpoint
			if (face_contains_vertex(ending_point_i, sym_symedge(cur_edge)))
			{
				process_triangle(segment_index, cur_edge);
				process_triangle(segment_index, sym_symedge(cur_edge));
				if (pre_candidate_check(cur_edge))
					will_be_flipped(segment_index, cur_edge);
				return;
			}

			// Checks if the segment intersects an edge
			vec2 v0 = get_vertex(cur_edge.vertex);
			vec2 v1 = get_vertex(get_symedge(cur_edge.nxt).vertex);

			if (line_line_test(constraint_edge[0], constraint_edge[1], v0, v1))
			{
				process_triangle(segment_index, cur_edge);
				if (Qi_check(segment_index, prev_intersecting_edge, cur_edge) && will_be_flipped(segment_index, cur_edge))
					return;
				prev_intersecting_edge = cur_edge;
				cur_edge = get_symedge(sym_symedge(cur_edge).nxt);
				break;
			}
			cur_edge = get_symedge(cur_edge.nxt);
		}


	}

}

void main(void)
{
	// Each thread is responsible for a segment

	// The purpose of this shader is to mark triangles that intersects a not yet inserted segment, but whose endpoints has been inserted,
	// an edge is marked to be flipped if it is intersected by a segment and if both of its adjacent triangles also are being intersected.

	// If both endpoint already have an edge beweeen them then the edge is marked to be a segment and to not get flipped.

	uint gid = gl_GlobalInvocationID.x;
	int index = int(gid);
	int num_threads = int(gl_NumWorkGroups.x * gl_WorkGroupSize.x);

	// Check if the segment has not been inserted and if both endpoints has been inserted
	while (index < seg_inserted.length())
	{
		//seg_inserted[index] = seg_inserted[index];
		//return;
		if (seg_inserted[index] == 0)
		{
			if (seg_endpoint_indices[index].x > -1)
			{
				int endpoints_inserted = point_inserted[seg_endpoint_indices[index].x] * point_inserted[seg_endpoint_indices[index].y];
				if (endpoints_inserted == 1)
				{
					int start_index = tri_symedges[point_tri_index[seg_endpoint_indices[index].x]].x;
					//if (start_index < 0)
					//{	
					//	edge_label[index] = -2;
					//	edge_is_constrained[index] = seg_inserted[index];
					//	return ;
					//}
					int starting_symedge = oriented_walk_point(start_index, seg_endpoint_indices[index].x);
					//if (start_index < 0)
					//	return;
					//if (ending_symedge < 0)
					//	return;
					// update the points triangle indexes
					point_tri_index[sym_edges[starting_symedge].vertex] = sym_edges[starting_symedge].face;
					int connecting_edge = points_connected(starting_symedge, seg_endpoint_indices[index].y);
					if (connecting_edge != -1)
					{
						edge_is_constrained[connecting_edge] = index;
						edge_label[connecting_edge] = 0;
						seg_inserted[index] = 1;
						status = 1;
					}
					else
					{
						int ending_symedge = oriented_walk_point(start_index, seg_endpoint_indices[index].y);
						point_tri_index[sym_edges[ending_symedge].vertex] = sym_edges[ending_symedge].face;

						straight_walk(index, get_symedge(starting_symedge), seg_endpoint_indices[index].y);
						//if (index < 0)
						//	return;
						straight_walk(index, get_symedge(ending_symedge), seg_endpoint_indices[index].x);
						//if (index < 0)
						//	return;
					}
				}
			}
		}
		index += num_threads;
	}
}
