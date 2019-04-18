#pragma once

#ifndef PIPELINE_HPP
#define PIPELINE_HPP

#include <glad/glad.h>
#include "Log.hpp"
#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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
	void add_pass(int type, ShaderPath&& input);
	
	virtual bool is_compatible(int type) = 0;
	virtual void draw(glm::mat4x4& camera_matrix) = 0;
protected:
	void compile_shaders(int type, ShaderPath&& input);

	std::map<int, GLuint> m_passes;
	bool m_valid = false;
};
#endif

