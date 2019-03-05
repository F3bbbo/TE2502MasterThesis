#include "DebugPipeline.hpp"

DebugPipeline::DebugPipeline()
{
}

void DebugPipeline::draw()
{
	glUseProgram(m_passes[DEBUG_PASS]);
	for (auto& model : m_debug_objects)
	{
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
