#pragma once
#ifndef DRAWABLE_HPP
#define DRAWABLE_HPP

#include <glad/glad.h>
#include "Mesh.hpp"

enum DRAW_MODE { FACE, EDGE, BOTH };

class Drawable
{
public:
	Drawable(Mesh& mesh, DRAW_MODE mode, glm::vec3 color);
	Drawable();
	~Drawable();
	void set_color(glm::vec3&& color);
private:
	void construct_GL_objects(Mesh& mesh);

	GLuint m_VBO = 0;
	GLuint m_EBO_edges = 0;
	GLuint m_EBO_faces = 0;
	GLuint m_VAO = 0;

	DRAW_MODE m_mode = FACE;
	glm::vec3 m_color = {1.f, 1.f, 1.f};
};

#endif