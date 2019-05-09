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
	// Creates an empty buffer
	m_vertex_input.create_buffer(GL_ARRAY_BUFFER, std::vector<DrawVertex>(), GL_DYNAMIC_DRAW);

	std::vector<glm::ivec2> edges_indices = { {0, 1} };
	m_index_buffer.create_buffer(GL_ELEMENT_ARRAY_BUFFER, edges_indices, GL_STATIC_DRAW);
	update_edge(vertices);
}

void DebugObject::update_edge(std::array<glm::vec2, 2> vertices)
{
	std::vector<DrawVertex> dv;
	dv.push_back({ vertices[0], glm::vec4(m_color.r, m_color.g, m_color.b, 1.f) });
	dv.push_back({ vertices[1], glm::vec4(m_color.r, m_color.g, m_color.b, 0.f) });

	m_vertex_input.update_buffer(dv);
	m_vertex_input.bind_buffer();
	m_vertex_input.set_vertex_attribute(0, 2, GL_FLOAT, 6 * sizeof(float), 0);
	m_vertex_input.set_vertex_attribute(1, 4, GL_FLOAT, 6 * sizeof(float), 2 * sizeof(float));
	m_vertex_input.unbind_buffer();
}


DebugObject::~DebugObject()
{
}

void DebugObject::bind_VAO()
{
	m_vertex_input.bind_buffer();
}

void DebugObject::draw_object()
{
	if (m_mode == DRAW_FACES)
	{
		m_index_buffer.bind_buffer();
		glDrawElements(GL_TRIANGLES, m_index_buffer.element_count() * 3, GL_UNSIGNED_INT, 0);
	}
	if (m_mode == DRAW_EDGES)
	{
		glLineWidth(m_edge_thiccness);
		m_index_buffer.bind_buffer();
		glDrawElements(GL_LINES, m_index_buffer.element_count() * 2, GL_UNSIGNED_INT, 0);
	}
	if (m_mode == DRAW_POINTS)
	{
		glPointSize(m_point_thiccness);
		glDrawArrays(GL_POINTS, 0, m_vertex_input.element_count());
	}
}

bool DebugObject::is_valid()
{
	return m_vertex_input.is_valid();
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

void DebugObject::set_draw_left_side(bool val)
{
	m_draw_left_side = val;
}

bool DebugObject::draw_left_side()
{
	return m_draw_left_side;
}

void DebugObject::build(CPU::Mesh& mesh)
{
	std::vector<CPU::VertexRef> const& mesh_verts = mesh.get_vertex_list();
	std::vector<DrawVertex> vertices;

	for (auto& vertex : mesh_verts)
		vertices.push_back({ vertex.vertice, {m_color.r, m_color.g, m_color.b, 1.f} });

	m_vertex_input.create_buffer(GL_ARRAY_BUFFER, vertices, GL_STATIC_DRAW);
	m_vertex_input.bind_buffer();
	m_vertex_input.set_vertex_attribute(0, 2, GL_FLOAT, 6 * sizeof(float), 0);
	m_vertex_input.set_vertex_attribute(1, 4, GL_FLOAT, 6 * sizeof(float), 2 * sizeof(float));
	m_vertex_input.unbind_buffer();

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

		m_index_buffer.create_buffer(GL_ELEMENT_ARRAY_BUFFER, edges_indices, GL_STATIC_DRAW);
	}
	if (m_mode == DRAW_FACES)
	{
		std::vector<CPU::Face> const&  mesh_faces = mesh.get_face_list();
		std::vector<glm::ivec3> face_indices;

		for (auto& face : mesh_faces)
			face_indices.push_back(face.vert_i);

		m_index_buffer.create_buffer(GL_ELEMENT_ARRAY_BUFFER, face_indices, GL_STATIC_DRAW);
	}
}

void DebugObject::build(GPU::GPUMesh & mesh)
{
	std::vector<glm::vec2> mesh_verts = mesh.get_vertices();
	std::vector<DrawVertex> vertices;

	int i = 0; 
	for (auto& vertex : mesh_verts) 
	{ 
		if (i != -1) 
			vertices.push_back({ vertex, {m_color.r, m_color.g, m_color.b, 1.f} }); 
		else 
			vertices.push_back({ vertex, {1.f, 0.f, 1.f, 1.f} }); 
		i++; 
	} 

	m_vertex_input.create_buffer(GL_ARRAY_BUFFER, vertices, GL_STATIC_DRAW);
	m_vertex_input.bind_buffer();
	m_vertex_input.set_vertex_attribute(0, 2, GL_FLOAT, 6 * sizeof(float), 0);
	m_vertex_input.set_vertex_attribute(1, 4, GL_FLOAT, 6 * sizeof(float), 2 * sizeof(float));
	m_vertex_input.unbind_buffer();

	if (m_mode == DRAW_EDGES)
	{
		std::vector<std::pair<glm::ivec2, bool>> edges = mesh.get_edges();
		std::vector<glm::ivec2> edges_indices;

		if (m_draw_constraints)
		{
			for (auto& edge : edges)
			{
				if (edge.second)
					edges_indices.push_back(edge.first);
			}
		}
		else
		{
			for (auto& edge : edges)
				edges_indices.push_back(edge.first);
		}

		m_index_buffer.create_buffer(GL_ELEMENT_ARRAY_BUFFER, edges_indices, GL_STATIC_DRAW);
	}
	if (m_mode == DRAW_FACES)
	{
		std::vector<glm::ivec3> mesh_faces = mesh.get_faces();
		m_index_buffer.create_buffer(GL_ELEMENT_ARRAY_BUFFER, mesh_faces, GL_STATIC_DRAW);
	}
}

void DebugObject::build(GPU::GCMesh & mesh)
{
	std::vector<glm::vec2> mesh_verts = mesh.get_vertices();
	std::vector<DrawVertex> vertices;

	for (auto& vertex : mesh_verts)
		vertices.push_back({ vertex, {m_color.r, m_color.g, m_color.b, 1.f} });

	m_vertex_input.create_buffer(GL_ARRAY_BUFFER, vertices, GL_STATIC_DRAW);
	m_vertex_input.bind_buffer();
	m_vertex_input.set_vertex_attribute(0, 2, GL_FLOAT, 6 * sizeof(float), 0);
	m_vertex_input.set_vertex_attribute(1, 4, GL_FLOAT, 6 * sizeof(float), 2 * sizeof(float));
	m_vertex_input.unbind_buffer();

	if (m_mode == DRAW_EDGES)
	{
		std::vector<std::pair<glm::ivec2, bool>> edges = mesh.get_edges();
		std::vector<glm::ivec2> edges_indices;

		if (m_draw_constraints)
		{
			for (auto& edge : edges)
			{
				if (edge.second)
					edges_indices.push_back(edge.first);
			}
		}
		else
		{
			for (auto& edge : edges)
				edges_indices.push_back(edge.first);
		}

		m_index_buffer.create_buffer(GL_ELEMENT_ARRAY_BUFFER, edges_indices, GL_STATIC_DRAW);
	}
	if (m_mode == DRAW_FACES)
	{
		std::vector<glm::ivec3> mesh_faces = mesh.get_faces();
		m_index_buffer.create_buffer(GL_ELEMENT_ARRAY_BUFFER, mesh_faces, GL_STATIC_DRAW);
	}
}
