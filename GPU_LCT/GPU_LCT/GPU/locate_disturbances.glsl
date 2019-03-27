#version 430
#define FLT_MAX 3.402823466e+38
#define EPSILON 0.00005f
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
	return nxt(rot(edge));
}

int prev(int edge)
{
	return nxt(nxt(edge));
}

int crot(int edge)
{
	int sym_i = sym(edge);
	return (sym_i != -1) ? nxt(sym_i) : -1; 
}

//-----------------------------------------------------------
// Locate disturbance functions
//-----------------------------------------------------------
int find_next_rot_constraint(int start, int curr, in out bool reverse)
{
	int edge = curr;
	while(true)
	{
	
		//Move to nxt edge to check if it is a constraint
		if(reverse) // reverse
		{
			edge = crot(edge);
			if(edge == -1) // if there is no more edges 
			{
				return -1;
			}
		}
		else // forward
		{
			edge = rot(edge);
			if(edge == start)
			{
				return -1;
			}
			else if(edge == -1)
			{
				reverse = true;
				edge = start;
				continue;
			}
		}
		//if edge is constrained set it to curr_constraint
		if(edge_is_constrained[sym_edges[edge].edge] != 0)
		{
			return edge;
		}

	}		
}

bool no_collinear_constraints(int v){
	int edge;
	bool reverse = false;
	int curr_constraint = v;
	int last_constraint = -1;
	vec2 point = point_positions[sym_edges[v].vertex];
	// first check if initial value is an constraint
	if(edge_is_constrained[sym_edges[v].edge] != 0)
	{
		curr_constraint = v;
	}
	else{
		curr_constraint = find_next_rot_constraint(v, v, reverse);
	}
	while(curr_constraint != -1)
	{	
		vec2 curr_vec = normalize(point_positions[sym_edges[nxt(curr_constraint)].vertex] - point);
		// explore if current constraint is collinear with another constraint
		bool explore_rd = reverse;
		edge = find_next_rot_constraint(v, curr_constraint, explore_rd);
		while(edge != -1)
		{
			// check if edge is a constraint
			if(edge_is_constrained[sym_edges[edge].edge] != 0)
			{
				//if it is a constraint check if it is collinear with curr_constraint
				vec2 other_vec = normalize(point_positions[sym_edges[nxt(edge)].vertex] - point);
				if(dot(curr_vec, other_vec) < - 1 + EPSILON)
				{
					return false;
				}
			}
			// find next constraint
			edge = find_next_rot_constraint(v, edge, explore_rd);
		}

		// find next constraint to explore
		curr_constraint = find_next_rot_constraint(v, curr_constraint, reverse);
	}
	return true;
}

vec2 project_point_on_line(vec2 point, vec2 a, vec2 b)
{
	vec2 ab = b - a;
	vec2 ap = point - a;
	return a + dot(ap, ab) / dot(ab, ab) * ab;
}

bool is_orthogonally_projectable(vec2 v, vec2 a, vec2 b)
{
	vec2 line = b - a;
	vec2 projected_point = project_point_on_line(v, a, b);

	float projected_length = dot(normalize(line), projected_point - a);

	if (projected_length < 0.f || projected_length * projected_length > dot(line, line))
		return false;

	return true;
}

bool orientation(in vec2 p1 , in vec2 p2 , in vec2 p3)
{
	float val = (p2.y - p1.y) * (p3.x - p2.x) - (p2.x - p1.x) * (p3.y - p2.y);
	return (val > 0.0f) ? true : false;
}

bool line_line_test(in vec2 s1p1 , in vec2 s1p2, in vec2 s2p1, in vec2 s2p2)
{
	bool o1 = orientation(s1p1, s1p2, s2p1);
	bool o2 = orientation(s1p1, s1p2, s2p2);
	bool o3 = orientation(s2p1, s2p2, s1p1);
	bool o4 = orientation(s2p1, s2p2, s1p2);

	return (o1 != o2 && o3 != o4) ? true : false;
}


float is_disturbed(int constraint, int b_sym, int v_sym, vec2 e)
{
	// 1
	if(no_collinear_constraints(v_sym))
		return -1.0f;

	// 2
	vec2 v = point_positions[sym_edges[v_sym].vertex];
	vec2 a = point_positions[sym_edges[prev(b_sym)].vertex];
	vec2 b = point_positions[sym_edges[b_sym].vertex];
	vec2 c = point_positions[sym_edges[nxt(b_sym)].vertex];

	if(!is_orthogonally_projectable(v, a, c))
		return -1.0f;

	// 3

	
	return 1.0f;
}





int find_constraint_disturbance(int constraint, int edge_ac, bool right)
{
	vec2 R[3];
	vec2 a;
	// Set variables needed to calculate R
	if(right)
	{
		R[0] = point_positions[sym_edges[edge_ac].vertex];
		R[1] = point_positions[sym_edges[prev(edge_ac)].vertex];
		a = point_positions[sym_edges[nxt(edge_ac)]];
	}
	else{
		R[0] = point_positions[sym_edges[nxt(edge_ac)].vertex];
		R[1] = point_positions[sym_edges[prev(edge_ac)].vertex];
		a = point_positions[sym_edges[edge_ac]];
	}
	// Calculate R
	vec2 ac = R[0] - a;
	vec2 dir = normalize(ac);
	vec2 ab = R[1] - a;
	float b_prim = dot(dir, ab);
	R[2] = R[1] + (dir * (length(ac) - b_prim));

	// Loop through points trying to find disturbance to current traversal
	float best_dist = FLT_MAX;
	int first_disturb = -1;
	for(int i = 0; i < point_positions.length(); i++)
	{
		
	}

	return first_disturb;
}



// Each thread represents one triangle
void main(void)
{
	uint gid = gl_GlobalInvocationID.x;
	int index = int(gid);
	int num_threads = int(gl_NumWorkGroups.x * gl_WorkGroupSize.x);
	while(index < tri_seg_inters_index.length())
	{
		int num_constraints;
		int c_edge_i = -1;
		// loop checking for segments in triangle
		for(int i = 0; i < 3; i++)
		{
			// Checking if edge of triangle is constrained.
			if(edge_is_constrained[sym_edges[tri_symedges[index][i]]] > 0)
			{
				num_constraints++;
				c_edge_i = tri_symedges[index][i];
			}
		}

		if(num_constraints == 0)
		{
			// TODO: Look for close constraints
		}
		else if(num_constraints == 1)
		{
			//
		}
		
		index += num_threads;
	}
}