#include "Renderer.hpp"
void GLAPIENTRY
MessageCallback(GLenum source,
	GLenum type,
	GLuint id,
	GLenum severity,
	GLsizei length,
	const GLchar* message,
	const void* userParam)
{
	if (type != GL_DEBUG_TYPE_ERROR)
		return;
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
}

Renderer::Renderer(glm::ivec2 screen_res)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	m_screen_res = screen_res;
	m_window = glfwCreateWindow(screen_res.x, screen_res.y, "LCT", NULL, NULL);
	if (m_window == NULL)
	{
		LOG_T(CRITICAL, "Failed to create GLFW window");
		glfwTerminate();
	}
	glfwMakeContextCurrent(m_window);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		LOG_T(CRITICAL, "Failed to initialize GLAD");

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_MULTISAMPLE);
	glEnable(GL_DEBUG_OUTPUT);
	//Set up debug callback
	glDebugMessageCallback(MessageCallback, NULL);
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

		// check and call events and swap the buffers
		glfwPollEvents();
		glfwSwapBuffers(m_window);
	}
	else
		shut_down = true;
}

void Renderer::set_debug_edge(CPU::SymEdge * start_edge)
{
	m_current_edge = start_edge;
}

glm::ivec2 Renderer::get_screen_res()
{
	return m_screen_res;
}

bool Renderer::mouse_clicked()
{
	return m_click_mouse;
}

glm::vec2 Renderer::get_mouse_pos()
{
	m_click_mouse = false;
	return mouse_pos;
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

	if (glfwGetKey(m_window, GLFW_KEY_LEFT) == GLFW_PRESS)
		m_left_side = true;
	if (glfwGetKey(m_window, GLFW_KEY_RIGHT) == GLFW_PRESS)
		m_left_side = false;

	if (glfwGetKey(m_window, GLFW_KEY_T) == GLFW_PRESS)
	{
		m_update_both_symedges = m_update_both_symedges ? false : true;
	}
	if (glfwGetKey(m_window, GLFW_KEY_R) == GLFW_PRESS)
	{
		m_pressed_r = true;
	}
	else if (m_pressed_r) {
		m_pressed_r = false;
		bool failed = false;

		if (m_left_side)
		{
			if (m_current_edge->rot != nullptr) {
				m_current_edge = m_current_edge->rot;
			}
			else
				failed = true;
		}
		if (!m_left_side || m_update_both_symedges)
		{
			if (m_current_GPU_edge.rot != -1)
			{
				m_GPU_edge_dirty = true;
				m_current_GPU_edge = m_gpu_mesh->get_symedge(m_current_GPU_edge.rot);
			}
			else
				failed = true;
		}
		if (failed)
			LOG("Rot is nullptr.");
	}
	if (glfwGetKey(m_window, GLFW_KEY_N) == GLFW_PRESS)
	{
		m_pressed_n = true;
	}
	else if (m_pressed_n) {
		m_pressed_n = false;
		bool failed = false;

		if (m_left_side)
		{
			if (m_current_edge->nxt != nullptr) {
				m_current_edge = m_current_edge->nxt;
			}
			else
				failed = true;
		}
		if (!m_left_side || m_update_both_symedges)
		{
			if (m_current_GPU_edge.nxt != -1)
			{
				m_GPU_edge_dirty = true;
				m_current_GPU_edge = m_gpu_mesh->get_symedge(m_current_GPU_edge.nxt);
			}
			else
				failed = true;
		}
		if (failed)
			LOG("Next is nullptr.");
	}

	if (glfwGetMouseButton(m_window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		m_pressed_mouse = true;
	}
	else if (m_pressed_mouse) {
		m_pressed_mouse = false;
		double x, y;
		glfwGetCursorPos(m_window, &x, &y);
		x = x / (m_screen_res.x / 2.0) * 2.0 - 1.0;
		y = -(y / m_screen_res.y * 2.0 - 1.0);
		mouse_pos = glm::vec2(x, y);
		m_click_mouse = true;
	}
}

void Renderer::set_gpu_mesh(GPU::GPUMesh * gpu_mesh)
{
	m_gpu_mesh = gpu_mesh;
	m_current_GPU_edge = m_gpu_mesh->get_symedge(0);
}

std::array<glm::vec2, 2> Renderer::get_GPU_edge()
{
	if (m_GPU_edge_dirty)
	{
		std::array<glm::vec2, 2> edge;
		edge[0] = m_gpu_mesh->get_vertex(m_current_GPU_edge.vertex);
		GPU::SymEdge nxt_symedge = m_gpu_mesh->get_symedge(m_current_GPU_edge.nxt);
		edge[1] = m_gpu_mesh->get_vertex(nxt_symedge.vertex);
		m_curr_gpu_edge = edge;
		m_GPU_edge_dirty = false;
	}
	return m_curr_gpu_edge;
}

bool Renderer::left_symedge_activated()
{
	return m_left_side;
}