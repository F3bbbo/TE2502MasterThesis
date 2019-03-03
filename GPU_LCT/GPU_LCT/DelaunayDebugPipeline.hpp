#pragma once

#ifndef DELAUNAY_DEBUG_PIPELINE_HPP
#define DELAUNAY_DEBUG_PIPELINE_HPP

#include "Pipeline.hpp"
#include "DelaunayDebugObject.hpp"

#include <map>

class DelaunayDebugPipeline : public Pipeline
{
public:
	enum DelaunayDebugPass { DELAUNAY_DEBUG_PASS };

	DelaunayDebugPipeline();
	DelaunayDebugPipeline(float screen_res);
	~DelaunayDebugPipeline();

	DelaunayDebugObject m_circles;
	void draw();
	bool is_compatible(int type);
	float m_screen_res;
};
#endif