#include "GPU_Mesh.hpp"
#include <glm/glm.hpp>

namespace GPU
{
	GPUMesh::GPUMesh()
	{
	}


	GPUMesh::~GPUMesh()
	{
	}

	void GPUMesh::initiate_buffers(std::vector<glm::vec2> vertices, std::vector<glm::ivec2> constraints_indices, std::vector<glm::ivec3> triangle_indices)
	{
		std::vector<glm::vec2> starting_vertices = { {-1.f, 1.f}, {-1.f, -1.f}, {1.f, -1.f}, {1.f, 1.f} };
		for (int i = 0; i < 4; i++)
			vertices.insert(vertices.begin(), starting_vertices[i]);
		
		int num_vertices = vertices.size();
		m_point_bufs.positions.create_buffer(GL_SHADER_STORAGE_BUFFER, vertices, GL_DYNAMIC_DRAW);

		std::vector<GLuint> default_values(num_vertices, false);
		default_values[0] = default_values[1] = default_values[2] = default_values[3] = true;
		m_point_bufs.inserted.create_buffer(GL_SHADER_STORAGE_BUFFER, default_values, GL_DYNAMIC_DRAW);

		default_values[0] = default_values[1] = default_values[2] = default_values[3] = false;
		m_point_bufs.inserted.create_buffer(GL_SHADER_STORAGE_BUFFER, default_values, GL_DYNAMIC_DRAW);
	}
}