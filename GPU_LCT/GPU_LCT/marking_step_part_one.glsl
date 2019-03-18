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
	int tri_vertex_indices[];
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

int nxt_symedge(int index)
{
	return sym_edges[index].nxt;
}

int rot_symedge(int index)
{
	return sym_edges[index].rot;
}

int crot_symedge(int index)
{
	int sym = rot_symedge(nxt_symedge(index));
	return sym != -1 ? nxt_symedge(sym) : -1;
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

		for (int j = 0; j < 3; j++)
			return_value = !return_value || check_side(line, point_array[(i + 2 + j) % num] - point_array[(i + j) % 5]); 	
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

//-----------------------------------------------------------
// Functions
//-----------------------------------------------------------

void oriented_walk_point(inout int curr_e, in vec2 goal_point, out bool on_edge, out vec2 tri_cent)
{
	bool done = false;
	int iter = 0;
	on_edge = false;
	while(!done){
		on_edge = false;
		// Loop through triangles edges to check if point is on the edge 
		for(int i = 0; i < 3; i++)
		{
			bool hit = false;
			point_line_test(goal_point,
				point_positions[sym_edges[curr_e].vertex],
				point_positions[sym_edges[sym_edges[curr_e].nxt].vertex],
				hit);
			if(hit)
			{
				on_edge = true;
				return;
			}
			curr_e = sym_edges[curr_e].nxt;
		}
		// calculate triangle centroid
		vec2 tri_points[3];
		get_face(sym_edges[curr_e].face, tri_points);
		tri_cent = (tri_points[0] +  tri_points[1] +  tri_points[2]) / 3.0f;
		// Loop through edges to see if we should continue through the edge
		// to the neighbouring triangle 
		bool line_line_hit = false;
		for(int i = 0; i < 3; i++)
		{
			line_line_test(
			tri_cent,
			goal_point,
			point_positions[sym_edges[curr_e].vertex],
			point_positions[sym_edges[sym_edges[curr_e].nxt].vertex],
			line_line_hit);

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
		if (point_positions[sym_edges[nxt_symedge(curr_e)].vertex] == other_vertex)
			return true;

		else
		{
			if (!reverse_direction)
			{
				if (sym_edges[curr_e].rot == edge)
					return false;

				if (sym_edges[curr_e].rot != -1)
					curr_e = rot_symedge(curr_e);
				else
				{
					reverse_direction = true;
					curr_e = crot_symedge(edge);
					if (curr_e == -1)
						return false;
				}
			}
			else
			{
				curr_e = crot_symedge(edge);
				if (curr_e == -1)
					return true;
			}
		}
	}
}

void main(void)
{
	// Each thread is responsible for a segment

	uint gid = gl_GlobalInvocationID.x;
	int index = int(gid);
	int endpoints_inserted = point_inserted[seg_endpoint_indices[index]] * point_inserted[seg_endpoint_indices[index + 1]];

	// Check if the segment has not been inserted and if both endpoints has been inserted
	if (index < num_points && seg_inserted[index] == 0 && endpoints_inserted == 1)
	{
		bool on_edge;
		vec2 tri_cent;
		
		// TODO: starting at the first symedge might not always be preferred, find a better solution
		int edge = 0;

		oriented_walk(edge, point_positions[seg_endpoint_indices[index]], on_edge, tri_cent);

		if (points_connected(edge, point_positions[seg_endpoint_indices[index + 1]]))
		{
			// mark edge as constrained
		}
		else
		{
			
		}
	}
}