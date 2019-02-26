#include "Renderer.hpp"

Renderer::Renderer()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	m_window = glfwCreateWindow(800, 600, "LCT", NULL, NULL);
	if (m_window == NULL)
	{
		LOG_T(CRITICAL, "Failed to create GLFW window");
		glfwTerminate();
	}
	glfwMakeContextCurrent(m_window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		LOG_T(CRITICAL, "Failed to initialize GLAD");
}

Renderer::~Renderer()
{
}

void Renderer::run()
{
	if (!glfwWindowShouldClose(m_window))
	{
		processInput();

		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		draw_frame();
		check_error();

		// check and call events and swap the buffers
		glfwPollEvents();
		glfwSwapBuffers(m_window);
	}
	else
		shut_down = true;
}

void Renderer::check_error()
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

void Renderer::set_debug_edge(SymEdge * start_edge)
{
	m_current_edge = start_edge;
}

void Renderer::draw_frame()
{
	for (auto& pipeline : m_pipelines)
		pipeline->draw();
}

void Renderer::processInput()
{
	if (glfwGetKey(m_window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(m_window, true);

	if (glfwGetKey(m_window, GLFW_KEY_R) == GLFW_PRESS)
	{
		m_pressed_r = true;
	}
	else if (m_pressed_r) {
		m_pressed_r = false;
		if (m_current_edge->rot != nullptr) {
			m_current_edge = m_current_edge->rot;
		}
		else {
			std::cout << "Rot is nullptr.\n";
		}
	}
	if (glfwGetKey(m_window, GLFW_KEY_N) == GLFW_PRESS)
	{
		m_pressed_n = true;
	}
	else if (m_pressed_n) {
		m_pressed_n = false;
		if (m_current_edge->nxt != nullptr) {
			m_current_edge = m_current_edge->nxt;
		}
		else {
			std::cout << "Next is nullptr.\n";
		}
	}
}
