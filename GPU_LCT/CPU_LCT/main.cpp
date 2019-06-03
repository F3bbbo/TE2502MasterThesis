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

#include "../GPU_LCT/Timer.hpp"
#include "../GPU_LCT/TestMap.hpp"
#include "../GPU_LCT/Log.hpp"

void first_test(glm::ivec2 obstacle_amount, glm::ivec2 obstacle_increase, int increase_iterations, int iterations);

int main()
{
	// Activate license:
	const char* licfile = "triplannerlic.txt";
	tp_verify_license_file(licfile); // first check if license file is there
	tp_activate(licfile); // ok, load and activate
	
	//first_test();

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
		test_map.set_map_size({ 45, 45 }, { -45, -45 });
		test_map.set_num_obsticles(obstacle_amount + obstacle_increase * iter);
		test_map.set_static_quota(1.f);
		auto gpu_frame = test_map.get_GPU_frame();

		auto polygons = test_map.get_CPU_static_obstacles();

		std::pair<std::vector<glm::vec2>, std::vector<glm::ivec2>> data = test_map.get_GPU_obstacles();

		vertice_counts[iter] = data.first.size();
		// test Kallmanns solution
		for (int i = 0; i < iterations; i++)
		{
			tpLct* lct = tp_lct_newref(0.001f);
			tp_lct_init(lct, -45, -45, 45, 45, 0);

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
			LOG_ND("First Test iteration: " + std::to_string(i + 1) + '\n');
		}
	}

	build_times;
}