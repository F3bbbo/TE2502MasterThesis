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
layout(std430, binding = 13) buffer new_points_buff
{
	vec2 new_points[];
};





// Each thread represents one triangle
void main(void)
{
	gid = gl_GlobalInvocationID.x;
	int index = int(gid);
	int num_threads = int(gl_NumWorkGroups.x * gl_WorkGroupSize.x);
	while(index < tri_seg_inters_index.length())
	{
		NewPoint new_point = refine_points[index];
		if (new_point.index >= 0)
		{
			new_points[new_point.index] = new_point.pos;
			// reset the insert point data structure
			new_point.pos = vec2(0.0f);
			new_point.index = -1;
			new_point.face_i = -1;
			refine_points[index] = new_point;
		}
		index += num_threads;
	}
	
}