#include "Drawable.h"



Drawable::Drawable(Mesh & mesh, DRAW_MODE mode, glm::vec3 color) : m_mode(mode), m_color(color)
{
	construct_GL_objects(mesh);
}

Drawable::Drawable()
{
}

void Drawable::set_color(glm::vec3 && color)
{
	m_color = std::move(color);
}

Drawable::~Drawable()
{
}

void Drawable::construct_GL_objects(Mesh& mesh)
{
	if (m_VBO > 0)
		glDeleteBuffers(1, &m_VBO);
	if (m_VAO > 0)
		glDeleteBuffers(1, &m_VAO);

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

		glGenBuffers(1, &m_EBO_edges);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO_edges);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(glm::ivec3) * face_indices.size(), face_indices.data(), GL_STATIC_DRAW);
	}
	glBindVertexArray(0);
}
