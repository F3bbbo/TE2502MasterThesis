#include "Pipeline.hpp"


Pipeline::Pipeline()
{
}

Pipeline::~Pipeline()
{
}

bool Pipeline::is_valid()
{
	return m_valid;
}

void Pipeline::add_pass(int type, ShaderPath && input)
{
	if (is_compatible(type))
		compile_shaders(type, std::move(input));
	else
		LOG_T(CRITICAL, "Could not attach the given pipeline!");
}

void Pipeline::compile_shaders(int type, ShaderPath&& input)
{
	if (m_passes[type] != 0)
		glDeleteProgram(m_passes[type]);

	m_passes[type] = glCreateProgram();
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
		glAttachShader(m_passes[type], shader_object);
		glDeleteShader(shader_object);
	}

	glLinkProgram(m_passes[type]);
	// check for linking errors
	glGetProgramiv(m_passes[type], GL_LINK_STATUS, &success);
	if (!success) {
		glGetProgramInfoLog(m_passes[type], 512, NULL, infoLog);
		std::cout << "Program linking failed\n" << infoLog << '\n';
	}
	else
		m_valid = true;
}
