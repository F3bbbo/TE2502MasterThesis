#pragma once

#ifndef DELAUNAY_DEBUG_OBJECT_HPP
#define DELAUNAY_DEBUG_OBJECT_HPP

#include "Drawable.hpp"
#include "trig_functions.hpp"
#include <array>

#include "Buffer.hpp"
#include "GPU/GPU_Mesh.hpp"
#include "GPU/GPU_CPU_Mesh.hpp"

class DelaunayDebugObject : public Drawable
{
public:
	struct input_parameters
	{
		glm::vec3 color;
		float circle_thiccness;
		float screen_resolution;
		glm::vec2 viewport_offset;
		float pad;
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
	void set_draw_left_side(bool val);
	bool draw_left_side();
	DelaunayDebugObject();
	DelaunayDebugObject(CPU::Mesh& mesh);
	DelaunayDebugObject(GPU::GPUMesh& mesh);
	DelaunayDebugObject(GPU::GCMesh& mesh);
	~DelaunayDebugObject();
	void bind_VAO();
	bool is_enabled();
	void enable(bool value);
private:
	void build(CPU::Mesh& mesh);
	void build(GPU::GPUMesh& mesh);
	void build(GPU::GCMesh& mesh);

	Buffer m_vertex_buffer;
	Buffer m_circle_buffer;

	bool m_draw_left_side = true;

	glm::vec3 m_circle_color = { 1.f, 1.f, 1.f };
	float m_circle_thiccness = 1.f;
	bool m_enabled = false;
};
#endif