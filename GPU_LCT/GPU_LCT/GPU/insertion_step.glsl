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

//-----------------------------------------------------------
// Access Functions
//-----------------------------------------------------------
void get_face(in int face_i, out vec2 face_v[3])
{
	face_v[0] = point_positions[sym_edges[tri_symedges[face_i].x].vertex];
	face_v[1] = point_positions[sym_edges[sym_edges[tri_symedges[face_i].x].nxt].vertex];
	face_v[2] = point_positions[sym_edges[sym_edges[sym_edges[tri_symedges[face_i].x].nxt].nxt].vertex];
}

// symedge movement functions
void nxt(inout int sym_edge)
{
	sym_edge = sym_edges[sym_edge].nxt;
}

void rot(inout int sym_edge)
{
	sym_edge = sym_edges[sym_edge].rot;
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
		if(point_index > -1 )
		{
			status = 1;

			// Create array of the indices of the three new triangles
			int tri_indices[3];
			tri_indices[0] = index;
			tri_indices[1] = 2 * point_index - 6;
			tri_indices[2] = 2 * point_index - 5;
			int edge_indices[3];
			edge_indices[0] = 3 * point_index - 7;
			edge_indices[1] = 3 * point_index - 6;
			edge_indices[2] = 3 * point_index - 5;
			int sym_edge_indices[6];
			sym_edge_indices[0] = 6 * point_index - 18;
			sym_edge_indices[1] = 6 * point_index - 17;
			sym_edge_indices[2] = 6 * point_index - 16;
			sym_edge_indices[3] = 6 * point_index - 15;
			sym_edge_indices[4] = 6 * point_index - 14;
			sym_edge_indices[5] = 6 * point_index - 13;

			// start working on the new triangles
			int orig_face[3];
			int orig_sym[3];
			// save edges of original triangle
			int curr_e = tri_symedges[index].x;
			for(int i = 0; i < 3; i++)
			{
				orig_face[i] = curr_e;
				int sym = curr_e;
				// sym operations
				nxt(sym);
				rot(sym);
				orig_sym[i] = sym;
				// move curr_e to next edge of triangle
				nxt(curr_e);
			}
			int insert_point = tri_ins_point_index[index];
			// Create symedge structure of the new triangles
			for(int i = 0; i < 3; i++)
			{
				ivec4 tri_syms;			
				int next_id = (i + 1) % 3;
				tri_syms.x = orig_face[i];
				tri_syms.y = sym_edge_indices[2*i];
				tri_syms.z = sym_edge_indices[2*i + 1];
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
			for(int i = 0; i < 3; i++)
			{
				curr_e = orig_face[i];
				int next_id = (i + 1) % 3;
				int new_edge = edge_indices[i];
				// get both symedges of one new inner edge
				int inner_edge = orig_face[i];
				nxt(inner_edge);
				int inner_edge_sym = orig_face[next_id];
				nxt(inner_edge_sym);
				nxt(inner_edge_sym);
				// set same edge index to both symedges
				sym_edges[inner_edge].edge = new_edge;
				sym_edges[inner_edge_sym].edge = new_edge;
				// connect the edges syms together
				int rot_connect_edge = inner_edge;
				nxt(rot_connect_edge);
				sym_edges[rot_connect_edge].rot = inner_edge_sym;
				int rot_connect_edge_sym = inner_edge_sym;
				nxt(rot_connect_edge_sym);
				sym_edges[rot_connect_edge_sym].rot = inner_edge;
				// connect original edge with its sym
				sym_edges[inner_edge].rot = orig_sym[i];
			}
			// Mark original edges as potential not delaunay
			for(int i = 0; i < 3; i++)
			{
				if(edge_is_constrained[sym_edges[orig_face[i]].edge] == 0)
					if(edge_label[sym_edges[orig_face[i]].edge] < 1)
						edge_label[sym_edges[orig_face[i]].edge] = 1; // candidate for not delaunay. 
			}
			// Set point as inserted
			point_inserted[point_index] = 1;

		}
		index += num_threads;
	}

}
