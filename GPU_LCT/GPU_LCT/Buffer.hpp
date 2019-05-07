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

		// It would be cool if glgenBuffers() actually, you know, generated a freaking buffer instead of just reserving an integer. There does not exist a function which only purpose is to create buffers.
		glGenBuffers(1, &m_buf);

		// The only way to create a buffer is to bind it to some random target, geez...
		glBindBuffer(m_type, m_buf);
		glBindBuffer(m_type, 0);

		// If we forget to call glBindBuffer the data transfer will not happen and we will not even get an error, debug mode sure is convenient!
		glNamedBufferData(m_buf, m_buffer_size, NULL, usage);
		glNamedBufferSubData(m_buf, 0, m_used_buffer_size, data.data());

		m_valid = true;
	}

	template <typename Data>
	void create_uniform_buffer(Data data, GLuint usage, GLuint location = 0)
	{
		m_loc = location;
		m_type = GL_UNIFORM_BUFFER;
		m_usage = usage;
		m_num_elements = 1;
		m_buffer_size = sizeof(Data);
		m_used_buffer_size = m_buffer_size;
		glGenBuffers(1, &m_buf);
		bind_buffer();
		glBufferData(m_type, sizeof(Data), &data, usage);
		unbind_buffer();
		glBindBufferRange(m_type, 0, m_buf, 0, sizeof(Data));
	}

	void set_unitform_buffer_block(GLuint program, const char* buffer_name);

	template <typename Data>
	void append_to_buffer(std::vector<Data> data)
	{
		GLuint append_byte_length = sizeof(Data) * data.size();
		if (append_byte_length == 0)
			return;
		if (append_byte_length + m_used_buffer_size > m_buffer_size)
		{
			// Need to reallocate data

			// Get the current data from the buffer
			void* ptr = malloc(m_used_buffer_size);
			void* mapped_data = glMapNamedBufferRange(m_buf, 0, m_used_buffer_size, GL_MAP_READ_BIT);
			memcpy(ptr, mapped_data, m_used_buffer_size);
			glUnmapNamedBuffer(m_buf);

			// Create a bigger buffer
			GLuint buffer_increase = BUFFER_APPEND_BYTE_AMMOUNT + (m_used_buffer_size + append_byte_length - m_buffer_size - (BUFFER_APPEND_BYTE_AMMOUNT % (m_used_buffer_size + append_byte_length - m_buffer_size)));
			glNamedBufferData(m_buf, m_buffer_size + buffer_increase, NULL, m_usage);
			glNamedBufferSubData(m_buf, 0, m_used_buffer_size, ptr);
			glNamedBufferSubData(m_buf, m_used_buffer_size, append_byte_length, data.data());
			m_buffer_size += buffer_increase;
			free(ptr);
		}
		else
		{
			// Use unused buffer space
			void* mapped_data = glMapNamedBufferRange(m_buf, m_used_buffer_size, append_byte_length, GL_MAP_WRITE_BIT);
			memcpy(mapped_data, data.data(), append_byte_length);
			glUnmapNamedBuffer(m_buf);
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
		Data* ptr = (Data*)malloc(elements * sizeof(Data));
		glGetNamedBufferSubData(m_buf, element_offset * sizeof(Data), elements * sizeof(Data), ptr);
		std::vector<Data> data;
		int end_index = elements == 0 ? m_num_elements : glm::min(element_offset + elements, m_num_elements);
		for (int i = 0; i < elements; i++)
			data.push_back(ptr[i]);
		free(ptr);
		return data;
	}

	template <typename Type>
	void set_used_element_count(int number)
	{
		GLuint new_size = number * sizeof(Type);
		if (new_size > m_buffer_size)
		{
			// Get the current data from the buffer
			void* ptr = malloc(m_used_buffer_size);
			void* mapped_data = glMapNamedBufferRange(m_buf, 0, m_used_buffer_size, GL_MAP_READ_BIT);
			memcpy(ptr, mapped_data, m_used_buffer_size);
			glUnmapNamedBuffer(m_buf);

			// Create a bigger buffer
			m_buffer_size = new_size;
			glNamedBufferData(m_buf, m_buffer_size, NULL, m_usage);
			glNamedBufferSubData(m_buf, 0, m_used_buffer_size, ptr);
			free(ptr);
			m_used_buffer_size = m_buffer_size;
			m_num_elements = number;
		}
		else
		{
			m_used_buffer_size = number * sizeof(Type);
			m_num_elements = number;
		}
	}

	void set_vertex_attribute(GLuint location, GLuint size, GLuint type, GLuint stride, GLuint offset, GLboolean normalized = false);
	void bind_buffer();
	void unbind_buffer();
	bool is_valid();

	int element_count();
	GLuint buffer_size();

	void clear();

	template <typename Data>
	void update_buffer(std::vector<Data> data)
	{
		glNamedBufferData(m_buf, sizeof(Data) * data.size(), data.data(), m_usage);
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
