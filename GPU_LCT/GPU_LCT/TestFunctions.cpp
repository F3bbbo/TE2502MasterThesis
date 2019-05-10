#include "TestFunctions.hpp"

void generate_third_test_input(std::string filename_end, std::vector<std::pair<glm::ivec2, float>> total_obstacle_amount)
{
	std::string filename = "Output files/third_test_input-" + filename_end;

	std::ofstream output (filename.c_str(), std::ofstream::out | std::ofstream::binary);
	if (output.is_open())
	{
		int num = total_obstacle_amount.size();
		output.write((char*)&num, sizeof(int));
		for (int i = 0; i < total_obstacle_amount.size(); i++)
		{
			TestMap test_map;
			test_map.set_num_obsticles(total_obstacle_amount[i].first);
			test_map.set_static_quota(total_obstacle_amount[i].second);
			test_map.set_dynamic_quota(1.f); // We want all of the dynamic objects
			test_map.set_map_size({ 45, 45 }, { -45, -45 });

			GPU::GCMesh gc_mesh;
			gc_mesh.initiate_buffers({ 45.f, 45.f });
			auto static_obstacle_data = test_map.get_GPU_static_obstacles();
			gc_mesh.build_CDT(static_obstacle_data.first, static_obstacle_data.second);
			gc_mesh.refine_LCT();

			auto dynamic_obstacle_data = test_map.get_GPU_dynamic_obstacles();
			std::string mesh_filename = gc_mesh.save_to_file(false, static_obstacle_data.first.size());
			
			// save filename of mesh
			num = sizeof(char) * mesh_filename.size();
			output.write((char*)&num, sizeof(int));
			output.write(mesh_filename.c_str(), num);
			
			// save number of static obstacle vertices
			num = gc_mesh.get_num_vertices();
			output.write((char*)&num, sizeof(int));

			// save dynamic vertices 
			num = (int)dynamic_obstacle_data.first.size() * (int)sizeof(glm::vec2);
			output.write((char*)&num, sizeof(int));
			output.write((char*)dynamic_obstacle_data.first.data(), num);
			
			num = sizeof(glm::ivec2) * dynamic_obstacle_data.second.size();
			output.write((char*)&num, sizeof(int));
			output.write((char*)dynamic_obstacle_data.second.data(), num);
		}
		output.close();
	}
}

void test_range(int iterations, glm::ivec2 start_dims, glm::ivec2 dim_increase, glm::ivec2 start_obstacles, glm::ivec2 obstacle_increase, bool build_lct)
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

		GPU::GCMesh gc_mesh;
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

void first_test(glm::ivec2 obstacle_amount, int iterations)
{
	// Record building of empty map with static objects only, save:
	// Filename:[Algorithm]-[CDT-vertice-amount]-[number of added refinement vertices]
	// [1-10],[time taken to build CDT],[time taken to build LCT]

	std::vector<long long> build_times;

	TestMap test_map;
	test_map.set_map_size({ 45, 45 }, { -45, -45 });
	test_map.set_num_obsticles(obstacle_amount);
	test_map.set_static_quota(1.f);
	auto gpu_frame = test_map.get_GPU_frame();

	std::pair<std::vector<glm::vec2>,std::vector<glm::ivec2>> data = test_map.get_GPU_obstacles();

	// test CPUGPU solution
	int num_vertices[2];
	for (int i = 0; i < iterations; i++)
	{
		GPU::GCMesh gc_mesh;
		gc_mesh.initiate_buffers({ 45, 45 });
		
		build_times.push_back(gc_mesh.build_CDT(data.first, data.second));
		num_vertices[0] = gc_mesh.get_num_vertices();
		build_times.push_back(gc_mesh.refine_LCT());
		num_vertices[1] = gc_mesh.get_num_vertices();
		build_times.push_back(0);
		build_times.push_back(0);
		LOG_ND("First Test CPUGPU iteration: " + std::to_string(i + 1));
	}

	std::string filename = "Output files/first_test_CPUGPU-" + std::to_string(num_vertices[0]) + '-' + std::to_string(num_vertices[1]) + ".txt";
	std::ofstream output(filename.c_str(), std::ofstream::out);

	if (output.is_open())
	{
		for (int i = 0; i < iterations; i++)
		{
			std::string output_string = std::to_string(i) + ',' + std::to_string(build_times[i * 2]) + ',' + std::to_string(build_times[i * 2 + 1]) + '\n';
			output << output_string;
		}
	}
	output.close();
	build_times.clear();
	// test GPU solution
	for (int i = 0; i < iterations; i++)
	{
		GPU::GPUMesh gc_mesh;
		gc_mesh.initiate_buffers({ 45, 45 });
		gc_mesh.add_frame_points(gpu_frame.first);

		build_times.push_back(gc_mesh.build_CDT(data.first, data.second));
		num_vertices[0] = gc_mesh.get_num_vertices();
		build_times.push_back(gc_mesh.refine_LCT());
		num_vertices[1] = gc_mesh.get_num_vertices();
		LOG_ND("First Test GPU iteration: " + std::to_string(i + 1));
	}

	filename = "Output files/first_test_GPU-" + std::to_string(num_vertices[0]) + '-' + std::to_string(num_vertices[1]) + ".txt";
	output.open(filename.c_str(), std::ofstream::out);

	if (output.is_open())
	{
		for (int i = 0; i < iterations; i++)
		{
			std::string output_string = std::to_string(i) + ',' + std::to_string(build_times[i * 2]) + ',' + std::to_string(build_times[i * 2 + 1]) + '\n';
			output << output_string;
		}
	}
	output.close();
}

