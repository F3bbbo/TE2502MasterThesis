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
// Uniforms
//-----------------------------------------------------------
layout (std140, binding = 1) uniform epsilon_buff
{
	float EPSILON;
};
//-----------------------------------------------------------
// Access Functions
//-----------------------------------------------------------
	void get_face(int face_i, out vec2 face_v[3])
	{
		face_v[0] = point_positions[sym_edges[tri_symedges[face_i].x].vertex];
		face_v[1] = point_positions[sym_edges[sym_edges[tri_symedges[face_i].x].nxt].vertex];
		face_v[2] = point_positions[sym_edges[sym_edges[sym_edges[tri_symedges[face_i].x].nxt].nxt].vertex];
	}

//-----------------------------------------------------------
// Symedge functions
//-----------------------------------------------------------
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

//-----------------------------------------------------------
// Functions
//-----------------------------------------------------------
void oriented_walk(inout int curr_e, int point_i, out bool on_edge)
{
	bool done = false;
	vec2 goal = point_positions[point_i];
	int iter = 0;
	on_edge = false;
	while (!done) {
		on_edge = false;
		// Loop through triangles edges to check if point is on the edge 
		for (int i = 0; i < 3; i++)
		{
			bool hit = false;
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
		vec2 tri_cent = (tri_points[0] + tri_points[1] + tri_points[2]) / 3.0f;
		// Loop through edges to see if we should continue through the edge
		// to the neighbouring triangle 
		bool line_line_hit = false;
		for (int i = 0; i < 3; i++)
		{
			line_line_hit = line_seg_intersection_ccw(
				tri_cent,
				goal,
				point_positions[sym_edges[curr_e].vertex],
				point_positions[sym_edges[sym_edges[curr_e].nxt].vertex]);

			if (line_line_hit)
			{
				break;
			}
			curr_e = sym_edges[curr_e].nxt;
		}

		if (line_line_hit)
		{
			curr_e = nxt(sym_edges[sym_edges[curr_e].nxt].rot); // sym
		}
		else
		{
			return;
		}
	}
}

// Each thread represents one point
void main(void)
{
	uint gid = gl_GlobalInvocationID.x;
	int index = int(gid);
	int num_threads = int(gl_NumWorkGroups.x * gl_WorkGroupSize.x);
	while(index < point_positions.length())
	{
		
		if (point_inserted[index] == 0)
		{
			bool on_edge;
			// find out which triangle the point is now
			int curr_e = tri_symedges[point_tri_index[index]].x;;
			oriented_walk(
				curr_e,
				index,
				on_edge);
			if (on_edge)
			{
				int sym_e = sym(curr_e);
				if (sym_e > -1)
				{
					// if neighbour triangle has a lower index 
					// chose that one as the triangle for the point.
					if (sym_edges[sym_e].face < sym_edges[curr_e].face)
					{
						curr_e = sym_e;
					}
				}
			}
			point_tri_index[index] = sym_edges[curr_e].face;
		}
		index += num_threads;
		
	}

}