#version 430
#define EPSILON 0.0005f
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
	ivec3 tri_vertex_indices[];
};
layout(std430, binding = 8) buffer Tri_buff_1
{
	ivec4 tri_symedges[];
};
layout(std430, binding = 9) buffer Tri_buff_2
{
	int tri_ins_point_index[];
};
layout(std430, binding = 10) buffer Tri_buff_3
{
	int tri_seg_inters_index[];
};
layout(std430, binding = 11) buffer Tri_buff_4
{
	int tri_edge_flip_index[];
};
layout(std430, binding = 12) buffer symedge_buff
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

int crot_symedge_i(SymEdge s)
{
	int index = get_symedge(s.nxt).rot;
	return index != -1 ? get_symedge(index).nxt : -1;
}

vec2 get_vertex(int index)
{
	return point_positions[index];
}

int get_label(int index)
{
	return edge_label[index];
}

vec2 get_face_center(int face_i)
{
	vec2 face_v[3];
	face_v[0] = point_positions[sym_edges[tri_symedges[face_i].x].vertex];
	face_v[1] = point_positions[sym_edges[sym_edges[tri_symedges[face_i].x].nxt].vertex];
	face_v[2] = point_positions[sym_edges[sym_edges[sym_edges[tri_symedges[face_i].x].nxt].nxt].vertex];

	return (face_v[0] + face_v[1] + face_v[2]) / 3.f; 
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

bool orientation(in vec2 p1 , in vec2 p2 , in vec2 p3)
{
	float val = (p2.y - p1.y) * (p3.x - p2.x) - (p2.x - p1.x) * (p3.y - p2.y);
	return (val > 0.0f) ? true : false;
}

bool line_line_test(in vec2 s1p1 , in vec2 s1p2, in vec2 s2p1, in vec2 s2p2)
{
	bool hit = false;
	bool o1 = orientation(s1p1, s1p2, s2p1);
	bool o2 = orientation(s1p1, s1p2, s2p2);
	bool o3 = orientation(s2p1, s2p2, s1p1);
	bool o4 = orientation(s2p1, s2p2, s1p2);

	if (o1 != o2 && o3 != o4)
		hit = true;
	else
		hit = false;

	return hit;
}

vec2 project_point_on_line(vec2 point, vec2 a, vec2 b)
{
	vec2 ab = b - a;
	vec2 ap = point - a;
	return a + dot(ap, ab) / dot(ab, ab) * ab;
}

bool check_side(vec2 direction, vec2 other)
{
	return dot(direction, other) > 0.f ? true : false;
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
	const vec2 point_array[5] = vec2[5](p1, p2, p3, p4, p5);
	bool return_value = true;

	for (int i = 0; i < num; i++)
	{
		vec2 line = point_array[(i + 1) % num] - point_array[i];

		// rotate vector by 90 degrees
		{
			float tmp = line.x;
			line.x = -line.y;
			line.y = tmp;
		}

		for (int j = 0; j < num - 2; j++)
			return_value = !return_value || check_side(line, point_array[(i + 2 + j) % num] - point_array[(i + 1) % 5]); 	
	}
	return return_value;
}

//-----------------------------------------------------------
// Intersection Functions
//-----------------------------------------------------------

bool segment_triangle_test(vec2 p1, vec2 p2, vec2 t1, vec2 t2, vec2 t3)
{
	bool test_one, test_two = false;

	test_one = point_triangle_test(p1, t1, t2, t3) && point_triangle_test(p2, t1, t2, t3);
		
	test_two = line_line_test(p1, p2, t1, t2) ||
		line_line_test(p1, p2, t2, t3) ||
		line_line_test(p1, p2, t3, t1);

	return test_one || test_two;
}

bool point_intersects_line(vec2 p, vec2 a, vec2 b, float epsilon = EPSILON)
{
	float hypotenuse = length(project_point_on_line(p, a, b) - a);
	float adjacent = length(p - a);

	return sqrt(hypotenuse * hypotenuse - adjacent * adjacent) < epsilon;
}

//-----------------------------------------------------------
// Functions
//-----------------------------------------------------------

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

int oriented_walk_point(int curr_e, int goal_point_i)
{
	vec2 tri_cent;
	vec2 goal_point = get_vertex(goal_point_i);
	int i = 0;
	while (i != 3)
	{
		if (face_contains_vertice(get_symedge(curr_e).face, goal_point_i))
			return get_face_vertex_symedge(get_symedge(curr_e).face, goal_point_i);
		
		tri_cent = get_face_center(sym_edges[curr_e].face);
		
		// Loop through edges to see if we should continue through the edge
		// to the neighbouring triangle 
		bool line_line_hit = false;
		for (i = 0; i < 3; i++)
		{
			if (sym_edges[sym_edges[curr_e].nxt].rot == -1)
			{
				curr_e = sym_edges[curr_e].nxt;
				continue;
			}

			line_line_hit = line_line_test(
				tri_cent,
				goal_point,
				point_positions[sym_edges[curr_e].vertex],
				point_positions[sym_edges[sym_edges[curr_e].nxt].vertex]);

			if (line_line_hit)
			{
				curr_e = sym_edges[sym_edges[curr_e].nxt].rot;
				break;
			}
			curr_e = sym_edges[curr_e].nxt;
		}
	}
	return -1;
}

bool pre_candidate_check(SymEdge s)
{
	vec2 p1 = get_vertex(get_symedge(get_symedge(s.nxt).nxt).vertex);
	vec2 e1 = get_vertex(s.vertex);
	vec2 e2 = get_vertex(get_symedge(s.nxt).vertex);
	vec2 p2 = get_vertex(get_symedge(get_symedge(sym_symedge(s).nxt).nxt).vertex);
	return polygonal_is_strictly_convex(4, p1, e1, p2, e2); 
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

int points_connected(int e1, int e2)
{
	int curr_e = e1;
	bool reverse_direction = false;
	int other_vertex = get_symedge(e2).vertex;

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

bool face_contains_vertex(int vert, SymEdge s_triangle)
{
	ivec3 tri = ivec3(s_triangle.vertex, get_symedge(s_triangle.nxt).vertex, prev_symedge(s_triangle).vertex);
	return vert == tri.x || vert == tri.y || vert == tri.z;
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

bool will_be_flipped(int segment_index, SymEdge triangle)
{
	if (tri_seg_inters_index[triangle.face] == segment_index && tri_seg_inters_index[sym_symedge(triangle).face] == segment_index)
	{
		edge_label[triangle.edge] = 2;
		return true;
	}

	return false;
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

void straight_walk(int segment_index, SymEdge s_starting_point, int ending_point_i)
{
	SymEdge cur_edge = s_starting_point;
	SymEdge prev_intersecting_edge = s_starting_point;
	vec2 constraint_edge[2];
	constraint_edge[0] = get_vertex(s_starting_point.vertex);
	constraint_edge[1] = get_vertex(ending_point_i);
	vec2 normalized_constrained_edge = normalize(constraint_edge[1] - constraint_edge[0]);

	while (true)
	{
		vec2 v0 = get_vertex(get_symedge(cur_edge.nxt).vertex);
		vec2 v1 = get_vertex(prev_symedge(cur_edge).vertex);
		if (line_line_test(constraint_edge[0], constraint_edge[1], v0, v1))
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

		if (checks == 2) // something is wrong
			return;
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
	int endpoints_inserted = point_inserted[seg_endpoint_indices[index].x] * point_inserted[seg_endpoint_indices[index].y];
	
	// Check if the segment has not been inserted and if both endpoints has been inserted
	if (index > 3 && index < seg_inserted.length() && seg_inserted[index] == 0 && endpoints_inserted == 1)
	{
		// TODO: starting at the first symedge might not always be preferred, find a better solution
		int starting_symedge = oriented_walk_point(0, seg_endpoint_indices[index].x);
		int ending_symedge = oriented_walk_point(starting_symedge, seg_endpoint_indices[index].y);

		int connecting_edge = points_connected(starting_symedge, ending_symedge);
		if (connecting_edge != -1)
		{
			edge_is_constrained[connecting_edge] = 1;
			edge_label[connecting_edge] = 0;
		}
		else
		{
			straight_walk(index, get_symedge(starting_symedge), seg_endpoint_indices[index].y);
			straight_walk(index, get_symedge(ending_symedge), seg_endpoint_indices[index].x);
		}
	}
}