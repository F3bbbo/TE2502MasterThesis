// GPU_LCT.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "Mesh.hpp"
#include "DebugObject.hpp"
#include "DebugPipeline.hpp"
#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "Log.hpp"

void checkError();

void processInput(GLFWwindow *window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}
int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	GLFWwindow* window = glfwCreateWindow(800, 600, "LCT", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	Mesh m;
	m.Initialize_as_quad({ 0.5f, 0.5f }, { 0.f, 0.f });

	DebugObject drawable(m, BOTH, { 1.0f, 0.5f, 0.2f, });

	ShaderPath debug_draw_path;
	debug_draw_path[VS] = "debug_vertex_shader.glsl";
	debug_draw_path[FS] = "debug_fragment_shader.glsl";

	DebugPipeline debug_draw(std::move(debug_draw_path));
	debug_draw.add_drawable(std::move(drawable));

	// render loop
	while (!glfwWindowShouldClose(window))
	{
		// input
		processInput(window);

		// rendering commands here
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		debug_draw.draw();
		checkError();

		// check and call events and swap the buffers
		glfwPollEvents();
		glfwSwapBuffers(window);
	}
	return 0;
}

void checkError()
{
	GLenum err;
	while ((err = glGetError()) != GL_NO_ERROR)
	{
		if (err == 0x0500)
			LOG_T(WARNING, "INVALID ENUM");
		else if (err == 0x0501)
			LOG_T(WARNING, "INVALID VALUE");
		else if (err == 0x0502)
			LOG_T(WARNING, "INVALID OPERATION");
		else if (err == 0x0503)
			LOG_T(WARNING, "STACK OVERFLOW");
		else if (err == 0x0504)
			LOG_T(WARNING, "STACK UNDERFLOW");
		else if (err == 0x0505)
			LOG_T(WARNING, "GL OUT OF MEMORY");
		else if (err == 0x0506)
			LOG_T(WARNING, "GL INVALID FRAMEBUFFER OPERATION");
		else if (err == 0x0507)
			LOG_T(WARNING, "GL CONTEXT LOST");
	}
}

