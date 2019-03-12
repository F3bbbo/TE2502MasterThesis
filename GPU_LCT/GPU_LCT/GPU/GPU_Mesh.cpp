#include "GPU_Mesh.hpp"

namespace GPU
{
	GPUMesh::GPUMesh()
	{
	}


	GPUMesh::~GPUMesh()
	{
	}

	void GPUMesh::initiate_buffers()
	{
		// Creates a starting rectangle

		// Fill point buffers
		std::vector<glm::vec2> starting_vertices = { {-1.f, 1.f}, {-1.f, -1.f}, {1.f, -1.f}, {1.f, 1.f} };
		
		m_point_bufs.positions.create_buffer(GL_SHADER_STORAGE_BUFFER, starting_vertices, GL_DYNAMIC_DRAW | GL_DYNAMIC_READ, 0, 10000);

		std::vector<GLuint> vert_indices(4, true);
		m_point_bufs.inserted.create_buffer(GL_SHADER_STORAGE_BUFFER, std::vector<GLuint>(4, 1), GL_DYNAMIC_DRAW | GL_DYNAMIC_READ, 0, 10000);
		m_point_bufs.tri_index.create_buffer(GL_SHADER_STORAGE_BUFFER, std::vector<GLuint>(4, 0), GL_DYNAMIC_DRAW | GL_DYNAMIC_READ, 0, 10000);
	
		// Fill edge buffers
		m_edge_bufs.label.create_buffer(GL_SHADER_STORAGE_BUFFER, std::vector<GLuint>(5, 0), GL_DYNAMIC_DRAW | GL_DYNAMIC_READ, 0, 10000);
		std::vector<GLuint> edge_constraints(5, 1);
		edge_constraints[4] = 0;
		m_edge_bufs.is_constrained.create_buffer(GL_SHADER_STORAGE_BUFFER, edge_constraints, GL_DYNAMIC_DRAW | GL_DYNAMIC_READ, 0, 10000);

		// Fill constraint buffers
		std::vector<glm::ivec2> starting_contraints = { {0, 1}, {1, 2}, {2, 3}, {3, 0} };
		m_segment_bufs.endpoint_indices.create_buffer(GL_SHADER_STORAGE_BUFFER, starting_contraints, GL_DYNAMIC_DRAW | GL_DYNAMIC_READ, 0, 10000);
		m_segment_bufs.inserted.create_buffer(GL_SHADER_STORAGE_BUFFER, std::vector<GLuint>(4, 1), GL_DYNAMIC_DRAW | GL_DYNAMIC_READ, 0, 10000);

		// Fill triangle buffers
		std::vector<glm::ivec3> starting_triangle_indices = { {0, 1, 3}, {3, 1, 2} }; // ccw
		m_triangle_bufs.vertex_indices.create_buffer(GL_SHADER_STORAGE_BUFFER, starting_triangle_indices, GL_DYNAMIC_DRAW | GL_DYNAMIC_READ, 0, 10000);

		std::vector<SymEdge> sym_edges;

		// left triangle
		sym_edges.push_back({ 1, -1, 0, 0, 0 });
		sym_edges.push_back({ 2, -1, 1, 4, 0 });
		sym_edges.push_back({ 0,  3, 3, 3, 0 });

		// right triangle
		sym_edges.push_back({ 4, -1, 3, 4, 1 });
		sym_edges.push_back({ 5,  1, 1, 1, 1 });
		sym_edges.push_back({ 3, -1, 2, 2, 1 });

	}
}