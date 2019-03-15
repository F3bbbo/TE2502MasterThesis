#pragma once
#ifndef BUFFER_HPP
#define BUFFER_HPP
#include <glad/glad.h>
#include <vector>
#include "Log.hpp"

// Guarantees a buffer increase of atleast the specidifed amount
#define BUFFER_APPEND_BYTE_AMMOUNT 10000

class Buffer
{
public:
	Buffer();
	~Buffer();

	template <typename Data>
	void create_buffer(GLuint type, std::vector<Data> data, GLuint usage, GLuint location = 0, GLuint preallocate_size = 0)
	{
		m_num_elements = data.size();
		if (preallocate_size != 0)
		{
			m_buffer_size = sizeof(Data) * preallocate_size;
			m_used_buffer_size = sizeof(Data) * m_num_elements;
			if (m_used_buffer_size > m_buffer_size)
				m_buffer_size = m_used_buffer_size;
		}
		else
		{
			m_buffer_size = sizeof(Data) * m_num_elements;
			m_used_buffer_size = m_buffer_size;
		}
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
		glBufferData(type, m_buffer_size, NULL, usage);
		glBufferSubData(type, 0, m_used_buffer_size, data.data());

		m_valid = true;

		// Does not unbind the buffers
	}

	template <typename Data>
	void create_uniform_buffer(Data data, GLuint usage, GLuint location = 0)
	{
		m_loc = location;
		m_type = GL_UNIFORM_BUFFER;
		m_usage = usage;
		glGenBuffers(1, &m_buf);
		glBindBuffer(m_type, m_buf);
		glBufferData(m_type, sizeof(Data), &data, usage);

	}

	template <typename Data>
	void append_to_buffer(std::vector<Data> data)
	{
		GLuint append_byte_length = sizeof(Data) * data.size();
		glBindBuffer(m_type, m_buf);
		if (append_byte_length + m_used_buffer_size > m_buffer_size)
		{
			// Need to reallocate data

			// Get the current data from the buffer
			void* ptr = malloc(m_used_buffer_size);
			void* mapped_data = glMapBufferRange(m_type, 0, m_used_buffer_size, GL_MAP_READ_BIT);
			memcpy(ptr, mapped_data, m_used_buffer_size);
			glUnmapBuffer(m_type);

			// Create a bigger buffer
			GLuint buffer_increase = BUFFER_APPEND_BYTE_AMMOUNT + (sizeof(Data) - BUFFER_APPEND_BYTE_AMMOUNT % sizeof(Data));
			glBufferData(m_type, m_buffer_size + buffer_increase, NULL, m_usage);
			glBufferSubData(m_type, 0, m_used_buffer_size, ptr);
			glBufferSubData(m_type, m_used_buffer_size, append_byte_length, data.data());
			m_buffer_size += buffer_increase;
			free(ptr);
		}
		else
		{
			// Use unused buffer space
			void* mapped_data = glMapBufferRange(m_type, m_used_buffer_size, append_byte_length, GL_MAP_WRITE_BIT);
			memcpy(mapped_data, data.data(), append_byte_length);
			glUnmapBuffer(m_type);
		}
		m_used_buffer_size += append_byte_length;
		m_num_elements += data.size();
	}

	template <typename Data>
	std::vector<Data> get_buffer_data(int element_offset = 0, int elements = 0)
	{
		if (element_offset > m_num_elements)
			element_offset = m_num_elements;
		if (element_offset + elements > m_num_elements)
			elements = m_num_elements - element_offset;
		if (elements == 0)
			elements = m_num_elements;
		Data* ptr = (Data*)malloc(m_used_buffer_size);
		bind_buffer();
		glGetBufferSubData(m_type, element_offset * sizeof(Data), elements * sizeof(Data), ptr);
		unbind_buffer();
		std::vector<Data> data;
		for (int i = element_offset; i < m_num_elements; i++)
			data.push_back(ptr[i]);
		free(ptr);
		return data;
	}

	void create_unitialized_buffer(GLuint type, GLuint usage, GLuint location = 0); // Creates a unitialized buffer with no size;

	void set_vertex_attribute(GLuint location, GLuint size, GLuint type, GLuint stride, GLuint offset, GLboolean normalized = false);
	void bind_buffer();
	void unbind_buffer();
	bool is_valid();

	int element_count();
	GLuint buffer_size();

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
	GLuint m_buffer_size = 0; // bytes
	GLuint m_used_buffer_size = 0; // bytes
	int m_num_elements = 0;
	bool m_valid = false;
};
#endif BUFFER_HPP
