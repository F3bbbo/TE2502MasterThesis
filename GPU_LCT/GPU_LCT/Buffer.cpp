#include "Buffer.hpp"

Buffer::Buffer()
{
}

Buffer::~Buffer()
{
}

void Buffer::create_unitialized_buffer(GLuint type, GLuint usage, GLuint location)
{
	m_loc = location;
	m_usage = usage;
	m_type = type;
	if (GL_ARRAY_BUFFER == type)
		glGenVertexArrays(1, &m_vao);
	glGenBuffers(1, &m_buf);
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
	if (GL_SHADER_STORAGE_BUFFER == m_type)
	{
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, m_loc, m_buf);
		return;
	}

	if (GL_ARRAY_BUFFER == m_type)
		glBindVertexArray(m_vao);

	glBindBuffer(m_type, m_buf);
}

void Buffer::unbind_buffer()
{
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
