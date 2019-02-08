#pragma once
#ifndef DRAWABLE_HPP
#define DRAWABLE_HPP

#include <glad/glad.h>
#include "Mesh.hpp"

enum DRAW_MODE { FACE, EDGE, BOTH };

class Drawable
{
public:
	Drawable(DRAW_MODE mode);
	Drawable();
	~Drawable();
	virtual void draw() = 0;
protected:
	virtual void construct_GL_objects(Mesh& mesh) = 0;
	DRAW_MODE m_mode = FACE;
};

#endif