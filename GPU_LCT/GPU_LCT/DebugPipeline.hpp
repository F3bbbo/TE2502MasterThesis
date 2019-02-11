#pragma once
#ifndef DEBUGPIPELINE_HPP
#define DEBUGPIPELINE_HPP

#include "Pipeline.h"
#include "DebugObject.hpp"

#include <map>

class DebugPipeline : public Pipeline
{
public:
	enum DebugPasses {DEBUG_PASS};
	DebugPipeline();
	void draw();
	int add_drawable(DebugObject&& object);
	bool is_compatible(int type);
	~DebugPipeline();
private:
	int counter = 0;
	std::map<int, DebugObject> m_debug_objects; // change to map if this doens't work
};

#endif