#version 430
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
layout (std140, binding = 0) uniform symedge_size
{
	int symedge_buffer_size;
};

//-----------------------------------------------------------
// Access Functions
//-----------------------------------------------------------
void get_face(in int face_i, out vec2 face_v[3])
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
// Each thread represents one triangle
void main(void)
{
	uint gid = gl_GlobalInvocationID.x;
	int num_threads = int(gl_NumWorkGroups.x * gl_WorkGroupSize.x);
	int index = int(gid);
	while(index < tri_seg_inters_index.length())
	{
		// If triangle has a point assigned to it add the point to it
		int point_index = tri_ins_point_index[index];
		if (point_index > -1 /*&& is_valid_face(index)*/)
		{
			status = 1;
			//if (!is_valid_face(index))
			//	break;

			// Create array of the indices of the three new triangles
			int tri_indices[3];
			tri_indices[0] = index;
			tri_indices[1] = tri_seg_inters_index.length() - 2 * (point_positions.length() - point_index);
			tri_indices[2] = tri_seg_inters_index.length() - 2 * (point_positions.length() - point_index) + 1;
			int edge_indices[3];
			edge_indices[0] = edge_label.length() - 3 * (point_positions.length() - point_index);
			edge_indices[1] = edge_label.length() - 3 * (point_positions.length() - point_index) + 1;
			edge_indices[2] = edge_label.length() - 3 * (point_positions.length() - point_index) + 2;
			int sym_edge_indices[6];
			sym_edge_indices[0] = symedge_buffer_size - 6 * (point_positions.length() - point_index);
			sym_edge_indices[1] = symedge_buffer_size - 6 * (point_positions.length() - point_index) + 1;
			sym_edge_indices[2] = symedge_buffer_size - 6 * (point_positions.length() - point_index) + 2;
			sym_edge_indices[3] = symedge_buffer_size - 6 * (point_positions.length() - point_index) + 3;
			sym_edge_indices[4] = symedge_buffer_size - 6 * (point_positions.length() - point_index) + 4;
			sym_edge_indices[5] = symedge_buffer_size - 6 * (point_positions.length() - point_index) + 5;

			// start working on the new triangles
			int orig_face[3];
			int orig_sym[3];
			// save edges of original triangle
			int curr_e = tri_symedges[index].x;
			for (int i = 0; i < 3; i++)
			{
				orig_face[i] = curr_e;
				int sym = curr_e;
				// sym operations
				sym = nxt(sym);
				sym = rot(sym);
				orig_sym[i] = sym;
				// move curr_e to next edge of triangle
				curr_e = nxt(curr_e);
			}
			int insert_point = tri_ins_point_index[index];
			// Create symedge structure of the new triangles
			for (int i = 0; i < 3; i++)
			{
				ivec4 tri_syms;
				int next_id = (i + 1) % 3;
				tri_syms.x = orig_face[i];
				tri_syms.y = sym_edge_indices[2 * i];
				tri_syms.z = sym_edge_indices[2 * i + 1];
				tri_syms.w = -1;
				// fix the first symedge of the triangle
				sym_edges[tri_syms.y].vertex = sym_edges[orig_face[next_id]].vertex;
				sym_edges[tri_syms.y].nxt = tri_syms.z;
				// fix the second symedge of the triangle
				sym_edges[tri_syms.z].vertex = insert_point;
				sym_edges[tri_syms.z].nxt = tri_syms.x;

				sym_edges[tri_syms.x].nxt = tri_syms.y;
				//sym_edges[tri_syms.x].nxt = 1;
				// add face index to symedges in this face
				sym_edges[tri_syms.x].face = tri_indices[i];
				sym_edges[tri_syms.y].face = tri_indices[i];
				sym_edges[tri_syms.z].face = tri_indices[i];

				// add symedges to current face
				tri_symedges[tri_indices[i]] = tri_syms;
			}
			// connect the new triangles together
			for (int i = 0; i < 3; i++)
			{
				curr_e = orig_face[i];
				int next_id = (i + 1) % 3;
				int new_edge = edge_indices[i];
				// get both symedges of one new inner edge
				int inner_edge = orig_face[i];
				inner_edge = nxt(inner_edge);
				int inner_edge_sym = orig_face[next_id];
				inner_edge_sym = nxt(inner_edge_sym);
				inner_edge_sym = nxt(inner_edge_sym);
				// set same edge index to both symedges
				sym_edges[inner_edge].edge = new_edge;
				sym_edges[inner_edge_sym].edge = new_edge;
				// connect the edges syms together
				int rot_connect_edge = inner_edge;
				rot_connect_edge = nxt(rot_connect_edge);
				sym_edges[rot_connect_edge].rot = inner_edge_sym;
				int rot_connect_edge_sym = inner_edge_sym;
				rot_connect_edge_sym = nxt(rot_connect_edge_sym);
				sym_edges[rot_connect_edge_sym].rot = inner_edge;
				// connect original edge with its sym
				sym_edges[inner_edge].rot = orig_sym[i];
			}
			// Mark original edges as potential not delaunay 
			// or as point on edge if the point is on any of the edges 
			for (int i = 0; i < 3; i++)
			{
				if (edge_is_constrained[sym_edges[orig_face[i]].edge] < 0)
				{
					// Check if the point is on the edge
					vec2 s1 = point_positions[sym_edges[orig_face[i]].vertex];
					vec2 s2 = point_positions[sym_edges[orig_face[(i + 1) % 3]].vertex];
					vec2 p = point_positions[point_index];
					if (point_line_test(p, s1, s2))
					{
						edge_label[sym_edges[orig_face[i]].edge] = 3;
					}
					else if (edge_label[sym_edges[orig_face[i]].edge] < 1)
					{
						edge_label[sym_edges[orig_face[i]].edge] = 1; // candidate for not delaunay. 
					}
				}
			}
			// Set point as inserted
			point_inserted[point_index] = 1;
			tri_ins_point_index[index] = -1;
		}
		index += num_threads;
	}

}
