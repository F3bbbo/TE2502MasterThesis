#include "TestFunctions.hpp"

// Test: 1
// Record building of empty map with static objects only, save:
// Filename:[Algorithm]-[obstacle_amount]
// [1-10],[number of total vertices],[number of static obstacles],[time taken to build CDT],[time taken to build LCT]

// Test: 2
// Record performance of each shader stage

// Test: 3
// Filename:[Algorithm]-[static_obstacle_amount]
// [1-10],[number of starting vertices],[number of added vertices],[number of static obstacles],[number of dynamic obstacles],[time taken to build CDT],[time taken to build LCT]

void test_range(glm::ivec2 start_resolution, int iterations, glm::ivec2 start_dims, glm::ivec2 dim_increase, glm::ivec2 start_obstacles, glm::ivec2 obstacle_increase, bool build_lct)
{
	std::vector<long long> build_times;
	std::vector<int> num_vertices;
	glm::vec2 dimensions = start_dims;
	glm::vec2 obstacles = start_obstacles;

	for (int i = 0; i < iterations; i++)
	{
		TestMap test_map;
		test_map.set_map_size(dimensions, -dimensions);
		test_map.set_num_obsticles(obstacles);

		GPU::GCMesh gc_mesh(start_resolution);
		gc_mesh.initiate_buffers(dimensions);
		auto data = test_map.get_GPU_obstacles();
		
		build_times.push_back(gc_mesh.build_CDT(data.first, data.second));
		num_vertices.push_back((int)gc_mesh.get_num_vertices());
		num_vertices.push_back((int)data.first.size());
		if (build_lct)
		{
			build_times.push_back(gc_mesh.refine_LCT());
			num_vertices.push_back(num_vertices[num_vertices.size() - 2] - (int)gc_mesh.get_num_vertices());
		}

		dimensions += dim_increase;
		obstacles += obstacle_increase;
	}

	LOG_ND("CDT and LCT Build time\n");
	LOG_ND("-------------------------------\n");

	size_t num_iterations = build_lct ? build_times.size() / 2 : build_times.size();
	for (size_t t = 0; t < num_iterations; t++)
	{
		int vertice_offset = build_lct ? t * 3 : t * 2;
		glm::ivec2 new_dim = start_dims + dim_increase * glm::ivec2(t);
		glm::ivec2 new_obstacles_dim = start_obstacles + obstacle_increase * glm::ivec2(t);
		std::string output_string = "map size: (" + std::to_string(new_dim.x) + ", " + std::to_string(new_dim.y) + ") | obstacles: (" + std::to_string(new_obstacles_dim.x) + ", " + std::to_string(new_obstacles_dim.y)
			+ ") | CDT construction time: " + std::to_string(build_times[t * (build_lct ? 2 : 1)]) + " | total vertices: " + std::to_string(num_vertices[vertice_offset]) + " | constraint vertices: " + std::to_string(num_vertices[vertice_offset + 1]);
		if (build_lct)
		{
			output_string += " | LCT construction time: " + std::to_string(build_times[t * (build_lct ? 2 : 1) + 1]) + " | Added refinement points: " + std::to_string(num_vertices[vertice_offset + 2]);
		}
		output_string += "\n";
		LOG_ND(output_string);

		// TODO: write output_string to file and add those files to .gitignore, add a flag to be able to push results to git anyways
	}
}
