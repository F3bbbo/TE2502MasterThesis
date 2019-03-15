#include "DelaunayDebugPipeline.hpp"

DelaunayDebugPipeline::DelaunayDebugPipeline()
{
}

DelaunayDebugPipeline::DelaunayDebugPipeline(glm::ivec2 screen_res)
{
	m_screen_res = screen_res;
}

DelaunayDebugPipeline::~DelaunayDebugPipeline()
{
}

void DelaunayDebugPipeline::draw()
{
	glUseProgram(m_passes[DELAUNAY_DEBUG_PASS]);
	GLuint color_location = glGetUniformLocation(m_passes[DELAUNAY_DEBUG_PASS], "input_data.color");
	GLuint circle_thiccness_location = glGetUniformLocation(m_passes[DELAUNAY_DEBUG_PASS], "input_data.circle_thiccness");
	GLuint screen_resolution_location = glGetUniformLocation(m_passes[DELAUNAY_DEBUG_PASS], "input_data.screen_resolution");
	GLuint viewport_offset = glGetUniformLocation(m_passes[DELAUNAY_DEBUG_PASS], "input_data.viewport_offset");
	glUniform1f(screen_resolution_location, (GLfloat)m_screen_res.y);

	for (auto& circle : m_circles)
	{
		if (!circle.is_enabled())
			continue;

		if (circle.draw_left_side())
		{
			glViewport(0, 0, m_screen_res.x / 2, m_screen_res.y);
			glUniform2f(viewport_offset, 0.f, 0.f);
		}
		else
		{
			glViewport(m_screen_res.x / 2, 0, m_screen_res.x / 2, m_screen_res.y);
			glUniform2f(viewport_offset, 800.f, 0.f);
		}
		circle.bind_VAO();
		auto& col = circle.get_circle_color();
		glUniform3f(color_location, col.r, col.g, col.b);
		glUniform1f(circle_thiccness_location, circle.get_circle_thiccness());
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}
}

bool DelaunayDebugPipeline::is_compatible(int type)
{
	return true;
}
