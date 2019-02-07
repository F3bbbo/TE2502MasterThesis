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
	int add_drawable(Drawable&& object);
private:
	int counter = 0;
	void compile_shaders(ShaderPath&& input);

	std::map<int, Drawable> m_drawables;
	GLuint m_program = 0;
	bool m_valid = false;
};

#endif

