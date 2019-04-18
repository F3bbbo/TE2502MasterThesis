#include "DebugPipeline.hpp"

DebugPipeline::DebugPipeline(glm::ivec2 screen_res)
{
	m_screen_res = screen_res;
}

void DebugPipeline::draw(glm::mat4x4& camera_matrix)
{
	glUseProgram(m_passes[DEBUG_PASS]);

	int matrix_location = glGetUniformLocation(m_passes[DEBUG_PASS], "matrix");
	glUniformMatrix4fv(matrix_location, 1, GL_FALSE, glm::value_ptr(camera_matrix));

	for (auto& model : m_debug_objects)
	{
		if (model.second.draw_left_side())
			glViewport(0, 0, m_screen_res.x / 2, m_screen_res.y);
		else
			glViewport(m_screen_res.x / 2, 0, m_screen_res.x / 2, m_screen_res.y);
		model.second.bind_VAO();
		model.second.draw_object();
	}
}

int DebugPipeline::add_drawable(DebugObject&& object)
{
	if (object.is_valid())
	{
		m_debug_objects[counter] = object;
		return counter++;
	}
	else
		return -1;
}

bool DebugPipeline::is_compatible(int type)
{
	DebugPasses pass = static_cast<DebugPasses>(type);
	return pass == DEBUG_PASS;
}

DebugPipeline::~DebugPipeline()
{
}
