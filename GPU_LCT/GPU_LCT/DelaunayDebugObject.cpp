#include "DelaunayDebugObject.hpp"

using namespace glm;

void DelaunayDebugObject::set_circle_color(vec3 && color)
{
	m_circle_color = color;
}

vec3 const & DelaunayDebugObject::get_circle_color()
{
	return m_circle_color;
}

void DelaunayDebugObject::set_circle_thiccness(float thiccness)
{
	m_circle_thiccness = thiccness;
}

float DelaunayDebugObject::get_circle_thiccness()
{
	return m_circle_thiccness;
}

DelaunayDebugObject::DelaunayDebugObject()
{
}

DelaunayDebugObject::DelaunayDebugObject(CPU::Mesh& mesh)
{
	build(mesh);
}

DelaunayDebugObject::~DelaunayDebugObject()
{
}

void DelaunayDebugObject::bind_VAO()
{
	m_vertex_buffer.bind_buffer();
	m_circle_buffer.bind_buffer();
}

bool DelaunayDebugObject::is_enabled()
{
	return m_enabled;
}

void DelaunayDebugObject::enable(bool value)
{
	m_enabled = value;
}

void DelaunayDebugObject::build(CPU::Mesh & mesh)
{
	std::vector<CPU::Face> face_list = mesh.get_face_list();
	std::vector<CircleData> circle_data;

	// Calculate circle centroid and radius
	for (auto& face : face_list)
	{
		std::array<vec2, 3> triangle;
		for (int i = 0; i < 3; i++)
			triangle[i] = mesh.get_vertex(face.vert_i[i]);

		CircleData cd;
		cd.center = circle_center_from_points(triangle[0], triangle[1], triangle[2]);
		cd.radius = line_length(triangle[0] - cd.center);

		//cd.radius = 0.001f;
		circle_data.push_back(std::move(cd));
	}

	std::vector<vec2> NDC_vertices =
	{
		{-1.f,  1.f},
		{-1.f, -1.f},
		{ 1.f,  1.f},

		{ 1.f,  1.f},
		{-1.f, -1.f},
		{ 1.f, -1.f}
	};

	m_vertex_buffer.create_buffer(GL_ARRAY_BUFFER, NDC_vertices, GL_STATIC_DRAW);
	m_vertex_buffer.set_vertex_attribute(0, 2, GL_FLOAT, 2 * sizeof(float), 0);
	m_circle_buffer.create_buffer(GL_SHADER_STORAGE_BUFFER, circle_data, GL_STATIC_DRAW, 1);
}
