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
		
	test_two = line_seg_intersection_ccw(p1, p2, t1, t2) ||
		line_seg_intersection_ccw(p1, p2, t2, t3) ||
		line_seg_intersection_ccw(p1, p2, t3, t1);

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

void oriented_walk_point(inout int curr_e, in vec2 goal_point, out vec2 tri_cent)
{
	bool done = false;
	int iter = 0;
	while(!done){
		// calculate triangle centroid
		vec2 tri_points[3];
		get_face(sym_edges[curr_e].face, tri_points);
		tri_cent = (tri_points[0] +  tri_points[1] +  tri_points[2]) / 3.0f;
		// Loop through edges to see if we should continue through the edge
		// to the neighbouring triangle 
		bool line_line_hit = false;
		for(int i = 0; i < 3; i++)
		{
			line_line_hit = line_seg_intersection_ccw(
				tri_cent,
				goal_point,
				point_positions[sym_edges[curr_e].vertex],
				point_positions[sym_edges[sym_edges[curr_e].nxt].vertex]);

			if(line_line_hit)
			{	
				break;
			}
			curr_e = sym_edges[curr_e].nxt;
		}

		if(line_line_hit)
		{	
			curr_e = sym_edges[sym_edges[curr_e].nxt].rot; // sym
		}
		else
		{
			return;
		}
	}
}

bool pre_candidate_check(vec2 p1, vec2 e2, vec2 e3, vec2 p2)
{
	return polygonal_is_strictly_convex(4, p1, e2, e3, p2); 
}

bool pre_candidate_check(SymEdge s)
{
	vec2 p1 = get_vertex(get_symedge(get_symedge(s.nxt).nxt).vertex);
	vec2 e2 = get_vertex(s.vertex);
	vec2 e3 = get_vertex(get_symedge(s.nxt).vertex);
	vec2 p2 = get_vertex(get_symedge(get_symedge(sym_symedge(s).nxt).nxt).vertex);
	return polygonal_is_strictly_convex(4, p1, e2, e3, p2); 
}

bool first_candidate_check(vec2 s1, vec2 s2, vec2 p1, vec2 e1, vec2 e2, vec2 p2)
{
	// input parameters forms a quadrilateral
	bool intersects_first = segment_triangle_test(s1, s2, p1, e1, p2);
	bool intersects_second = segment_triangle_test(s1, s2, p1, e2, p2);
	return intersects_first ^^ intersects_second;
}

bool second_candidate_check(vec2 p1, vec2 e1, vec2 e2, vec2 e3, vec2 p2)
{
	// input parameters forms a pentagonal polygonal
	return polygonal_is_strictly_convex(5, p1, e1, e2, e3, p2);
}

bool third_candidate_check(vec2 p1, vec2 e1, vec2 e2, vec2 e3, vec2 p2)
{
	// Assumes that the input points are ordered starting from the triangle with the lowest index.
	return !polygonal_is_strictly_convex(4, p1, e1, e2, e3) && polygonal_is_strictly_convex(4, p1, e1, e2, p2) == true ? true : false;
}

