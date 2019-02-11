#pragma once
#ifndef RENDERER_HPP
#define RENDERER_HPP

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
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
private:
	void draw_frame();
	void processInput();

	std::vector<std::unique_ptr<Pipeline>> m_pipelines;
	float m_dt;

	GLFWwindow* m_window = nullptr;
};

#endif