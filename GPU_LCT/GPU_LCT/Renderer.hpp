#pragma once

#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "data_structures.hpp"
#include "Pipeline.hpp"
#include <glm/glm.hpp>
#include "GPU/data_structures.hpp"
#include "GPU/GPU_Mesh.hpp"
#include "GPU/GPU_CPU_Mesh.hpp"
#include <array>
#include "Camera.hpp"

class Renderer
{
public:
	Renderer(glm::ivec2 screen_res);
	~Renderer();

	template<typename T>
	void add_pipeline(T&& pipeline)
	{
		if (std::is_base_of<Pipeline, T>())
			m_pipelines.push_back(std::make_unique<T>(pipeline));
	}

	void run();
	void set_debug_edge(CPU::SymEdge* start_edge, GPU::SymEdge gc_start_edge);
	glm::ivec2 get_screen_res();
	bool mouse_clicked();
	glm::vec2 get_mouse_pos();
	void set_gpu_mesh(GPU::GPUMesh* gpu_mesh);
	void set_gc_mesh(GPU::GCMesh* gc_mesh);
	std::array<glm::vec2, 2> get_GPU_edge();
	std::array<glm::vec2, 2> get_gc_edge();
	bool left_symedge_activated();

	bool shut_down = false;
	CPU::SymEdge* m_current_edge = nullptr;
	GPU::SymEdge m_gc_current_edge;

	GPU::SymEdge m_current_GPU_edge;
private:
	void draw_frame();
	void processInput();

	std::vector<std::unique_ptr<Pipeline>> m_pipelines;
	float m_dt;
	GLFWwindow* m_window = nullptr;
	glm::ivec2 m_screen_res = { 600, 600 };
	GPU::GPUMesh* m_gpu_mesh;
	GPU::GCMesh* m_gc_mesh;
	bool m_GPU_edge_dirty = true;
	std::array<glm::vec2, 2> m_curr_gpu_edge;
	
	Camera m_camera;
	glm::vec2 m_scroll_mouse_pos;
	bool m_middle_mouse_button_pressed = false;
	// Debug walking variables
	bool m_update_both_symedges = false;
	bool m_left_side = true;
	bool m_pressed_r = false;
	bool m_pressed_n = false;
	bool m_pressed_mouse = false;
	bool m_click_mouse = false;
	glm::vec2 m_mouse_pos;

};
#endif
