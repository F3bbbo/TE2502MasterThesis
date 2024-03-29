// ------------------------------
// Geometric spaces include files

#include <gspaces/tpapi_mesh.h>
#include <gspaces/tpapi_polygon.h>
#include <gspaces/tpapi_lct_planner.h>

// end
// ------------------------------

#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <fstream>
#include <sstream>

#include "../GPU_LCT/Timer.hpp"
#include "../GPU_LCT/TestMap.hpp"
#include "../GPU_LCT/Log.hpp"
#include "../GPU_LCT/TestFunctions.hpp"

void first_test(glm::ivec2 obstacle_amount, glm::ivec2 obstacle_increase, int increase_iterations, int iterations);
void third_test(glm::ivec2 obstacle_amount, glm::ivec2 obstacle_increase, float static_percentage, int increase_iterations, int iterations);

int main()
{
	// Activate license:
	const char* licfile = "triplannerlic.txt";
	tp_verify_license_file(licfile); // first check if license file is there
	tp_activate(licfile); // ok, load and activate
	float iterations = 100;
	float number_of_increase = 20;
	int start = 10;
	int increase = 10;
	first_test({ start, start }, {increase, increase}, number_of_increase + 9, iterations);
	third_test({ start, start }, {increase, increase}, 0.25, number_of_increase, iterations);
	third_test({ start, start }, {increase, increase}, 0.5, number_of_increase, iterations);
	third_test({ start, start }, {increase, increase}, 0.75, number_of_increase, iterations);

	iterations = 100;
	number_of_increase = 16;
	start = 3;
	increase = 3;

	first_test({ start, start }, {increase, increase}, number_of_increase, iterations);
	third_test({ start, start }, {increase, increase}, 0.25, number_of_increase, iterations);
	third_test({ start, start }, {increase, increase}, 0.5, number_of_increase, iterations);
	third_test({ start, start }, {increase, increase}, 0.75, number_of_increase, iterations);

	LOG_ND("Finished testing");
	getchar();
	return 0;
}

void first_test(glm::ivec2 obstacle_amount, glm::ivec2 obstacle_increase, int increase_iterations, int iterations)
{
	std::vector<std::vector<long long>> build_times;
	build_times.resize(increase_iterations);

	std::vector<int> vertice_counts;
	vertice_counts.resize(increase_iterations);
	for (int iter = 0; iter < increase_iterations; iter++)
	{
		TestMap test_map;
		test_map.set_map_size({ TEST_MAP_SIZE_X, TEST_MAP_SIZE_Y }, { -TEST_MAP_SIZE_X, -TEST_MAP_SIZE_Y });
		test_map.set_num_obsticles(obstacle_amount + obstacle_increase * iter);
		test_map.set_static_quota(1.f);

		auto polygons = test_map.get_CPU_static_obstacles();

		std::pair<std::vector<glm::vec2>, std::vector<glm::ivec2>> data = test_map.get_GPU_obstacles();

		vertice_counts[iter] = data.first.size();
		// test Kallmanns solution
		for (int i = 0; i < iterations + 1; i++)
		{
			if (i == 0)
			{
				// Warmup run, otherwise the first result will be significantly slower than the rest.
				tpLct* lct = tp_lct_newref(0.001f);
				tp_lct_init(lct, -TEST_MAP_SIZE_X, -TEST_MAP_SIZE_Y, TEST_MAP_SIZE_X, TEST_MAP_SIZE_Y, 0);

				// Only build CDT
				tp_lct_mode(lct, TpRefMode::TpRefGlobal, TpRemMode::TpRemFull);

				for (std::vector<glm::vec2>& polygon : polygons)
					tp_lct_insert_polygonfv(lct, (float*)polygon.data(), polygon.size(), TpClosedPolygon);

				tp_lct_refine(lct);
				tp_lct_unref(lct);
			}
			else
			{
				tpLct* lct = tp_lct_newref(0.001f);
				tp_lct_init(lct, -TEST_MAP_SIZE_X, -TEST_MAP_SIZE_Y, TEST_MAP_SIZE_X, TEST_MAP_SIZE_Y, 0);

				// Only build CDT
				tp_lct_mode(lct, TpRefMode::TpRefGlobal, TpRemMode::TpRemFull);
				Timer timer;

				timer.start();
				for (std::vector<glm::vec2>& polygon : polygons)
					tp_lct_insert_polygonfv(lct, (float*)polygon.data(), polygon.size(), TpClosedPolygon);
				timer.stop();
				build_times[iter].push_back(timer.elapsed_time());

				timer.start();
				tp_lct_refine(lct);
				timer.stop();
				build_times[iter].push_back(timer.elapsed_time());

				tp_lct_unref(lct);
			}
		}
		LOG_ND("First Test completed map: " + std::to_string(iter + 1) + '\n');
	}

	int new_obstacle_amount = (obstacle_amount.x + obstacle_increase.x * (increase_iterations - 1)) * (obstacle_amount.y + obstacle_increase.y * (increase_iterations - 1));

	std::string filename = "Output files/first_test_CPU-" + std::to_string(obstacle_amount.x * obstacle_amount.y) + '-' + std::to_string(new_obstacle_amount) + "-v0.txt";
	std::ofstream output(filename.c_str(), std::ofstream::out);

	if (output.is_open())
	{
		output << "CDT build time, LCT build time \n" << std::to_string(iterations) << ',' << std::to_string(increase_iterations) << '\n';
		for (int iter = 0; iter < increase_iterations; iter++)
		{
			output << std::to_string(vertice_counts[iter]) << '\n';
			for (int i = 0; i < iterations; i++)
				output << std::to_string(build_times[iter][i * 2]) + ',' + std::to_string(build_times[iter][i * 2 + 1]) + '\n';
			output << '\n';
		}
	}
	output.close();
}

