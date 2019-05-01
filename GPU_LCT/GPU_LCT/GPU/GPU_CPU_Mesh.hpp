#pragma once
#ifndef GPU_CPU_MESH_HPP
#define GPU_CPU_MESH_HPP

#include "data_structures.hpp"
#include "../trig_functions.hpp"
#include "..//Timer.hpp"
#include <glm/glm.hpp>
#include <utility>
#include <fstream>

using namespace glm;
namespace GPU
{
	class GCMesh
	{
	public:
		GCMesh(glm::ivec2 screen_res);
		~GCMesh();
		void initiate_buffers(glm::vec2 scale = { 1.f, 1.f });
		long long build_CDT(std::vector<glm::vec2> points, std::vector<glm::ivec2> segments);
		long long refine_LCT();
		std::vector<glm::vec2> get_vertices();
		size_t get_num_vertices();
		glm::vec2 get_vertex(int index);
		SymEdge get_symedge(int index);
		int get_label(int index);
		std::vector<std::pair<glm::ivec2, bool>> get_edges();
		std::vector<glm::ivec3> get_faces();
		int locate_face(glm::vec2 p);
		void save_to_file(std::string filename);
		void load_from_file(std::string filename);
	private:
		void setup_compute_shaders();
		void compile_cs(GLuint& program, const char* path, int work_group_size = 64);

		// point buffers
		std::vector<glm::vec2> point_positions;
		std::vector<int> point_inserted;
		std::vector<int> point_tri_index;
		// edge buffers
		std::vector<int> edge_label;
		std::vector<int> edge_is_constrained;
		// segment buffers
		std::vector<glm::ivec2> seg_endpoint_indices;
		std::vector<int> seg_inserted;
		// triangle buffers
		std::vector<glm::ivec4> tri_symedges;
		std::vector<int> tri_ins_point_index;
		std::vector<int> tri_seg_inters_index;
		std::vector<int> tri_edge_flip_index;
		std::vector<NewPoint> tri_insert_points;
		// symedges
		std::vector<SymEdge> sym_edges;
		int symedge_buffer_size;
		int status;

		// Access functions
		int nxt(int edge);
		SymEdge nxt(SymEdge s);
		int rot(int edge);
		SymEdge rot(SymEdge s);
		int sym(int edge);
		SymEdge sym(SymEdge s);
		int prev(int edge);
		SymEdge prev(SymEdge s);
		int crot(int edge);
		void get_face(int face_i, std::array<vec2, 3> &face_v);
		//SymEdge get_symedge(int index);
		SymEdge prev_symedge(SymEdge s);
		SymEdge sym_symedge(SymEdge s);
		int crot_symedge_i(SymEdge s);
		//vec2 get_vertex(int index);
		//int get_label(int index);
		vec2 get_face_center(int face_i);
		int get_index(SymEdge s);
		int get_face_vertex_symedge(int face, int vertex);
		bool face_contains_vertice(int face, int vertex);
		bool face_contains_vertex(int vert, SymEdge s_triangle);
		std::array<vec2, 2> get_segment(int index);
		std::array<vec2, 2>  get_edge(int s_edge);


		// CDT shaders
		//GLuint m_location_program;
		void location_program();
		//GLuint m_location_tri_program;
		void location_tri_program();
		//GLuint m_insertion_program;
		void insertion_program();
		//GLuint m_marking_part_one_program;
		void marking_part_one_program();
		//GLuint m_marking_part_two_program;
		void marking_part_two_program();
		//GLuint m_flip_edges_part_one_program;
		void flip_edges_part_one_program();
		//GLuint m_flip_edges_part_two_program;
		void flip_edges_part_two_program();
		//GLuint m_flip_edges_part_three_program;
		void flip_edges_part_three_program();

		// LCT shaders
		//GLuint m_locate_disturbances_program;
		void locate_disturbances_program();
		//GLuint m_add_new_points_program;
		void add_new_points_program();
		//GLuint m_insert_in_edge_program;
		void insert_in_edge_program();
		//GLuint m_locate_point_triangle_program;
		void locate_point_triangle_program();
		//GLuint m_validate_edges_program;
		void validate_edges_program();

		// shader functions 
		void oriented_walk(int &curr_e, int point_i, bool &on_edge, vec2 &tri_cent);
		// insertion step
		bool is_valid_face(int face_i);
		// insertion_tri step
		bool valid_point_into_face(int face, vec2 p);
		// Marking_step_part_one functions
		int oriented_walk_point(int curr_e, int goal_point_i);
		int points_connected(int e1, int e2);
		bool check_for_sliver_tri(int sym_edge);
		void straight_walk(int segment_index, SymEdge s_starting_point, int ending_point_i);
		void process_triangle(int segment_index, SymEdge triangle);
		bool will_be_flipped(int segment_index, SymEdge triangle);
		bool pre_candidate_check(SymEdge s);
		bool polygonal_is_strictly_convex(int num, vec2 p1, vec2 p2, vec2 p3, vec2 p4, vec2 p5 = vec2(0.f, 0.f));
		bool check_side(vec2 direction, vec2 other);
		bool Qi_check(int segment_index, SymEdge ei1, SymEdge ei);
		bool first_candidate_check(vec2 s1, vec2 s2, SymEdge ei);
		bool second_candidate_check(vec2 p1, vec2 p2, vec2 p3, vec2 p4, vec2 p5);
		bool third_candidate_check(bool edges_connected, vec2 p1, vec2 p2, vec2 p3, vec2 p4, vec2 p5);
		// Marking_step_part_two functions
		bool is_delaunay(int sym);
		bool is_flippable(int e);
		// Flipping_part_two functions
		void set_quad_edges_label(int label, SymEdge edge);
		// Flipping_part_three functions
		void flip_edge(SymEdge edge);
		// LCT functions
		// Locate refinements functions
		int find_closest_constraint(vec2 a, vec2 b, vec2 c);
		bool possible_disturbance(vec2 a, vec2 b, vec2 c, vec2 s[2]);
		int find_segment_symedge(int start, int segment);
		void oriented_walk_edge(int &curr_e, vec2 point, bool &on_edge);
		int find_constraint_disturbance(int constraint, int edge_ac, bool right);
		float is_disturbed(int constraint, int b_sym, int& v_sym);
		bool no_collinear_constraints(int v);
		int find_next_rot(int start, int curr, bool &reverse);
		int find_next_rot_constraint(int start, int curr, bool& reverse);
		bool is_orthogonally_projectable(vec2 v, vec2 a, vec2 b);
		float local_clearance(vec2 b, std::array<vec2, 2> &segment);
		vec2 find_e_point(int &v_sym, vec2 v, vec2 v_prim);
		vec2 calculate_refinement(int c, int v_sym, bool &success);
		// Validate_edges functions
		bool adjacent_tri_point_intersects_edge(SymEdge curr_edge, int &face_index);


		// test functions
		template<typename T>
		std::vector<int> find_equal(std::vector<T> arr, T e)
		{
			std::vector<int> ret;
			for (int i = 0; i < arr.size(); i++)
			{
				if (arr[i] == e)
				{
					ret.push_back(i);
				}
			}
			return ret;
		};
	};
}
#endif
