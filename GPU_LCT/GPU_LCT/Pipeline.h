#pragma once
#ifndef PIPELINE_HPP
#define PIPELINE_HPP

#include <glad/glad.h>
#include "Drawable.h"
#include <map>
#include <string>
#include <iostream>
#include <fstream>

enum Shader
{
	VS = GL_VERTEX_SHADER, FS = GL_FRAGMENT_SHADER, CS = GL_COMPUTE_SHADER
};

using ShaderPath = std::map<Shader, const char*>;
class Pipeline
{
public:
	Pipeline(ShaderPath&& input);
	~Pipeline();
	bool is_valid();
	virtual void draw() = 0;

	template <typename Object>
	int add_drawable(Object&& object)
	{
		m_drawables[counter] = std::make_unique<Object>(object);
		return counter++;
	};
protected:
	void compile_shaders(ShaderPath&& input);

	int counter = 0;
	std::map<int, std::unique_ptr<Drawable>> m_drawables;
	GLuint m_program = 0;
	bool m_valid = false;
};

#endif

