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
#define FLT_MAX 3.402823466e+38

// Each thread represents one triangle
void main(void)
{
	uint gid = gl_GlobalInvocationID.x;
	int index = int(gid);
	int num_threads = int(gl_NumWorkGroups.x * gl_WorkGroupSize.x);
	while(index < num_tris)
	{
		
		if(tri_symedges[index].x != -1)
		{
			vec2 tri_points[3];
			get_face(index, tri_points);
			// calculate the centroid of the triangle
			vec2 tri_cent = (tri_points[0] +  tri_points[1] +  tri_points[2]) / 3.0f;
			int point_index = -1;
			float best_dist = FLT_MAX;
			// Figure out which point should be the new point of this triangle
			for(int i = 0; i < num_points; i++)
			{
				// Check so it is an uninserted point.
				if(point_inserted[i] == 0)
				{
					vec2 pos = point_positions[i];
					float dist = distance(pos, tri_cent);
					if(dist < best_dist)
					{
						best_dist = dist;
						point_index = i;
					}	
				}
			}
			tri_ins_point_index[index] = point_index;
		}
		index += num_threads;
	}

}