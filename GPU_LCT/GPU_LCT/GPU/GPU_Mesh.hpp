#pragma once
#ifndef GPU_MESH_HPP
#define GPU_MESH_HPP

#include "data_structures.hpp"
#include "../trig_functions.hpp"
#include "..//Timer.hpp"
#include <glm/glm.hpp>
#include <utility>

namespace GPU
{
	class GPUMesh
	{
	public:
		GPUMesh();
		virtual ~GPUMesh();
		GPUMesh& operator=(GPUMesh& other);
		GPUMesh(const GPUMesh &other) = delete; 
		void initiate_buffers(glm::vec2 scale = { 1.f, 1.f });
		void add_frame_points(std::vector<glm::vec2> points);
		double build_CDT(std::vector<glm::vec2> points, std::vector<glm::ivec2> segments);
		double refine_LCT();
		std::vector<double> measure_shaders(std::vector<glm::vec2> points, std::vector<glm::ivec2> segments);
		std::vector<glm::vec2> get_vertices();
		int get_num_vertices();
		glm::vec2 get_vertex(int index);
		SymEdge get_symedge(int index);
		std::vector<std::pair<glm::ivec2, bool>> get_edges();
		std::vector<glm::ivec3> get_faces();
		int locate_face(glm::vec2 p);
		void set_epsilon(float epsi);
		void set_version(int vers);
		std::string save_to_file(bool upload, int inserted_objects = -1);
		void load_from_file(std::string filename);
		Find_Disturbance_Status get_find_dist_status();
		// CDT shaders
		static GLuint m_location_program;
		static GLuint m_location_tri_program;
		static GLuint m_insertion_program;
		static GLuint m_marking_part_one_program;
		static GLuint m_marking_part_two_program;
		static GLuint m_flip_edges_part_one_program;
		static GLuint m_flip_edges_part_two_program;
		static GLuint m_flip_edges_part_three_program;

		// LCT shaders
		static GLuint m_locate_disturbances_program;
		static GLuint m_add_new_points_program;
		static GLuint m_locate_point_triangle_program;
		static GLuint m_validate_edges_program;
		static GLuint m_insert_in_edge_program;

	private:
		bool m_buffers_initated = false;
		glm::ivec2 m_scale;
		void setup_compute_shaders();
		void compile_cs(GLuint& program, const char* path, int work_group_size = 64);
		void remove_duplicate_points(std::vector<vec2> &list);
		PointBuffers m_point_bufs;
		EdgeBuffers m_edge_bufs;
		SegmentBuffers m_segment_bufs;
		TriangleBuffers m_triangle_bufs;
		Buffer m_sym_edges;
		Buffer m_new_points;
		Buffer m_refine_points;
		Buffer m_nr_of_symedges;
		Buffer m_status;
		Buffer m_find_dist_status;
		Buffer m_epsilon_buff;
		Buffer m_version_buff;
		Buffer m_semaphores;
		float m_epsilon = 0.0001f;
		int version = 2;



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
