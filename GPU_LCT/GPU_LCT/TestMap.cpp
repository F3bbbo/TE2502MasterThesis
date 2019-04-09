#include "TestMap.hpp"

TestMap::TestMap()
{
	m_obsticles_scale = 0.01f;
}

TestMap::~TestMap()
{
	clear_shapes();
}

void TestMap::generate_map()
{
	if (m_dirty) {
		clear_shapes();
		m_shapes.reserve(m_num_obsticles.x * m_num_obsticles.y);

		glm::vec2 delta;
		delta.x = (m_end.x - m_start.x) / (m_num_obsticles.x + 1);
		delta.y = (m_end.y - m_start.y) / (m_num_obsticles.y + 1);

		glm::vec2 curr_pos = m_start;
		// add obsticles inside the map area according to specified values

		int shape_type = 0;
		for (unsigned int y = 0; y < m_num_obsticles.y; y++)
		{
			curr_pos.y += delta.y;
			curr_pos.x = m_start.x;
			for (unsigned int x = 0; x < m_num_obsticles.x; x++)
			{
				curr_pos.x += delta.x;
				Shape* tmp;
				if (shape_type == 0)
				{
					tmp = new Triangle();
				}
				else
				{
					tmp = new Square();
				}
				shape_type = (shape_type + 1) % 2;
				tmp->set_location(curr_pos);
				tmp->set_scale(m_obsticles_scale);
				m_shapes.push_back(tmp);

			}
		}



		m_dirty = false;
	}
}

std::vector<std::vector<glm::vec2>> TestMap::get_CPU_obsticles()
{
	generate_map();
	std::vector<std::vector<glm::vec2>> ret_list;
	ret_list.reserve(m_shapes.size());
	for (auto shape : m_shapes)
	{
		ret_list.push_back(shape->get_segments());
	}
	return ret_list;
}

std::pair<std::vector<glm::vec2>, std::vector<glm::ivec2>> TestMap::get_GPU_obstacles()
{
	generate_map();
	std::pair<std::vector<glm::vec2>, std::vector<glm::ivec2>> ret_obs;
	int last_shape_stop = 0;
	for (auto shape : m_shapes)
	{
		auto new_points = shape->get_segments();
		ret_obs.first.insert(ret_obs.first.end(), new_points.begin(), std::prev(new_points.end()));
		// add the segment indices
		unsigned int curr_i;
		for (unsigned int i = 0; i < new_points.size() - 2; i++)
		{
			curr_i = last_shape_stop + i;
			ret_obs.second.push_back(glm::ivec2(curr_i, curr_i + 1));
		}
		ret_obs.second.push_back(glm::ivec2(curr_i + 1, last_shape_stop));
		last_shape_stop = ret_obs.first.size();
	}
	return ret_obs;
}

void TestMap::set_num_obsticles(glm::ivec2 num)
{
	m_num_obsticles = num;
	m_dirty = true;
}

void TestMap::set_map_size(glm::vec2 start, glm::vec2 end)
{
	m_start = start;
	m_end = end;
	m_dirty = true;
}

void TestMap::set_obsticle_scale(float scale)
{
	m_obsticles_scale = scale;
	m_dirty = true;
}

void TestMap::clear_shapes()
{
	for (unsigned int i = 0; i < m_shapes.size(); i++)
	{
		delete m_shapes[i];
	}
	m_shapes.clear();
}
