#pragma once
#ifndef BUFFER_HPP
#define BUFFER_HPP
#include <glad/glad.h>
#include <vector>

class Buffer
{
public:
	Buffer();
	~Buffer();

	template <typename Data>
	void create_buffer(GLuint type, std::vector<Data> data, GLuint usage, GLuint location = 0)
	{
		m_loc = location;
		m_type = type;
		m_usage = usage;
		if (GL_ARRAY_BUFFER == type)
		{
			glGenVertexArrays(1, &m_vao);
			glBindVertexArray(m_vao);
		}

		glGenBuffers(1, &m_buf);
		glBindBuffer(type, m_buf);
		glBufferData(type, sizeof(Data) * data.size(), data.data(), usage);
		m_valid = true;
		
		// Does not unbind the buffers
	}
	void create_buffer(GLuint type, GLuint usage, GLuint location = 0);

	void set_vertex_attribute(GLuint location, GLuint size, GLuint type, GLuint stride, GLuint offset, GLboolean normalized = false);
	void bind_buffer();
	void unbind_buffer();
	bool is_valid();

	template <typename Data>
	void update_buffer(std::vector<Data> data)
	{
		glBindBuffer(m_type, m_buf);
		glBufferData(m_type, sizeof(Data) * data.size(), data.data(), m_usage);
		m_valid = true;
	};
private:
	GLuint m_type;
	GLuint m_buf;
	GLuint m_vao;
	GLuint m_loc;
	GLuint m_usage;
	bool m_valid = false;
};
#endif BUFFER_HPP
