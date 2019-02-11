#pragma once

#ifndef DRAWABLE_HPP
#define DRAWABLE_HPP

#include <glad/glad.h>
#include "Mesh.hpp"

enum DRAW_TYPES { DRAW_FACES, DRAW_EDGES, DRAW_POINTS, DRAW_ALL };

class Drawable
{
public:
	Drawable(DRAW_TYPES mode);
	Drawable();
	~Drawable();
	virtual void bind_VAO() = 0;
protected:
	virtual void construct_GL_objects(Mesh& mesh) = 0;
	DRAW_TYPES m_mode = DRAW_FACES;
};
#endif