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
	Pipeline();
	~Pipeline();
	
	bool is_valid();
	void add_pipeline(int type, ShaderPath&& input);
	
	virtual void draw() = 0;
protected:
	void compile_shaders(int type, ShaderPath&& input);

	std::map<int, GLuint> m_passes;
	bool m_valid = false;
};

#endif

