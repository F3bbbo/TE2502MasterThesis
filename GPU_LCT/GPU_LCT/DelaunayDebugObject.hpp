#pragma once

#ifndef DELAUNAY_DEBUG_OBJECT_HPP
#define DELAUNAY_DEBUG_OBJECT_HPP

#include "Drawable.hpp"
#include "trig_functions.hpp"
#include <array>

class DelaunayDebugObject : public Drawable
{
public:
	struct input_parameters
	{
		glm::vec3 color;
		float circle_thiccness;
		float screen_resolution;
		glm::vec3 pad;
	};

	struct CircleData
	{
		glm::vec2 center;
		float radius;
		float pad;
	};

	void set_circle_color(glm::vec3&& color);
	glm::vec3 const& get_circle_color();

	void set_circle_thiccness(float thiccness);
	float get_circle_thiccness();

	DelaunayDebugObject();
	DelaunayDebugObject(Mesh& mesh);
	~DelaunayDebugObject();
	void bind_VAO();
	bool is_enabled();
	void enable(bool value);
private:
	void build(Mesh& mesh);

	GLuint m_VBO = 0;
	GLuint m_ssbo = 0;
	GLuint m_VAO = 0;

	GLuint m_num_circles = 0;

	glm::vec3 m_circle_color = { 1.f, 1.f, 1.f };
	float m_circle_thiccness = 1.f;
	bool m_enabled = false;
};
#endif