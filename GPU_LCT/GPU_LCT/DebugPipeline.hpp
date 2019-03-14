#pragma once

#ifndef DEBUGPIPELINE_HPP
#define DEBUGPIPELINE_HPP

#include "Pipeline.hpp"
#include "DebugObject.hpp"

#include <map>

class DebugPipeline : public Pipeline
{
public:
	enum DebugPasses { DEBUG_PASS };

	DebugPipeline(glm::ivec2 screen_res);
	~DebugPipeline();

	void draw();
	int add_drawable(DebugObject&& object);
	bool is_compatible(int type);
private:
	glm::ivec2 m_screen_res;
	int counter = 0;
	std::map<int, DebugObject> m_debug_objects;
};
#endif