bool points_connected(int edge, vec2 other_vertex)
{
	int curr_e = edge;
	bool reverse_direction = false;
	
	while (true)
	{
		if (point_positions[get_symedge(sym_edges[curr_e].nxt).vertex] == other_vertex)
			return true;

		else
		{
			if (!reverse_direction)
			{
				if (sym_edges[curr_e].rot == edge)
					return false;

				if (sym_edges[curr_e].rot != -1)
					curr_e = get_symedge(curr_e).rot;
				else
				{
					reverse_direction = true;
					curr_e = crot_symedge_i(get_symedge(edge));
					if (curr_e == -1)
						return false;
				}
			}
			else
			{
				curr_e = crot_symedge_i(get_symedge(edge));
				if (curr_e == -1)
					return true;
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
	vec2 s1 = get_vertex(seg_endpoint_indices[segment_index]);
	vec2 s2 = get_vertex(seg_endpoint_indices[segment_index + 1]);
	return true;
}

void process_triangle(int segment_index, SymEdge triangle)
{
	// Assumes that the provided symedge is the symedge between two triangles
	if (get_label(triangle.edge) != 3 &&
		get_label(get_symedge(triangle.nxt).edge) != 3 &&
		get_label(prev_symedge(triangle).edge) != 3 &&
		tri_seg_inters_index[triangle.face] > segment_index)
		{
			tri_seg_inters_index[triangle.face] = segment_index;
			if (tri_seg_inters_index[triangle.face] == segment_index && tri_seg_inters_index[sym_symedge(triangle).face] == segment_index)
			{
				tri_edge_flip_index[triangle.face] = triangle.edge;
				tri_edge_flip_index[sym_symedge(triangle).face] = triangle.edge;
				edge_label[triangle.edge] = 2;
			}
		}
}

void straight_walk(int segment_index, SymEdge s_starting_point, SymEdge s_ending_point)
{
	SymEdge cur_edge = s_starting_point;
	SymEdge prev_inter_edge = s_starting_point;
	vec2 constraint_edge[2];
	constraint_edge[0] = get_vertex(s_starting_point.vertex);
	constraint_edge[1] = get_vertex(s_ending_point.vertex);
	vec2 normalized_constrained_edge = normalize(constraint_edge[1] - constraint_edge[0]);

	while (true)
	{
		vec2 v0 = get_vertex(get_symedge(cur_edge.nxt).vertex);
		vec2 v1 = get_vertex(prev_symedge(cur_edge).vertex);
		if (line_seg_intersection_ccw(constraint_edge[0], constraint_edge[1], v0, v1))
		{
			cur_edge = get_symedge(cur_edge.nxt);
			process_triangle(segment_index, cur_edge);
			if (pre_candidate_check(cur_edge))
				return;
			cur_edge = get_symedge(sym_symedge(cur_edge).nxt);
			prev_inter_edge = cur_edge;
			break;
		}
		cur_edge = get_symedge(cur_edge.rot);
		if (cur_edge == s_starting_point)
			return;
	}

	// walk towards the constraint endpoins and stop if we reach the triangle that contains the segment endpoint
	while (true)
	{
		// Check if we have arrived to a triangle that contains the segment endpoint
		if (face_contains_vertex(s_ending_point.vertex, cur_edge))
		{
			process_triangle(segment_index, prev_symedge(cur_edge));
			return;
		}

		int checks = 0;
		while (checks < 2)
		{
			// Checks if the segment intersects an edge
			vec2 v0 = get_vertex(cur_edge.vertex);
			vec2 v1 = get_vertex(get_symedge(cur_edge.nxt).vertex);
				
			if (line_seg_intersection_ccw(constraint_edge[0], constraint_edge[1], v0, v1))
			{
				process_triangle(segment_index, cur_edge);
				if (Qi_check(segment_index, prev_inter_edge, cur_edge))
					return;
				prev_inter_edge = cur_edge;
				cur_edge = get_symedge(sym_symedge(cur_edge).nxt);
				break;
			}

			cur_edge = get_symedge(cur_edge.nxt);
			checks++;
		}

		if (checks == 2)
			return;
	}
}

void main(void)
{
	// Each thread is responsible for a segment

	uint gid = gl_GlobalInvocationID.x;
	int index = int(gid);
	int endpoints_inserted = point_inserted[seg_endpoint_indices[index]] * point_inserted[seg_endpoint_indices[index + 1]];

	// Check if the segment has not been inserted and if both endpoints has been inserted
	if (index < seg_inserted.length() && seg_inserted[index] == 0 && endpoints_inserted == 1)
	{
		vec2 tri_cent;
		
		// TODO: starting at the first symedge might not always be preferred, find a better solution
		int edge = 0;

		oriented_walk_point(edge, point_positions[seg_endpoint_indices[index]], tri_cent);

		if (points_connected(edge, point_positions[seg_endpoint_indices[index + 1]]))
		{
			edge_is_constrained[edge] = 1;
			edge_label[edge] = 0;
		}
		else
		{
			SymEdge s = get_symedge(edge);
			straight_walk(index, s, get_symedge(s.nxt));
			straight_walk(index, get_symedge(s.nxt), s);
		}
	}
}