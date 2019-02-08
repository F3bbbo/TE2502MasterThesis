#pragma once
#ifndef DEBUG_OBJECT_HPP
#define DEBUG_OBJECT_HPP

#include "Drawable.h"
class DebugObject : public Drawable
{
public:
	void set_color(glm::vec3&& color);
	DebugObject(Mesh& mesh, DRAW_MODE mode, glm::vec3 color);
	~DebugObject();
	void draw();
private:
	void construct_GL_objects(Mesh& mesh);

	GLuint m_VBO = 0;
	GLuint m_EBO_edges = 0;
	GLuint m_EBO_faces = 0;
	GLuint m_VAO = 0;

	GLuint m_num_edges = 0;
	GLuint m_num_faces = 0;

	glm::vec3 m_color = {1.f, 1.f, 1.f};
};

#endif