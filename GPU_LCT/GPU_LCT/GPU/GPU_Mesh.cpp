#include "GPU_Mesh.hpp"

namespace GPU
{
	GPUMesh::GPUMesh()
	{
		setup_compute_shaders();
	}


	GPUMesh::~GPUMesh()
	{
	}

	void GPUMesh::initiate_buffers()
	{
		// Creates a starting rectangle

		// Fill point buffers
		std::vector<glm::vec2> starting_vertices = { {-1.f, 1.f}, {-1.f, -1.f}, {1.f, -1.f}, {1.f, 1.f} };
		int type = GL_SHADER_STORAGE_BUFFER;
		int usage = GL_DYNAMIC_DRAW | GL_DYNAMIC_READ;
		int n = 100000;

		m_point_bufs.positions.create_buffer(type, starting_vertices, usage, 0, n);

		std::vector<GLuint> vert_indices(4, true);
		m_point_bufs.inserted.create_buffer(type, std::vector<int>(4, 1), usage, 1, n);
		m_point_bufs.tri_index.create_buffer(type, std::vector<int>(4, 0), usage, 2, n);
	
		// Fill edge buffers
		m_edge_bufs.label.create_buffer(type, std::vector<int>(5, 0), usage, 3, n);
		std::vector<GLuint> edge_constraints(5, 1);
		edge_constraints[4] = 0;
		m_edge_bufs.is_constrained.create_buffer(type, edge_constraints, usage, 4, n);

		// Fill constraint buffers
		std::vector<glm::ivec2> starting_contraints = { {0, 1}, {1, 2}, {2, 3}, {3, 0} };
		m_segment_bufs.endpoint_indices.create_buffer(type, starting_contraints, usage, 5, n);
		m_segment_bufs.inserted.create_buffer(type, std::vector<int>(4, 1), usage, 6, n);

		// Fill triangle buffers
		std::vector<glm::ivec3> starting_triangle_indices = { {0, 1, 3}, {3, 1, 2} }; // ccw
		m_triangle_bufs.vertex_indices.create_buffer(type, starting_triangle_indices, usage, 7, n);

		std::vector<glm::ivec3> sym_edge_indices;
		sym_edge_indices.push_back({ 0, 1, 2 });
		sym_edge_indices.push_back({ 3, 4 ,5 });
		m_triangle_bufs.symedge_indices.create_buffer(type, sym_edge_indices, usage, 8, n);

		m_triangle_bufs.ins_point_index.create_buffer(type, std::vector<int>(2, -1), usage, 9, n);
		m_triangle_bufs.seg_inters_index.create_buffer(type, std::vector<int>(2, -1), usage, 10, n);
		m_triangle_bufs.edge_flip_index.create_buffer(type, std::vector<int>(2, -1), usage, 11, n);

		// Separate sym edge list
		std::vector<SymEdge> sym_edges;

		// left triangle
		sym_edges.push_back({ 1, -1, 0, 0, 0 });
		sym_edges.push_back({ 2, -1, 1, 4, 0 });
		sym_edges.push_back({ 0,  3, 3, 3, 0 });

		// right triangle
		sym_edges.push_back({ 4, -1, 3, 4, 1 });
		sym_edges.push_back({ 5,  1, 1, 1, 1 });
		sym_edges.push_back({ 3, -1, 2, 2, 1 });

		m_sym_edges.create_buffer(type, sym_edges, usage, 12, n);
	}


	void GPUMesh::setup_compute_shaders()
	{
		// TODO, specify paths
		compile_cs(m_location_program, "location_step.glsl");
		compile_cs(m_insertion_program, "");
		compile_cs(m_marking_program, "");
		compile_cs(m_flip_edges_program, "");
	}

	void GPUMesh::compile_cs(GLuint & program, const char * path)
	{
		GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(shader, 1, &path, NULL);
		glCompileShader(shader);
		// check for compilation errors as per normal here
		int success;
		char infoLog[512];
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

		if (!success)
		{
			glGetShaderInfoLog(shader, 512, NULL, infoLog);
			std::cout << " CS shader compile failed\n" << infoLog << '\n';
		}

		program = glCreateProgram();
		glAttachShader(program, shader);
		glLinkProgram(program);

		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(program, 512, NULL, infoLog);
			std::cout << "CS program linking failed\n" << infoLog << '\n';
		}
	}
}