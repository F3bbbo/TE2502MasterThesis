#pragma once
#ifndef DEBUGPIPELINE_HPP
#define DEBUGPIPELINE_HPP

#include "Pipeline.h"

class DebugPipeline : public Pipeline
{
public:
	DebugPipeline(ShaderPath&& input);
	void draw();
	~DebugPipeline();
};

#endif