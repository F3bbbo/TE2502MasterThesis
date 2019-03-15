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
	DelaunayDebugPipeline(glm::ivec2 screen_res);
	~DelaunayDebugPipeline();

	std::vector<DelaunayDebugObject> m_circles;
	void draw();
	bool is_compatible(int type);
	glm::ivec2 m_screen_res;
};
#endif