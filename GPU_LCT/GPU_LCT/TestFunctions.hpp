#pragma once
#include "GPU/GPU_Mesh.hpp"
#include "GPU/GPU_CPU_Mesh.hpp"
#include "TestMap.hpp"
#include "Timer.hpp"
#include "TestParameters.hpp"


void generate_third_test_input(std::string filename_end, std::vector<std::pair<glm::ivec2, float>> total_obstacle_amount);

void test_range(int iterations, glm::ivec2 start_dims, glm::ivec2 dim_increase, glm::ivec2 start_obstacles, glm::ivec2 obstacle_increase, bool build_lct);

void first_test(glm::ivec2 obstacle_amount, glm::ivec2 obstacle_increase, int increase_iterations, int iterations, bool test_CPUGPU, bool test_GPU, int version = 2);

void second_test(glm::ivec2 obstacle_amount, int iterations, int version = 2);

void third_test(std::string input_file, int iterations, bool test_CPUGPU, bool test_GPU, float static_quota, int version = 2);

std::string get_lct_status_string(GPU::Find_Disturbance_Status& status);

bool lct_failed(GPU::Find_Disturbance_Status& status);

std::vector<std::pair<glm::ivec2, float>> gen_obstacle_range(glm::ivec2 start_dims, glm::ivec2 dim_increase, int maps, float static_percentage);

std::vector<std::pair<glm::ivec2, float>> gen_obstacle_range(glm::ivec2 start_dims, glm::ivec2 dim_increase, int maps, std::vector<float> static_percentage);