#include "DebugObject.hpp"
#include "Log.hpp"

DebugObject::DebugObject()
{
}

DebugObject::DebugObject(DRAW_TYPES mode) : Drawable(mode)
{
}

DebugObject::DebugObject(std::array<glm::vec2, 2> vertices, DRAW_TYPES mode) : Drawable(mode)
{
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);
	m_num_primitives = 2;

	glGenBuffers(1, &m_EBO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
	std::vector<glm::ivec2> edges_indices = { {0, 1} };
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(glm::ivec2) * 2, edges_indices.data(), GL_STATIC_DRAW);

	update_edge(vertices);
}

void DebugObject::update_edge(std::array<glm::vec2, 2> vertices)
{
	glBindVertexArray(m_VAO);

	std::array<DrawVertex, 2> dv;
	dv[0] = DrawVertex(vertices[0], glm::vec4(m_color.r, m_color.g, m_color.b, 1.f) );
	dv[1] = DrawVertex(vertices[1], glm::vec4(m_color.r, m_color.g, m_color.b, 0.f) );

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(DrawVertex) * 2, dv.data(), GL_DYNAMIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);

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
	if (m_mode == DRAW_FACES)
	{
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glDrawElements(GL_TRIANGLES, m_num_primitives, GL_UNSIGNED_INT, 0);
	}
	if (m_mode == DRAW_EDGES)
	{
		glLineWidth(m_edge_thiccness);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glDrawElements(GL_LINES, m_num_primitives, GL_UNSIGNED_INT, 0);
	}
	if (m_mode == DRAW_POINTS)
	{
		glPointSize(m_point_thiccness);
		glDrawArrays(GL_POINTS, 0, m_num_primitives);
	}
}

bool DebugObject::is_valid()
{
	return m_VBO != 0;
}

void DebugObject::set_color(glm::vec3 && color)
{
	m_color = std::move(color);
}

glm::vec3 const & DebugObject::get_color()
{
	return m_color;
}

void DebugObject::draw_constraints(bool value)
{
	m_draw_constraints = value;
}

void DebugObject::set_edge_thiccness(float thiccness)
{
	m_edge_thiccness = thiccness;
}

void DebugObject::set_point_thiccness(float thiccness)
{
	m_point_thiccness = thiccness;
}

void DebugObject::build(CPU::Mesh& mesh)
{
	if (m_VBO > 0)
		glDeleteBuffers(1, &m_VBO);
	if (m_VAO > 0)
		glDeleteBuffers(1, &m_VAO);

	glGenVertexArrays(1, &m_VAO);
	glBindVertexArray(m_VAO);

	std::vector<CPU::VertexRef> const& mesh_verts = mesh.get_vertex_list();
	std::vector<DrawVertex> vertices;

	for (auto& vertex : mesh_verts)
		vertices.push_back({ vertex.vertice, {m_color.r, m_color.g, m_color.b, 1.f} });

	m_num_primitives = (GLuint)vertices.size();

	glGenBuffers(1, &m_VBO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(DrawVertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(2 * sizeof(float)));
	glEnableVertexAttribArray(1);
	
	if (m_mode == DRAW_EDGES)
	{
		std::vector<CPU::Edge> const&  mesh_edges = mesh.get_edge_list();
		std::vector<glm::ivec2> edges_indices;

		if (m_draw_constraints)
		{
			for (auto& edge : mesh_edges)
			{
				if (edge.constraint_ref.size() > 0)
					edges_indices.push_back(edge.edge);
			}
		}
		else
		{
			for (auto& edge : mesh_edges)
				edges_indices.push_back(edge.edge);
		}

		m_num_primitives = (GLuint)edges_indices.size() * 2;

		glGenBuffers(1, &m_EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(glm::ivec2) * edges_indices.size(), edges_indices.data(), GL_STATIC_DRAW);
	}
	if (m_mode == DRAW_FACES)
	{
		std::vector<CPU::Face> const&  mesh_faces = mesh.get_face_list();
		std::vector<glm::ivec3> face_indices;

		for (auto& face : mesh_faces)
			face_indices.push_back(face.vert_i);

		m_num_primitives = (GLuint)face_indices.size() * 3;

		glGenBuffers(1, &m_EBO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(glm::ivec3) * face_indices.size(), face_indices.data(), GL_STATIC_DRAW);
	}
	glBindVertexArray(0);
}
