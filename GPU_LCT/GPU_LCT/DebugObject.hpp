#pragma once

#ifndef DEBUG_OBJECT_HPP
#define DEBUG_OBJECT_HPP

#include "Drawable.hpp"
#include "Buffer.hpp"
#include "GPU/GPU_Mesh.hpp"
#include "GPU/GPU_CPU_Mesh.hpp"

class DebugObject : public Drawable
{
public:
	struct DrawVertex
	{
		DrawVertex() {};
		DrawVertex(glm::vec2 v, glm::vec4 c) : vertex(v), color(c) {};
		glm::vec2 vertex;
		glm::vec4 color;
	};

	bool is_valid();

	void set_color(glm::vec3&& color);
	glm::vec3 const& get_color();

	void draw_constraints(bool value);
	void set_edge_thiccness(float thiccness);
	void set_point_thiccness(float thiccness);
	void set_draw_left_side(bool val);
	bool draw_left_side();

	DebugObject();
	DebugObject(DRAW_TYPES mode);
	DebugObject(std::array<glm::vec2, 2> vertices, DRAW_TYPES mode);
	void update_edge(std::array<glm::vec2, 2> vertices);
	~DebugObject();
	void bind_VAO();
	void draw_object();
	void build(CPU::Mesh& mesh);
	void build(GPU::GPUMesh& mesh);
	void build(GPU::GCMesh& mesh);
private:

	Buffer m_vertex_input;
	Buffer m_index_buffer;

	glm::vec3 m_color = { 1.f, 0.f, 0.f };

	float m_edge_thiccness = 1.f;
	float m_point_thiccness = 5.f;
	bool m_draw_constraints = false;
	bool m_draw_left_side = true;
};
#endif