#include "DebugPipeline.hpp"

DebugPipeline::DebugPipeline()
{
}

void DebugPipeline::draw()
{
	glUseProgram(m_passes[DEBUG_PASS]);
	GLuint color_location = glGetUniformLocation(m_passes[DEBUG_PASS], "color");
	for (auto& model : m_debug_objects)
	{
		model.second.bind_VAO();
		model.second.draw_object(color_location);
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

DebugPipeline::~DebugPipeline()
{
}
