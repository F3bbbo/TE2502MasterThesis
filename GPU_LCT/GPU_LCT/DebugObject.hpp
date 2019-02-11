#pragma once

#ifndef DEBUG_OBJECT_HPP
#define DEBUG_OBJECT_HPP

#include "Drawable.h"

class DebugObject : public Drawable
{
public:
	bool is_valid();
	
	void set_point_color(glm::vec3&& color);
	void set_edge_color(glm::vec3&& color);
	void set_face_color(glm::vec3&& color);
	
	glm::vec3 const& get_point_color();
	glm::vec3 const& get_edge_color();
	glm::vec3 const& get_face_color();

	void set_edge_thiccness(float thiccness);
	void set_point_thiccness(float thiccness);

	DebugObject();
	DebugObject(Mesh& mesh, DRAW_TYPES mode);
	~DebugObject();
	void bind_VAO();
	void draw_object(GLuint color_location);
private:
	void construct_GL_objects(Mesh& mesh);

	GLuint m_VBO = 0;
	GLuint m_EBO_edges = 0;
	GLuint m_EBO_faces = 0;
	GLuint m_VAO = 0;

	GLuint m_num_points = 0;
	GLuint m_num_edges = 0;
	GLuint m_num_faces = 0;

	glm::vec3 m_point_color = {0.5f, 0.5f, 0.5f};
	glm::vec3 m_edge_color = {0.f, 0.f, 0.f};
	glm::vec3 m_face_color = {1.f, 1.f, 1.f};

	float m_edge_thiccness = 1.f;
	float m_point_thiccness = 5.f;
};
#endif