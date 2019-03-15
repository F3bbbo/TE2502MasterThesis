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
// Uniforms
//-----------------------------------------------------------
layout (std140, binding = 0) uniform Sizes
{
	int num_tris;
	int num_points;
};


//-----------------------------------------------------------
// Access Functions
//-----------------------------------------------------------
void get_face(in int face_i, out vec2 face_v[3])
{
	face_v[0] = point_positions[tri_symedges[face_i].x];
	face_v[1] = point_positions[tri_symedges[face_i].y];
	face_v[2] = point_positions[tri_symedges[face_i].z];
}

//-----------------------------------------------------------
// Intersection Functions
//-----------------------------------------------------------
#define EPSILON 0.00005f

void point_line_test(in vec2 p, in vec2 s1, in vec2 s2, out bool hit)
{
	vec3 v1 = vec3(s1 - p, 0.0f);
	vec3 v2 = vec3(s1 - s2, 0.0f);
	if (abs(length(cross(v1, v2))) > EPSILON)
	{
		hit = false;
		return;
	}
	float dot_p = dot(v1, v2);
	if (dot_p < EPSILON)
	{
		hit = false;
		return;
	}
	if (dot_p > (dot(v2, v2) - EPSILON))
	{
		hit = false;
		return;
	}
	hit = true;
}

void orientation(in vec2 p1 , in vec2 p2 , in vec2 p3, out bool is_ccw)
{
	float val = (p2.y - p1.y) * (p3.x - p2.x) - (p2.x - p1.x) * (p3.y - p2.y);
	is_ccw = (val > 0.0f) ? true : false;
}

void line_line_test(in vec2 s1p1 , in vec2 s1p2, in vec2 s2p1, in vec2 s2p2, out bool hit)
{
	bool o1;
	orientation(s1p1, s1p2, s2p1, o1);
	bool o2;
	orientation(s1p1, s1p2, s2p2, o2);
	bool o3;
	orientation(s2p1, s2p2, s1p1, o3);
	bool o4;
	orientation(s2p1, s2p2, s1p2, o4);

	if (o1 != o2 && o3 != o4)
		hit = true;
	else
		hit = false;
}

//-----------------------------------------------------------
// Functions
//-----------------------------------------------------------
void oriented_walk(inout int symedge_i,in int point_i, out bool on_edge)
{
	bool done = false;
	int curr_e = symedge_i;
	vec2 goal = point_positions[point_i];
	while(!done){
		// Loop through triangles edges to check if point is on the edge 
		for(int i = 0; i < 3; i++)
		{
			bool hit;
			point_line_test(goal,
				point_positions[sym_edges[curr_e].vertex],
				point_positions[sym_edges[sym_edges[curr_e].nxt].vertex],
				hit);
			if(hit)
			{
				on_edge = true;
				symedge_i = curr_e;
				return;
			}
			curr_e = sym_edges[curr_e].nxt;
		}
		// calculate triangle centroid
		vec2 tri_points[3];
		get_face(sym_edges[curr_e].face, tri_points);
		vec2 tri_cent = (tri_points[0] +  tri_points[1] +  tri_points[2]) / 3.0f;

		// Loop through edges to see if we should continue through the edge
		// to the neighbouring triangle 
		bool hit;
		for(int i = 0; i < 3; i++)
		{
			line_line_test(
			tri_cent,
			goal,
			point_positions[sym_edges[curr_e].vertex],
			point_positions[sym_edges[sym_edges[curr_e].nxt].vertex],
			hit);

			if(hit)
			{
				break;
			}
			curr_e = sym_edges[curr_e].nxt;
		}
		if(hit)
		{
			curr_e = sym_edges[sym_edges[curr_e].nxt].rot; // sym
		}
		else
		{
			on_edge = false;
			return;
		}
	}
}


void main(void)
{
	uint gid = gl_GlobalInvocationID.x;

	point_positions[gid];


}