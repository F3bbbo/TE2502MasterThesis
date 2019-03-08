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
	glBindVertexArray(m_VAO);
	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_ssbo);
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

	std::array<vec2, 6> NDC_vertices =
	{
		vec2(-1.f,  1.f),
		vec2(-1.f, -1.f),
		vec2( 1.f,  1.f),

		vec2( 1.f,  1.f),
		vec2(-1.f, -1.f),
		vec2( 1.f, -1.f)
	};

	//circle_data[0].radius = 0.5f;
	glGenVertexArrays(1, &m_VAO);
	glGenBuffers(1, &m_VBO);

	glBindVertexArray(m_VAO);

	glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vec2) * NDC_vertices.size(), NDC_vertices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	// Setup the circle data buffer
	glGenBuffers(1, &m_ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, m_ssbo);
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(CircleData) * circle_data.size(), circle_data.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);

	glBindVertexArray(0);
}
