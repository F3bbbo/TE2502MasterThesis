#include "DelaunayDebugPipeline.hpp"

DelaunayDebugPipeline::DelaunayDebugPipeline()
{
}

DelaunayDebugPipeline::DelaunayDebugPipeline(float screen_res)
{
	m_screen_res = screen_res;
}

DelaunayDebugPipeline::~DelaunayDebugPipeline()
{
}

void DelaunayDebugPipeline::draw()
{
	if (!m_circles.is_enabled())
		return;

	glViewport(0, 0, (GLsizei)m_screen_res, (GLsizei)m_screen_res);
	glUseProgram(m_passes[DELAUNAY_DEBUG_PASS]);
	GLuint color_location = glGetUniformLocation(m_passes[DELAUNAY_DEBUG_PASS], "input_data.color");
	GLuint circle_thiccness_location = glGetUniformLocation(m_passes[DELAUNAY_DEBUG_PASS], "input_data.circle_thiccness");
	GLuint screen_resolution_location = glGetUniformLocation(m_passes[DELAUNAY_DEBUG_PASS], "input_data.screen_resolution");
	m_circles.bind_VAO();
	auto& col = m_circles.get_circle_color();
	glUniform3f(color_location, col.r, col.g, col.b);
	glUniform1f(circle_thiccness_location, m_circles.get_circle_thiccness());
	glUniform1f(screen_resolution_location, m_screen_res);

	glDrawArrays(GL_TRIANGLES, 0, 6);
}

bool DelaunayDebugPipeline::is_compatible(int type)
{
	return true;
}
