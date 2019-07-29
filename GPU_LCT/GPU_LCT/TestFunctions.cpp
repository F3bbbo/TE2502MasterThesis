#include "TestFunctions.hpp"

void generate_third_test_input(std::string filename_end, std::vector<std::pair<glm::ivec2, float>> total_obstacle_amount)
{
	std::string filename = "Output files/third_test_input-" + filename_end;

	std::ofstream output (filename.c_str(), std::ofstream::out | std::ofstream::binary);
	std::vector<std::string> saved_files(total_obstacle_amount.size(), "");

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

			GPU::GPUMesh g_mesh;
			g_mesh.initiate_buffers({ 45.f, 45.f });
			auto static_obstacle_data = test_map.get_GPU_static_obstacles();
			g_mesh.build_CDT(static_obstacle_data.first, static_obstacle_data.second);
			g_mesh.refine_LCT();

			bool failed = false;
			auto status = g_mesh.get_find_dist_status();
			if (status.const_list_status == 1)
			{
				LOG("const_list overflow");
				failed = true;
			}
			if (status.const_queue_status == 1)
			{
				LOG("const_queue overflow");
				failed = true;
			}
			if (status.dist_list_status == 1)
			{
				LOG("dist_list overflow");
				failed = true;
			}
			if (status.dist_queue_status == 1)
			{
				LOG("dist_queue overflow");
				failed = true;
			}

			if (failed)
			{
				LOG("Error during creation of input with index: " + std::to_string(i));
				output.close();
				remove(filename.c_str());

				for (int j = 0; j < i; j++)
					remove(saved_files[j].c_str());
				return;
			}

			auto dynamic_obstacle_data = test_map.get_GPU_dynamic_obstacles();
			std::string mesh_filename = g_mesh.save_to_file(false, static_obstacle_data.first.size());
			saved_files[i] = mesh_filename;

			// save filename of mesh
			num = sizeof(char) * mesh_filename.size();
			output.write((char*)&num, sizeof(int));
			output.write(mesh_filename.c_str(), num);
			
			// save number of static obstacle vertices
			num = static_obstacle_data.first.size();
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

void first_test(glm::ivec2 obstacle_amount, glm::ivec2 obstacle_increase, int increase_iterations, int iterations, bool test_CPUGPU, bool test_GPU, int version)
{
	// Record building of empty map with static objects only, save:
	// Filename:[Version]-[lowest obstacle count]-[highest obstacle count]
	// File content: 
	// [num iterations, num obstacle increase iterations]

	// [vertices]
	// Results

	std::vector<std::string> build_times(increase_iterations, "");
	std::vector<std::pair<bool, std::string>> iteration_failed(increase_iterations, {false, ""});
	int failed_count = 0;
	std::vector<int> vertice_counts(increase_iterations, 0);
	if (test_CPUGPU)
	{
		for (int iter = 0; iter < increase_iterations; iter++)
		{
			TestMap test_map;
			test_map.set_map_size({ 45, 45 }, { -45, -45 });
			test_map.set_num_obsticles(obstacle_amount + obstacle_increase * iter);
			test_map.set_static_quota(1.f);
			auto map_size = (obstacle_amount + obstacle_increase * iter);
			auto gpu_frame = test_map.get_GPU_frame();

			std::pair<std::vector<glm::vec2>, std::vector<glm::ivec2>> data = test_map.get_GPU_obstacles();

			vertice_counts[iter] = data.first.size();
			// test CPUGPU solution
			for (int i = 0; i < iterations; i++)
			{
				GPU::GCMesh gc_mesh;
				gc_mesh.set_version(version);
				gc_mesh.initiate_buffers({ 45, 45 });

				auto cdt_time = gc_mesh.build_CDT(data.first, data.second);
				auto lct_time = gc_mesh.refine_LCT();

				build_times[iter] += std::to_string(cdt_time) + ',' + std::to_string(lct_time) + '\n';
				auto status = gc_mesh.get_find_dist_status();
				if (lct_failed(status))
				{
					iteration_failed[iter] = {true, get_lct_status_string(status)};
					failed_count++;
					LOG_T(WARNING, "first test, GPUCPU, LCT refinement of map index: " + std::to_string(iter) + " failed. Skipping map...");
					break;
				}
				LOG_ND("First Test CPUGPU " + std::to_string( map_size.x * map_size.y ) + " iteration: " + std::to_string(i + 1) + '\n');
			}
		}

		int new_obstacle_amount = (obstacle_amount.x + obstacle_increase.x * (increase_iterations - 1)) * (obstacle_amount.y + obstacle_increase.y * (increase_iterations - 1));
		std::string filename = "Output files/first_test_CPUGPU-" + std::to_string(obstacle_amount.x * obstacle_amount.y) + '-' + std::to_string(new_obstacle_amount) +  "-v" + std::to_string(version) + ".txt";
		std::ofstream output(filename.c_str(), std::ofstream::out);
		std::string error_filename = "Output files/Error_file_first_test-CPUGPU-" + std::to_string(obstacle_amount.x * obstacle_amount.y) + '-' + std::to_string(new_obstacle_amount) + ".txt";
		std::ofstream error_file(error_filename);
		if (output.is_open())
		{
			output << "CDT build time, LCT build time \n" << std::to_string(iterations) << ',' << std::to_string(increase_iterations - failed_count) << '\n';
			for (int iter = 0; iter < increase_iterations; iter++)
			{
				if (iteration_failed[iter].first == true)
				{
					error_file << std::to_string((obstacle_amount.x + obstacle_increase.x * iter) * (obstacle_amount.y + obstacle_increase.y * iter)) << " error: " << iteration_failed[iter].second << '\n';
					continue;
				}
				output << std::to_string(vertice_counts[iter]) << '\n' << build_times[iter] << '\n';
			}
		}
		output.close();
		error_file.close();

		if (0 == failed_count)
			remove(error_filename.c_str());
		if (increase_iterations == failed_count)
			remove(filename.c_str());

		build_times.clear();
		build_times.resize(increase_iterations);
	}
	
	int new_obstacle_amount = (obstacle_amount.x + obstacle_increase.x * (increase_iterations - 1)) * (obstacle_amount.y + obstacle_increase.y * (increase_iterations - 1));
	std::string filename = "Output files/first_test_GPU-" + std::to_string(obstacle_amount.x * obstacle_amount.y) + '-' + std::to_string(new_obstacle_amount) +  "-v" + std::to_string(version) + ".txt";
	std::ofstream output(filename.c_str(), std::ofstream::out);
	std::string error_filename = "Output files/Error_file_first_test-GPU-" + std::to_string(obstacle_amount.x * obstacle_amount.y) + '-' + std::to_string(new_obstacle_amount) + ".txt";
	std::ofstream error_file(error_filename);

	if (test_GPU && output.is_open())
	{
		output << "CDT build time, LCT build time \n" << std::to_string(iterations) << ',' << std::to_string(increase_iterations) << '\n';

		std::fill(vertice_counts.begin(), vertice_counts.end(), 0);
		std::fill(iteration_failed.begin(), iteration_failed.end(), std::make_pair(false, ""));
		failed_count = 0;
		for (int iter = 0; iter < increase_iterations; iter++)
		{
			TestMap test_map;
			test_map.set_map_size({ 45, 45 }, { -45, -45 });
			test_map.set_num_obsticles(obstacle_amount + obstacle_increase * iter);
			test_map.set_static_quota(1.f);
			auto map_size = (obstacle_amount + obstacle_increase * iter);
			auto gpu_frame = test_map.get_GPU_frame();

			std::pair<std::vector<glm::vec2>, std::vector<glm::ivec2>> data = test_map.get_GPU_obstacles();

			vertice_counts[iter] = data.first.size();
			// test GPU solution
			for (int i = 0; i < iterations + 1; i++)
			{
				if (i == 0)
				{
					// The GPU needs to be "warmed up" or else the first result will be very slow
					GPU::GPUMesh gc_mesh;
					gc_mesh.initiate_buffers({ 45, 45 });
					gc_mesh.set_version(version);
					gc_mesh.add_frame_points(gpu_frame.first);

					gc_mesh.build_CDT(data.first, data.second);
					gc_mesh.refine_LCT();
					auto status = gc_mesh.get_find_dist_status();
					if (lct_failed(status))
					{
						iteration_failed[iter] = {true, get_lct_status_string(status)};
						failed_count++;
						LOG_T(WARNING, "first test, GPU, LCT refinement of map index: " + std::to_string(iter) + " failed. Skipping map...");
						break;
					}
				}
				else
				{
					GPU::GPUMesh gc_mesh;
					gc_mesh.initiate_buffers({ 45, 45 });
					gc_mesh.set_version(version);
					gc_mesh.add_frame_points(gpu_frame.first);

					build_times[iter] += std::to_string(gc_mesh.build_CDT(data.first, data.second)) + ',' + std::to_string(gc_mesh.refine_LCT()) + '\n';

					auto status = gc_mesh.get_find_dist_status();
					if (lct_failed(status))
					{
						iteration_failed[iter] = {true, get_lct_status_string(status)};
						failed_count++;
						LOG_T(WARNING, "first test, GPU, LCT refinement of map index: " + std::to_string(iter) + " failed. Skipping map...");
						break;
					}
					LOG_ND("First Test GPU " + std::to_string( map_size.x * map_size.y ) + " iteration: " + std::to_string(i) + '\n');
				}
			}
			if (iteration_failed[iter].first == true)
			{
				error_file << std::to_string((obstacle_amount.x + obstacle_increase.x * iter) * (obstacle_amount.y + obstacle_increase.y * iter)) << " error: " << iteration_failed[iter].second << '\n';
				error_file.flush();
			}
			output << std::to_string(vertice_counts[iter]) << '\n' << build_times[iter] << '\n';
			output.flush();
		}

		output.close();
		error_file.close();

		if (0 == failed_count)
			remove(error_filename.c_str());
		if (increase_iterations == failed_count)
			remove(filename.c_str());
	}
}

void second_test(glm::ivec2 obstacle_amount, int iterations, int version)
{
	// Record building of empty map, save:
	// Filename:[obstacle count]
	// File content: 
	// [num iterations]

	// [vertices]
	// Results
	int num_shaders = 16;
	std::vector<std::vector<long long>> total_times;
	total_times.resize(iterations);
	for (auto& iteration : total_times)
		iteration.resize(num_shaders);

	
	// Record performance of each shader stage
	TestMap test_map;
	test_map.set_map_size({ 45, 45 }, { -45, -45 });
	test_map.set_num_obsticles(obstacle_amount);
	test_map.set_static_quota(1.f);
	test_map.set_dynamic_quota(1.f);

	std::pair<std::vector<glm::vec2>, std::vector<glm::ivec2>> data = test_map.get_GPU_obstacles();

	bool failed = false;
	std::string fail_string = "";

	for (int i = 0; i < iterations + 1; i++)
	{
		if (i == 0)
		{
			// The GPU needs to be "warmed up" or else the first result will be very slow
			GPU::GPUMesh mesh;
			mesh.initiate_buffers({ 45, 45 });
			mesh.set_version(version);
			mesh.measure_shaders(data.first, data.second);

			auto status = mesh.get_find_dist_status();
			if (lct_failed(status))
			{
				failed = true;
				fail_string = get_lct_status_string(status);
				break;
			}
		}
		else
		{
			GPU::GPUMesh mesh;
			mesh.initiate_buffers({ 45, 45 });
			mesh.set_version(version);
			total_times[i - 1] = mesh.measure_shaders(data.first, data.second);

			auto status = mesh.get_find_dist_status();
			if (lct_failed(status))
			{
				failed = true;
				fail_string = get_lct_status_string(status);
				break;
			}
			LOG_ND("Second Test iteration: " + std::to_string(i) + '\n');
		}
	}

	if (failed == true)
	{
		std::ofstream error_file("Output files/Error_file_second_test-" + std::to_string(obstacle_amount.x) + '-' + std::to_string(obstacle_amount.y) + ".txt");
		error_file << fail_string;
		error_file.close();
		LOG("Test 2 failed with the sizes: " + std::to_string(obstacle_amount.x) + '-' + std::to_string(obstacle_amount.y));
		return;
	}

	unsigned int total_obstacles = obstacle_amount.x * obstacle_amount.y;
	std::string filename = "Output files/second_test-" + std::to_string(obstacle_amount.x * obstacle_amount.y) + "-v" + std::to_string(version) + ".txt";
	std::ofstream output(filename.c_str(), std::ofstream::out);
	if (!output.is_open())
	{
		LOG_T(WARNING, "Can not open output file: " + filename + '\n');
		return;
	}

	output << "time taken for each shader in ms to complete \n" << std::to_string(iterations) << '\n';
	
	output << std::to_string(data.first.size()) << '\n';
	for (auto& iteration : total_times)
	{
		for (int i = 0; i < num_shaders; i++)
		{
			output << std::to_string(iteration[i]);
			if (i < num_shaders - 1)
				output << ',';
			else
				output << '\n';
		}
	}
	output << '\n';

	output.close();
}

void third_test(std::string input_file, int iterations, bool test_CPUGPU, bool test_GPU, int version)
{
	// Test: 3
	// Record building from prebuilt map with dynamic objects, save:
	// Filename:[Version]-[lowest vertex count]-[highest vertex count]
	// File content: 
	// [num iterations, num obstacle increase iterations]

	// [static vertices, dynamic vertices]
	// Results

	struct InputMapData
	{
		std::string filename;
		int static_vertices;
		std::vector<glm::vec2> dynamic_vertices;
		std::vector<glm::ivec2> dynamic_vertice_indices;
	};
	// Read input data
	std::vector<InputMapData> input_data_maps;
	std::string input_filename = "Output files/third_test_input-" + input_file;
	std::ifstream input(input_filename.c_str(), std::ios::in | std::ios::binary);
	int maps;
	if (!input.is_open())
	{
		LOG_T(WARNING, "can not open file:" + input_filename);
		return;
	}
	input.read((char*)&maps, sizeof(int));
	input_data_maps.resize(maps);
	int size;
	for (int i = 0; i < maps; i++)
	{
		input.read((char*)&size, sizeof(int));
		input_data_maps[i].filename.resize(size);
		input.read((char*)input_data_maps[i].filename.c_str(), size);

		input.read((char*)&input_data_maps[i].static_vertices, sizeof(int)); // get number of static object vertices

		input.read((char*)&size, sizeof(int)); // get size of vertice data in bytes
		input_data_maps[i].dynamic_vertices.resize(size / sizeof(glm::vec2));
		input.read((char*)input_data_maps[i].dynamic_vertices.data(), size); // get vertice data

		input.read((char*)&size, sizeof(int)); // get size of vertice indices size
		input_data_maps[i].dynamic_vertice_indices.resize(size / sizeof(glm::ivec2));
		input.read((char*)input_data_maps[i].dynamic_vertice_indices.data(), size); // get indices data
	}
	input.close();
	// done reading input data
	
	std::vector<std::string> map_output(maps, "");
	std::vector<std::pair<bool, std::string>> map_failed(maps, { false, {} });
	int failed_count = 0;

	if (test_CPUGPU)
	{
		std::string output_filename = "Output files/third_test_CPUGPU-" + std::to_string(input_data_maps.front().static_vertices + (int)input_data_maps.front().dynamic_vertices.size()) + '-' + std::to_string(input_data_maps.back().static_vertices + (int)input_data_maps.back().dynamic_vertices.size()) +  "-v" + std::to_string(version) + ".txt";
		std::ofstream output(output_filename.c_str(), std::ifstream::out);

		if (!output.is_open())
		{
			LOG_T(WARNING, "can not open file:" + output_filename);
			return;
		}
		
		for (int map_i = 0; map_i < maps; map_i++)
		{
			std::string output_string = "";
			output_string += std::to_string(input_data_maps[map_i].static_vertices) + ',' + std::to_string(input_data_maps[map_i].dynamic_vertices.size()) + '\n';

			GPU::GCMesh gc_mesh;
			gc_mesh.load_from_file(input_data_maps[map_i].filename);
			gc_mesh.set_version(version);
			GPU::GCMesh gc_mesh_copy = gc_mesh;
			gc_mesh_copy.set_version(version);

			for (int j = 0; j < iterations; j++)
			{
				output_string += std::to_string(gc_mesh.build_CDT(input_data_maps[map_i].dynamic_vertices, input_data_maps[map_i].dynamic_vertice_indices)) + ',';
				output_string += std::to_string(gc_mesh.refine_LCT()) + '\n';

				auto status = gc_mesh.get_find_dist_status();
				if (status.const_list_status == 1 || status.const_queue_status == 1 || status.dist_list_status == 1 || status.dist_queue_status == 1)
				{
					map_failed[map_i] = { true, get_lct_status_string(status) };
					failed_count++;
					break;
				}
				gc_mesh = gc_mesh_copy;
				gc_mesh.set_version(version);
			}
			map_output[map_i] = output_string + '\n';
		}

		std::string error_filename = "Output files/Error_file_third_test-CPUGPU-" + std::to_string(input_data_maps.front().static_vertices + (int)input_data_maps.front().dynamic_vertices.size()) + '-' + std::to_string(input_data_maps.back().static_vertices + (int)input_data_maps.back().dynamic_vertices.size()) + ".txt";
		std::ofstream error_file(error_filename);
		output << "CDT build time, LCT build time \n" + std::to_string(iterations) + ',' + std::to_string(maps - failed_count) + '\n';
		for (int i = 0; i < maps; i++)
		{
			if (map_failed[i].first == true)
			{
				error_file << std::to_string(input_data_maps[i].static_vertices) << ',' << std::to_string(input_data_maps[i].dynamic_vertices.size()) << " error: " << map_failed[i].second << '\n';
			}
			else
			{
				output << map_output[i];
			}
		}
		output.close();
		error_file.close();

		if (0 == failed_count)
			remove(error_filename.c_str());
		if (maps == failed_count)
			remove(output_filename.c_str());
	}

	if (test_GPU)
	{
		std::string output_filename = "Output files/third_test_GPU-" + std::to_string(input_data_maps.front().static_vertices + (int)input_data_maps.front().dynamic_vertices.size()) + '-' + std::to_string(input_data_maps.back().static_vertices + (int)input_data_maps.back().dynamic_vertices.size()) +  "-v" + std::to_string(version) + ".txt";
		std::ofstream output(output_filename.c_str(), std::ifstream::out);
		std::string error_filename = "Output files/Error_file_third_test-GPU-" + std::to_string(input_data_maps.front().static_vertices + (int)input_data_maps.front().dynamic_vertices.size()) + '-' + std::to_string(input_data_maps.back().static_vertices + (int)input_data_maps.back().dynamic_vertices.size()) + ".txt";
		std::ofstream error_file(error_filename);
		output << "CDT build time, LCT build time \n" + std::to_string(iterations) + ',' + std::to_string(maps) + '\n';

		if (!output.is_open() || !error_file.is_open())
		{
			LOG_T(WARNING, "can not open output file or error file");
			return;
		}
		
		std::fill(map_failed.begin(), map_failed.end(), std::make_pair(false, ""));
		failed_count = 0;

		for (int map_i = 0; map_i < maps; map_i++)
		{
			std::string output_string = "";
			output_string += std::to_string(input_data_maps[map_i].static_vertices) + ',' + std::to_string(input_data_maps[map_i].dynamic_vertices.size()) + '\n';
			GPU::GPUMesh g_mesh;
			g_mesh.initiate_buffers({45.f, 45.f});
			g_mesh.set_version(version);
			g_mesh.load_from_file(input_data_maps[map_i].filename);
			GPU::GPUMesh g_mesh_copy;
			g_mesh_copy = g_mesh;
			g_mesh_copy.set_version(version);

			for (int j = 0; j < iterations + 1; j++)
			{
				if (j == 0)
				{
					// The GPU needs to be "warmed up" or else the first result will be very slow
					g_mesh.build_CDT(input_data_maps[map_i].dynamic_vertices, input_data_maps[map_i].dynamic_vertice_indices);
					g_mesh.refine_LCT();
					g_mesh = g_mesh_copy;
					g_mesh.set_version(version);

					auto status = g_mesh.get_find_dist_status();
					if (status.const_list_status == 1 || status.const_queue_status == 1 || status.dist_list_status == 1 || status.dist_queue_status == 1)
					{
						map_failed[map_i] = { true, get_lct_status_string(status) };
						failed_count++;
						break;
					}
				}
				else
				{
					output_string += std::to_string(g_mesh.build_CDT(input_data_maps[map_i].dynamic_vertices, input_data_maps[map_i].dynamic_vertice_indices)) + ',';
					output_string += std::to_string(g_mesh.refine_LCT()) + '\n';
					g_mesh = g_mesh_copy;
					g_mesh.set_version(version);

					auto status = g_mesh.get_find_dist_status();
					if (status.const_list_status == 1 || status.const_queue_status == 1 || status.dist_list_status == 1 || status.dist_queue_status == 1)
					{
						map_failed[map_i] = { true, get_lct_status_string(status) };
						failed_count++;
						break;
					}
				}
			}

			if (map_failed[map_i].first == true)
			{
				error_file << std::to_string(input_data_maps[map_i].static_vertices) << ',' << std::to_string(input_data_maps[map_i].dynamic_vertices.size()) << " error: " << map_failed[map_i].second << '\n';
				error_file.flush();
			}
			else
			{
				output << output_string + '\n';
				output.flush();
			}
		}
		output.close();
		error_file.close();

		if (0 == failed_count)
			remove(error_filename.c_str());
		if (maps == failed_count)
			remove(output_filename.c_str());
	}
}

std::string get_lct_status_string(GPU::Find_Disturbance_Status & status)
{
	std::string ret = "";
	if (status.const_list_status == 1)
		ret = "const_list ";
	if (status.const_queue_status == 1)
		ret += "const_queue ";
	if (status.dist_list_status == 1)
		ret += "dist_list ";
	if (status.dist_queue_status == 1)
		ret +="dist_queue ";

	return ret + "overflow";
}

bool lct_failed(GPU::Find_Disturbance_Status& status)
{
	if (status.const_list_status == 1 || status.const_queue_status == 1 || status.dist_list_status == 1 || status.dist_queue_status == 1)
		return true;
	return false;
}
