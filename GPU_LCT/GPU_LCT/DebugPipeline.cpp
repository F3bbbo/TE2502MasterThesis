#include "DebugPipeline.hpp"

DebugPipeline::DebugPipeline(ShaderPath&& input) : Pipeline(std::move(input))
{
}

void DebugPipeline::draw()
{
	glUseProgram(m_program);
	for (auto& model : m_drawables)
		model.second->draw();
}

DebugPipeline::~DebugPipeline()
{
}
