#pragma once
#include "GPU/GPU_Mesh.hpp"
#include "GPU/GPU_CPU_Mesh.hpp"
#include "TestMap.hpp"
#include "Timer.hpp"

void test_range(glm::ivec2 start_resolution, int iterations, glm::ivec2 start_dims, glm::ivec2 dim_increase, glm::ivec2 start_obstacles, glm::ivec2 obstacle_increase, bool build_lct);

void first_test(glm::vec2 static_obstacle_amount, int iterations);