void third_test(glm::ivec2 obstacle_amount, glm::ivec2 obstacle_increase, float static_percentage, int increase_iterations, int iterations)
{
	std::vector<std::vector<long long>> build_times;
	build_times.resize(increase_iterations);

	std::vector<std::pair<int,int>> vertice_counts;
	vertice_counts.resize(increase_iterations);

	for (int iter = 0; iter < increase_iterations; iter++)
	{
		tpLct* lct = tp_lct_newref(0.001f);
		tp_lct_init(lct, -TEST_MAP_SIZE_X, -TEST_MAP_SIZE_Y, TEST_MAP_SIZE_X, TEST_MAP_SIZE_Y, 0);

		TestMap test_map;
		test_map.set_map_size({ TEST_MAP_SIZE_X, TEST_MAP_SIZE_Y }, { -TEST_MAP_SIZE_X, -TEST_MAP_SIZE_Y });
		test_map.set_num_obsticles(obstacle_amount + obstacle_increase * iter);
		test_map.set_static_quota(static_percentage);
		test_map.set_dynamic_quota(1.f);

		auto static_polygons = test_map.get_CPU_static_obstacles();
		auto dynamic_polygons = test_map.get_CPU_dynamic_obstacles();

		vertice_counts[iter] = { static_polygons.size() * 3, dynamic_polygons.size() * 3 };
		// insert all the static obstacles and create the LCT
		for (auto& polygon : static_polygons)
			tp_lct_insert_polygonfv(lct, (float*)polygon.data(), polygon.size(), TpClosedPolygon);

		tp_lct_mode(lct, TpRefMode::TpRefGlobal, TpRemMode::TpRemFull);

		std::string filename = "Output files/test_3_input";
		tp_lct_save(lct, filename.c_str());

		for (int j = 0; j < iterations + 1; j++)
		{
			if (j == 0)
			{
				// Warmup run, otherwise the first result will be significantly slower than the rest.
				for (std::vector<glm::vec2>& polygon : dynamic_polygons)
					tp_lct_insert_polygonfv(lct, (float*)polygon.data(), polygon.size(), TpClosedPolygon);

				tp_lct_refine(lct);
				tp_lct_load(lct, filename.c_str());
			}
			else
			{
				Timer timer;

				timer.start();
				for (std::vector<glm::vec2>& polygon : dynamic_polygons)
					tp_lct_insert_polygonfv(lct, (float*)polygon.data(), polygon.size(), TpClosedPolygon);
				timer.stop();
				build_times[iter].push_back(timer.elapsed_time());

				timer.start();
				tp_lct_refine(lct);
				timer.stop();
				build_times[iter].push_back(timer.elapsed_time());

				tp_lct_load(lct, filename.c_str());
			}
		}
		tp_lct_unref(lct);
		LOG_ND("Third Test completed map: " + std::to_string(iter + 1) + '\n');
	}
	std::ostringstream static_quota_stream;
	static_quota_stream.precision(2);
	static_quota_stream << std::fixed << static_percentage;
	std::string output_filename = "Output files/third_test_CPU-" + std::to_string(vertice_counts.front().first + (int)vertice_counts.front().second) + '-' + std::to_string(vertice_counts.back().first + (int)vertice_counts.back().second) + "-v0-" + static_quota_stream.str() + ".txt";
	std::ofstream output(output_filename.c_str(), std::ifstream::out);

	if (!output.is_open())
	{
		LOG_T(WARNING, "can not open file:" + output_filename);
		return;
	}

	output << "CDT build time, LCT build time \n" + std::to_string(iterations) + ',' + std::to_string(increase_iterations) + '\n';

	for (int iter = 0; iter < increase_iterations; iter++)
	{
		output << std::to_string(vertice_counts[iter].first) + ',' + std::to_string(vertice_counts[iter].second) + '\n';
		for (int i = 0; i < iterations; i++)
		{
			output << std::to_string(build_times[iter][2 * i    ]) << ',';
			output << std::to_string(build_times[iter][2 * i + 1]) << '\n';
		}
		output << '\n';
	}
	output.close();
}
