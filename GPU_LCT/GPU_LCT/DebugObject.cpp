#include "DebugObject.hpp"
#include "Log.hpp"

DebugObject::DebugObject()
{
}

DebugObject::DebugObject(Mesh& mesh, DRAW_TYPES mode, bool draw_constraints) : Drawable(mode)
{
	m_draw_constraints = draw_constraints;
	construct_GL_objects(mesh);
}

DebugObject::DebugObject(std::array<glm::vec2, 2> vertices, DRAW_TYPES mode) : Drawable(mode)
{
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	m_num_points = 2;
	m_num_edges = 2;

	glGenBuffers(1, &m_EBO_edges);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO_edges);
	std::vector<glm::ivec2> edges_indices = { {0, 1} };
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(glm::ivec2) * 2, edges_indices.data(), GL_STATIC_DRAW);

	update_edge(vertices);
}

void DebugObject::update_edge(std::array<glm::vec2, 2> vertices)
{
	glBindVertexArray(m_VAO);

	std::array<DrawVertex, 2> dv;
	dv[0] = DrawVertex(vertices[0], glm::vec4(m_point_color.r, m_point_color.g, m_point_color.b, 1.f) );
	dv[1] = DrawVertex(vertices[1], glm::vec4(m_point_color.r, m_point_color.g, m_point_color.b, 0.f) );

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(DrawVertex) * 2, vertices.data(), GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
}


DebugObject::~DebugObject()
{
}

void DebugObject::bind_VAO()
{
	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
}

void DebugObject::draw_object()
{
	if (m_mode == DRAW_FACES || m_mode == DRAW_ALL)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO_faces);
		glDrawElements(GL_TRIANGLES, m_num_faces, GL_UNSIGNED_INT, 0);
	}
	if (m_mode == DRAW_EDGES || m_mode == DRAW_ALL)
	{
		glLineWidth(m_edge_thiccness);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO_edges);
		glDrawElements(GL_LINES, m_num_edges, GL_UNSIGNED_INT, 0);

		if (m_draw_constraints)
		{
			glLineWidth(m_edge_thiccness);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO_cedges);
			//glUniform3f(color_location, 0.1f, 0.6f, 0.1f);
			glDrawElements(GL_LINES, m_num_cedges, GL_UNSIGNED_INT, 0);
		}
	}
	if (m_mode == DRAW_POINTS || m_mode == DRAW_ALL)
	{
		glPointSize(m_point_thiccness);
		glDrawArrays(GL_POINTS, 0, m_num_points);
	}
}

bool DebugObject::is_valid()
{
	return m_VBO != 0;
}

void DebugObject::set_point_color(glm::vec3 && color)
{
	m_point_color = std::move(color);
}

void DebugObject::set_edge_color(glm::vec3 && color)
{
	m_edge_color = std::move(color);
}

void DebugObject::set_face_color(glm::vec3 && color)
{
	m_face_color = std::move(color);
}

glm::vec3 const & DebugObject::get_point_color()
{
	return m_point_color;
}

glm::vec3 const & DebugObject::get_edge_color()
{
	return m_edge_color;
}

glm::vec3 const & DebugObject::get_face_color()
{
	return m_face_color;
}

void DebugObject::set_edge_thiccness(float thiccness)
{
	m_edge_thiccness = thiccness;
}

void DebugObject::set_point_thiccness(float thiccness)
{
	m_point_thiccness = thiccness;
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
	std::vector<DrawVertex> vertices;

	for (auto& vertex : mesh_verts)
		vertices.push_back({ vertex.vertice, {m_point_color.r, m_point_color.g, m_point_color.b, 1.f} });

	m_num_points = (GLuint)vertices.size();

	glGenBuffers(1, &m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);

	glEnableVertexAttribArray(0);
	
	if (m_mode == DRAW_EDGES || m_mode == DRAW_ALL)
	{
		std::vector<Edge> const&  mesh_edges = mesh.get_edge_list();
		std::vector<glm::ivec2> edges_indices;

		for (auto& edge : mesh_edges)
			edges_indices.push_back(edge.edge);

		m_num_edges = (GLuint)edges_indices.size() * 2;

		glGenBuffers(1, &m_EBO_edges);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO_edges);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(glm::ivec2) * edges_indices.size(), edges_indices.data(), GL_STATIC_DRAW);

		if (m_draw_constraints)
		{
			std::vector<glm::ivec2> constraint_indices;

			for (auto& edge : mesh_edges)
			{
				if (edge.constraint_ref.size() > 0)
					constraint_indices.push_back(edge.edge);
			}

			m_num_cedges = (GLuint)constraint_indices.size() * 2;
			if (m_num_cedges > 0)
			{
				m_draw_constraints = true;
				glGenBuffers(1, &m_EBO_cedges);
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO_cedges);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(glm::ivec2) * constraint_indices.size(), constraint_indices.data(), GL_STATIC_DRAW);
			}
			else
				m_draw_constraints = false;
		}
	}

	if (m_mode == DRAW_FACES || m_mode == DRAW_ALL)
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
