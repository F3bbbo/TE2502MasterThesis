#pragma once
#include "GPU/GPU_Mesh.hpp"
#include "GPU/GPU_CPU_Mesh.hpp"
#include "TestMap.hpp"
#include "Timer.hpp"

void generate_third_test_input(std::string filename_end, std::vector<glm::vec2> static_obstacles_list, std::vector<glm::vec2> dynamic_obstacle_list);

void test_range(glm::ivec2 start_resolution, int iterations, glm::ivec2 start_dims, glm::ivec2 dim_increase, glm::ivec2 start_obstacles, glm::ivec2 obstacle_increase, bool build_lct);

void first_test(glm::vec2 static_obstacle_amount, int iterations);

void third_test(std::string input_file);
