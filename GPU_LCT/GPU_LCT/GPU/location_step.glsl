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
	int seg_endpoint_indices[];
};

layout(std430, binding = 6) buffer Seg1
{
	int seg_inserted[];
};

layout(std430, binding = 7) buffer Tri_buff_0
{
	int tri_vertex_indices[];
};
layout(std430, binding = 8) buffer Tri_buff_1
{
	ivec3 tri_symedges[];
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
// Uniforms
//-----------------------------------------------------------
layout (std140, binding = 0) uniform Sizes
{
	int num_tris;
	int num_points;
	vec2 pad;
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
// Intersection Functions
//-----------------------------------------------------------
#define EPSILON 0.0005f

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

void orientation(in vec2 p1 , in vec2 p2 , in vec2 p3, out bool clockwise)
{
	float val = (p2.y - p1.y) * (p3.x - p2.x) - (p2.x - p1.x) * (p3.y - p2.y);
	clockwise = (val > 0.0f) ? true : false;
}

void line_line_test(in vec2 s1p1 , in vec2 s1p2, in vec2 s2p1, in vec2 s2p2, out bool hit)
{
	hit = false;
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
void oriented_walk(inout int curr_e,in int point_i, out bool on_edge, out vec2 tri_cent)
{
	bool done = false;
	vec2 goal = point_positions[point_i];
	int iter = 0;
	on_edge = false;
	while(!done){
		on_edge = false;
		// Loop through triangles edges to check if point is on the edge 
		for(int i = 0; i < 3; i++)
		{
			bool hit = false;
			point_line_test(goal,
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
			goal,
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
//	iter++;
//	if(iter > 10)
//		//symedge_i = curr_e;
//		break;
	}
}


void main(void)
{
	uint gid = gl_GlobalInvocationID.x;
	int index = int(gid);
	point_tri_index[index] = index;
	if(index < num_points)
	{
		
		if(point_inserted[index] == 0)
		{
			bool on_edge;
			vec2 tri_cent;
			// find out which triangle the point is now
			int curr_e = tri_symedges[point_tri_index[index]].x;;
			oriented_walk(
				curr_e,
				index,
				on_edge,
				tri_cent);
			point_tri_index[index] = sym_edges[curr_e].face;
			// TODO: figure out if point should be the new point of the triangle 
		}
		
	}

}