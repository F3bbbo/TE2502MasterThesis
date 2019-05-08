#include "Buffer.hpp"

Buffer::Buffer()
{
}

Buffer::~Buffer()
{
	glDeleteBuffers(1, &m_buf);
	glDeleteBuffers(1, &m_vao);
}

void Buffer::set_unitform_buffer_block(GLuint program, const char * buffer_name)
{
	glUniformBlockBinding(program, glGetUniformBlockIndex(program, buffer_name), m_loc);
}

void Buffer::set_vertex_attribute(GLuint location, GLuint size, GLuint type, GLuint stride, GLuint offset, GLboolean normalized)
{
	glBindVertexArray(m_vao);
	glVertexAttribPointer(location, size, type, normalized, stride, (void*)offset);
	glEnableVertexAttribArray(location);
	glBindVertexArray(0);
}

void Buffer::bind_buffer()
{
	// Bind should ony be used when drawing or dispatching a compute shader and when setting vertex attributes.
	// Do not try to bind a buffer to e.g. update its content, the bind call will not affect the update in any way.

	if (GL_SHADER_STORAGE_BUFFER == m_type || GL_UNIFORM_BUFFER == m_type)
	{
		// If an offset ever is provided it has to be a multiple of GL_SHADER_STORAGE_BUFFER_OFFSET_ALIGNMENT for shader storage objects, uniform buffer objects have their own value that needs to be queried.
		// No such requirements are needed for the size parameter

		glBindBufferRange(m_type, m_loc, m_buf, 0, m_used_buffer_size);
		return;
	}

	if (GL_ARRAY_BUFFER == m_type)
		glBindVertexArray(m_vao);

	glBindBuffer(m_type, m_buf);
}

void Buffer::unbind_buffer()
{
	if (GL_SHADER_STORAGE_BUFFER == m_type || GL_UNIFORM_BUFFER == m_type)
	{
		glBindBufferBase(m_type, m_loc, 0);
		return;
	}

	if (GL_ARRAY_BUFFER == m_type)
		glBindVertexArray(0);

	glBindBuffer(m_type, 0);
}

bool Buffer::is_valid()
{
	return m_valid;
}

int Buffer::element_count()
{
	return m_num_elements;
}

GLuint Buffer::buffer_size()
{
	return m_buffer_size;
}

void Buffer::clear()
{
	unbind_buffer();
	glNamedBufferData(m_buf, 0, NULL, m_usage);
	m_buffer_size = 0;
	m_num_elements = 0;
	m_used_buffer_size = 0;
	m_valid = false;
}
