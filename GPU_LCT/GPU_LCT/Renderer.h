#pragma once

#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include "data_structures.hpp"
#include "Pipeline.h"

class Renderer
{
public:
	Renderer();
	~Renderer();

	template<typename T>
	void add_pipeline(T&& pipeline)
	{
		if (std::is_base_of<Pipeline, T>())
			m_pipelines.push_back(std::make_unique<T>(pipeline));
	}

	void run();
	void check_error();
	void set_debug_edge(SymEdge* start_edge);

	bool shut_down = false;
	SymEdge* m_current_edge = nullptr;
private:
	void draw_frame();
	void processInput();

	std::vector<std::unique_ptr<Pipeline>> m_pipelines;
	float m_dt;
	GLFWwindow* m_window = nullptr;

	// Debug walking variables

	bool m_pressed_r = false;
	bool m_pressed_n = false;
};

#endif