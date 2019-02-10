#include "DebugObject.hpp"
#include "Log.hpp"

DebugObject::DebugObject()
{
}

DebugObject::DebugObject(Mesh& mesh, DRAW_MODE mode, glm::vec3 face_color, glm::vec3 edge_color) : Drawable(mode), m_face_color(face_color), m_edge_color(edge_color)
{
	construct_GL_objects(mesh);
}


DebugObject::~DebugObject()
{
}

void DebugObject::bind_VAO()
{
	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
}

void DebugObject::draw_object(GLuint color_location)
{
	if (m_mode == FACE || m_mode == BOTH)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO_faces);
		glUniform3f(color_location, m_face_color.r, m_face_color.g, m_face_color.b);
		glDrawElements(GL_TRIANGLES, m_num_faces, GL_UNSIGNED_INT, 0);
	}
	if (m_mode == EDGE || m_mode == BOTH)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO_edges);
		glUniform3f(color_location, m_edge_color.r, m_edge_color.g, m_edge_color.b);
		glDrawElements(GL_LINES, m_num_edges, GL_UNSIGNED_INT, 0);
	}
}

bool DebugObject::is_valid()
{
	return m_VBO != 0;
}

void DebugObject::set_edge_color(glm::vec3 && color)
{
	m_edge_color = std::move(color);
}

void DebugObject::set_face_color(glm::vec3 && color)
{
	m_face_color = std::move(color);
}

glm::vec3 const & DebugObject::get_edge_color()
{
	return m_edge_color;
}

glm::vec3 const & DebugObject::get_face_color()
{
	return m_face_color;
}

void DebugObject::construct_GL_objects(Mesh& mesh)
{
	if (m_VBO > 0)
		glDeleteBuffers(1, &m_VBO);
	if (m_VAO > 0)
		glDeleteBuffers(1, &m_VAO);

	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	std::vector<VertexRef> const& mesh_verts = mesh.get_vertex_list();
	std::vector<glm::vec2> vertices;

	for (auto& vertex : mesh_verts)
		vertices.push_back(vertex.vertice);

	glBindVertexArray(m_VAO);

	glGenBuffers(1, &m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	
	if (m_mode == EDGE || m_mode == BOTH)
	{
		std::vector<Edge> const&  mesh_edges = mesh.get_edge_list();
		std::vector<glm::ivec2> edges_indices;

		for (auto& edge : mesh_edges)
			edges_indices.push_back(edge.edge);

		m_num_edges = (GLuint)edges_indices.size() * 2;

		glGenBuffers(1, &m_EBO_edges);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO_edges);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(glm::ivec2) * edges_indices.size(), edges_indices.data(), GL_STATIC_DRAW);
	}

	if (m_mode == FACE || m_mode == BOTH)
	{
		std::vector<Face> const&  mesh_faces = mesh.get_face_list();
		std::vector<glm::ivec3> face_indices;

		for (auto& face : mesh_faces)
			face_indices.push_back(face.vert_i);

		m_num_faces = (GLuint)face_indices.size() * 3;

		glGenBuffers(1, &m_EBO_faces);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO_faces);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(glm::ivec3) * face_indices.size(), face_indices.data(), GL_STATIC_DRAW);
	}
	glBindVertexArray(0);
}
