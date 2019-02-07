#include "Pipeline.h"


Pipeline::Pipeline(ShaderPath&& input)
{
	compile_shaders(std::move(input));
}

Pipeline::~Pipeline()
{
}

bool Pipeline::is_valid()
{
	return m_valid;
}

int Pipeline::add_drawable(Drawable && object)
{
	m_drawables[counter] = std::move(object);
	return counter++;
}

void Pipeline::compile_shaders(ShaderPath&& input)
{
	m_program = glCreateProgram();
	char infoLog[512];
	int success;
	int shader_object;
	
	for (auto& shader_data : input)
	{
		std::ifstream shader_file;
		std::string str;
		shader_file.open(shader_data.second);
		while (!shader_file.eof())
		{
			std::string tmp;
			getline(shader_file, tmp);
			str += tmp + '\n';
		}
		shader_file.close();
		const char* c = str.c_str();
		shader_object = glCreateShader(shader_data.first);
		glShaderSource(shader_object, 1, &c, NULL);
		glCompileShader(shader_object);
		// check for shader compile errors
		glGetShaderiv(shader_object, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader_object, 512, NULL, infoLog);
			std::cout << "Shader compile failed\n" << infoLog << '\n';
		}
		glAttachShader(m_program, shader_object);
		glDeleteShader(shader_object);
	}

	glLinkProgram(m_program);
	// check for linking errors
	glGetProgramiv(m_program, GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(m_program, 512, NULL, infoLog);
		std::cout << "Program linking failed\n" << infoLog << '\n';
	}
	else
		m_valid = true;
}
