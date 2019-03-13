#version 430
layout(local_size_x = 1, local_size_y= 1) in;

struct SymEdge{
	int nxt;
	int rot;

	int vertex;
	int edge;
	int face;
};


layout(std140, binding = 0) buffer Point0
{
	vec2 positions[];
};

layout(std140, binding = 1) buffer Points1
{
	int inserted[];
};

layout(std140, binding = 2) buffer Points2
{
	int tri_index[];
};

layout(std140, binding = 3) buffer Edge0
{
	int label[];
};

layout(std140, binding = 4) buffer Edge1
{
	int is_constrained[];
};

layout(std140, binding = 5) buffer Seg0
{
	int endpoint_indices[];
};

layout(std140, binding = 6) buffer Seg1
{
	int inserted[];
};

layout(std140, binding = 7) buffer Tri_buff_0
{
	int vertex_indices[];
};
layout(std140, binding = 8) buffer Tri_buff_1
{
	SymEdge symedges[];
};
layout(std140, binding = 9) buffer Tri_buff_2
{
	int ins_point_index[];
};
layout(std140, binding = 10) buffer Tri_buff_3
{
	int seg_inters_index[];
};
layout(std140, binding = 11) buffer Tri_buff_4
{
	int edge_flip_index[];
};


void main(void)
{
	uint gid = gl_GlobalInvocationID.x;

}