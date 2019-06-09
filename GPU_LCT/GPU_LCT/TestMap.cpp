#include "TestMap.hpp"
#define _USE_MATH_DEFINES
#include <math.h>

TestMap::TestMap()
{
	m_static_quota = 1.0f;
	m_dynamic_quota = 1.0f;
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

		// create the outer barrier to try and remove sliver triangles
		float wall_point_interval = 10.f;
		glm::ivec2 num_wall_intervals = m_num_obsticles / int(wall_point_interval) + 1;
		glm::vec2 pads = glm::vec2(0.0f);
		glm::vec2 wall_delta = ((m_end - pads) - (m_start + pads)) / glm::vec2(num_wall_intervals);
		glm::vec2 curr_pos = m_start + pads;
		std::vector<glm::vec2> wall_points;
		// do line in y-dir
		for (int y = 0; y < num_wall_intervals.y; y++)
		{
			wall_points.push_back(curr_pos);
			curr_pos.y += wall_delta.y;
		}
		// do line in x-dir
		for (int x = 0; x < num_wall_intervals.x; x++)
		{
			wall_points.push_back(curr_pos);
			curr_pos.x += wall_delta.x;
		}
		// do line in y-dir
		for (int y = 0; y < num_wall_intervals.y; y++)
		{
			wall_points.push_back(curr_pos);
			curr_pos.y -= wall_delta.y;
		}
		// do line in x-dir
		for (int x = 0; x < num_wall_intervals.x; x++)
		{
			wall_points.push_back(curr_pos);
			curr_pos.x -= wall_delta.x;
		}
		wall_points.push_back(curr_pos);
		m_wall = new Shape(wall_points);
		// add wall to shapes
		//m_shapes.push_back(wall);
		// start from beginning to fill map with objects
		curr_pos = m_start;
		// calculate the scale
		glm::vec2 obstacles_scale;
		obstacles_scale = -delta / 2.0f;
		glm::vec2 scale_variance = -delta / 8.0f;
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
					tmp = new Triangle();
				}
				shape_type = (shape_type + 1) % 2;
				tmp->set_location(curr_pos);
				float input = float(x + (y * m_num_obsticles.x)) * M_PI / 4.0f;
				glm::vec2 scale = obstacles_scale - (scale_variance * sinf(input));
				tmp->set_scale(scale);
				m_shapes.push_back(tmp);

			}
		}



		m_dirty = false;
	}
}

std::vector<std::vector<glm::vec2>> TestMap::get_CPU_obstacles()
{
	generate_map();
	return generate_CPU_segments(m_shapes);
}

std::vector<std::vector<glm::vec2>> TestMap::get_CPU_static_obstacles()
{
	generate_map();
	auto shapes = get_static_shapes();
	return generate_CPU_segments(shapes);
}

std::vector<std::vector<glm::vec2>> TestMap::get_CPU_dynamic_obstacles()
{
	generate_map();
	auto shapes = get_dynamic_shapes();
	return generate_CPU_segments(shapes);
}

std::vector<std::vector<glm::vec2>> TestMap::get_CPU_frame()
{
	generate_map();
	std::vector<Shape*> wall({ m_wall });
	return generate_CPU_segments(wall);
}

std::pair<std::vector<glm::vec2>, std::vector<glm::ivec2>> TestMap::get_GPU_obstacles()
{
	generate_map();
	return generate_GPU_segments(m_shapes);
}

std::pair<std::vector<glm::vec2>, std::vector<glm::ivec2>> TestMap::get_GPU_static_obstacles()
{
	generate_map();
	auto statics = get_static_shapes();
	return generate_GPU_segments(statics);
}

std::pair<std::vector<glm::vec2>, std::vector<glm::ivec2>> TestMap::get_GPU_dynamic_obstacles()
{
	generate_map();
	auto dynamics = get_dynamic_shapes();
	return generate_GPU_segments(dynamics);
}

std::pair<std::vector<glm::vec2>, std::vector<glm::ivec2>> TestMap::get_GPU_frame()
{
	generate_map();
	std::pair<std::vector<glm::vec2>, std::vector<glm::ivec2>> ret_obs;
	int last_shape_stop = 0;
	Shape* shape = m_wall;
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
	return ret_obs;
}

void TestMap::set_num_obsticles(glm::ivec2 num)
{
	m_num_obsticles = num;
	m_dirty = true;
}

void TestMap::set_static_quota(float quota)
{
	m_static_quota = quota;
}

void TestMap::set_dynamic_quota(float quota)
{
	m_dynamic_quota = quota;
}

void TestMap::set_map_size(glm::vec2 start, glm::vec2 end)
{
	m_start = start;
	m_end = end;
	m_dirty = true;
}


std::pair<std::vector<glm::vec2>, std::vector<glm::ivec2>> TestMap::generate_GPU_segments(std::vector<Shape*>& shapes)
{
	std::pair<std::vector<glm::vec2>, std::vector<glm::ivec2>> ret_obs;
	int last_shape_stop = 0;
	for (auto shape : shapes)
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

std::vector<std::vector<glm::vec2>> TestMap::generate_CPU_segments(std::vector<Shape*>& shapes)
{
	std::vector<std::vector<glm::vec2>> ret_list;
	ret_list.reserve(shapes.size());
	for (auto shape : shapes)
	{
		ret_list.push_back(shape->get_segments());
	}
	return ret_list;
}

std::vector<Shape*> TestMap::get_static_shapes()
{
	std::vector<Shape*> statics;
	int last_full_num = 0;
	float counter = 0.0f;
	for (int i = 0; i < m_shapes.size(); i++)
	{
		counter += m_static_quota;
		if (last_full_num < int(counter))
		{
			statics.push_back(m_shapes[i]);
			last_full_num = int(counter);
		}
	}
	return statics;
}

std::vector<Shape*> TestMap::get_dynamic_shapes()
{
	std::vector<Shape*> dynamics;
	int static_last_full_num = 0;
	float static_counter = 0.0f;
	int dyna_last_full_num = 0;
	float dyna_counter = 0.0f;
	for (int i = 0; i < m_shapes.size(); i++)
	{
		// ignore statics objects
		static_counter += m_static_quota;
		if (static_last_full_num < int(static_counter))
		{
			static_last_full_num = int(static_counter);
		}
		else
		{
			dyna_counter += m_dynamic_quota;
			if (dyna_last_full_num < int(dyna_counter))
			{
				dynamics.push_back(m_shapes[i]);
				dyna_last_full_num = int(dyna_counter);
			}
		}

	}
	return dynamics;
}

void TestMap::clear_shapes()
{
	for (unsigned int i = 0; i < m_shapes.size(); i++)
	{
		delete m_shapes[i];
	}
	m_shapes.clear();
}
