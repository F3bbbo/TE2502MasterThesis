#pragma once
#ifndef TEST_MAP_HPP
#define TEST_MAP_HPP
#include "Shapes.hpp"
#include <vector>
#include <glm/glm.hpp>

class TestMap {
public:
	TestMap();
	virtual ~TestMap();
	void generate_map();
	std::vector<std::vector<glm::vec2>> get_obsticles();
	void set_num_obsticles(glm::ivec2 num);
	void set_map_size(glm::vec2 start, glm::vec2 end);
	void set_obsticle_scale(float scale);
private:
	void clear_shapes();
	bool m_dirty = true;
	glm::ivec2 m_num_obsticles;
	float m_obsticles_scale;
	glm::vec2 m_start;
	glm::vec2 m_end;
	std::vector<Shape*> m_shapes;
};


#endif