void second_test(glm::ivec2 obstacles, int iterations)
{
	// Record performance of each shader stage
	TestMap test_map;
	test_map.set_map_size({ 45, 45 }, { -45, -45 });
	test_map.set_num_obsticles(obstacles);
	test_map.set_static_quota(1.f);
	test_map.set_dynamic_quota(1.f);

	std::pair<std::vector<glm::vec2>,std::vector<glm::ivec2>> data = test_map.get_GPU_obstacles();
	std::vector<std::vector<long long>> shader_times;

	std::string filename = "Output files/second_test-" + std::to_string(obstacles.x * obstacles.y) + ".txt";
	std::ofstream output(filename.c_str(), std::ofstream::out);

	if (!output.is_open())
	{
		LOG_T(WARNING, "Can not open output file: " + filename + '\n');
		return;
	}

	for (int i = 0; i < iterations; i++)
	{
		GPU::GPUMesh mesh;
		mesh.initiate_buffers({ 45, 45 });

		shader_times.push_back(mesh.measure_shaders(data.first, data.second));
		LOG_ND("Second Test iteration: " + std::to_string(i + 1) + '\n');
	}

	int num_shaders = 17;
	std::vector<long long> total_times;
	total_times.resize(num_shaders, 0);

	for (int i = 0; i < iterations; i++)
	{
		for (int j = 0; j < num_shaders; j++)
			total_times[j] += shader_times[i][j];
	}

	for (int i = 0; i < total_times.size(); i++)
	{
		output << std::to_string(total_times[i] / iterations);
		if (i < num_shaders - 1)
			output << ',';
	}

	output.close();
}

void third_test(std::string input_file, bool test_CPUGPU, bool test_GPU)
{
	// Test: 3
	// Filename:[Algorithm]-[number of static vertices]-[number of dynamic obstacle vertices]
	// [1-10],[time taken to build CDT],[time taken to build LCT]-[number of added refinement vertices]
	std::string input_filename = "Output files/third_test_input-" + input_file;
	std::ifstream input(input_filename.c_str(), std::ios::in | std::ios::binary);
	int maps;
	if (!input.is_open())
	{
		LOG_T(WARNING, "can not open file:" + input_filename);
		return;
	}

	input.read((char*)&maps, sizeof(int));
	int size;

	for (int i = 0; i < maps; i++)
	{
		input.read((char*)&size, sizeof(int));
		char* mesh_name_c = new char[size];
		input.read(mesh_name_c, size);
		std::string mesh_name(mesh_name_c, size);
		
		int num_static_vertices;
		input.read((char*)&num_static_vertices, sizeof(int)); // get number of static object vertices

		input.read((char*)&size, sizeof(int)); // get size of vertive data in bytes
		std::vector<glm::vec2> dynamic_vertices;
		dynamic_vertices.resize(size / sizeof(glm::vec2));
		input.read((char*)dynamic_vertices.data(), size); // get vertice data

		input.read((char*)&size, sizeof(int)); // get size of vertice indices size
		std::vector<glm::ivec2> dynamic_vertex_indices;
		dynamic_vertex_indices.resize(size / sizeof(glm::ivec2));
		input.read((char*)dynamic_vertex_indices.data(), size); // get indices data

		// All input data has been loaded
		// GPU CPU
		std::string output_filename = "Output files/third_test_result_of_" + input_file + "_CPUGPU-" + std::to_string(num_static_vertices) + '-' + std::to_string(dynamic_vertices.size()) + ".txt";
		std::ofstream output (output_filename.c_str(), std::ifstream::out);

		if (!output.is_open())
		{
			LOG_T(WARNING, "can not open file:" + output_filename);
			continue;
		}
		
		std::vector<long long> build_times;

		if (test_CPUGPU)
		{
			GPU::GCMesh gc_mesh;
			gc_mesh.load_from_file(mesh_name);
			GPU::GCMesh gc_mesh_copy = gc_mesh;

			for (int j = 0; j < 10; j++)
			{
				build_times.push_back(gc_mesh.build_CDT(dynamic_vertices, dynamic_vertex_indices));
				build_times.push_back(gc_mesh.refine_LCT());
				output << std::to_string(j) + ',' + std::to_string(build_times[j * 2]) + ',' + std::to_string(build_times[j * 2 + 1]) + ',' + std::to_string((int)gc_mesh.get_num_vertices() - num_static_vertices - (int)dynamic_vertices.size()) + '\n';
				gc_mesh = gc_mesh_copy;
				LOG("Third Test CPUGPU iteration: " + std::to_string(i + 1));
			}
			output.close();
			build_times.clear();
		}
		
		if (test_GPU)
		{
			GPU::GPUMesh g_mesh;
			g_mesh.initiate_buffers({45.f, 45.f});
			g_mesh.load_from_file(mesh_name);
			GPU::GPUMesh g_mesh_copy;
			g_mesh_copy = g_mesh;

			output_filename = "Output files/third_test_result_of_" + input_file + "_GPU-" + std::to_string(num_static_vertices) + '-' + std::to_string(dynamic_vertices.size());
			output.open(output_filename.c_str(), std::ifstream::out);

			if (!output.is_open())
			{
				LOG_T(WARNING, "can not open file:" + output_filename);
				continue;
			}

			for (int j = 0; j < 10; j++)
			{
				build_times.push_back(g_mesh.build_CDT(dynamic_vertices, dynamic_vertex_indices));
				build_times.push_back(g_mesh.refine_LCT());

				output << std::to_string(j) + ',' + std::to_string(build_times[j * 2]) + ',' + std::to_string(build_times[j * 2 + 1]) + ',' + std::to_string((int)g_mesh.get_num_vertices() - num_static_vertices - (int)dynamic_vertices.size()) + '\n';
				g_mesh = g_mesh_copy;
				LOG("Second Test GPU iteration: " + std::to_string(i + 1));
			}
			output.close();
		}
	}
	input.close();
}
