#include "GPU_Mesh.hpp"
#include <fstream>

namespace GPU
{
	GPUMesh::GPUMesh(glm::ivec2 screen_res)
	{
		setup_compute_shaders();
	}


	GPUMesh::~GPUMesh()
	{
	}

	void GPUMesh::initiate_buffers(glm::vec2 scale)
	{
		// Creates a starting rectangle

		// Fill point buffers
		std::vector<glm::vec2> starting_vertices = { {-1.f * scale.x, 1.f * scale.y}, {-1.f * scale.x, -1.f * scale.y}, {1.f * scale.x, -1.f * scale.y}, {1.f * scale.x, 1.f * scale.y} };
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

		std::vector<glm::ivec4> sym_edge_indices;
		sym_edge_indices.push_back({ 0, 1, 2, -1 });
		sym_edge_indices.push_back({ 3, 4 ,5, -1 });
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

		// create uniform buffer to store size of other buffers
		BufferSizes bs;
		bs.num_points = 4;
		bs.num_tris = 2;
		m_sizes.create_uniform_buffer(bs, usage);

		// Probably not needed?

		//m_sizes.bind_buffer();
		//m_sizes.set_unitform_buffer_block(m_location_program, "Sizes");
		//// TODO: uncomment when ready
		////m_sizes.set_unitform_buffer_block(m_insertion_program, "Sizes");
		////m_sizes.set_unitform_buffer_block(m_marking_program, "Sizes");
		////m_sizes.set_unitform_buffer_block(m_flip_edges_program, "Sizes");
		//m_sizes.unbind_buffer();
	}

	void GPUMesh::build_CDT(std::vector<glm::vec2> points, std::vector<glm::ivec2> segments)
	{
		m_point_bufs.positions.append_to_buffer(points);
		m_point_bufs.inserted.append_to_buffer(std::vector<int>(points.size(), 0));
		m_point_bufs.tri_index.append_to_buffer(std::vector<int>(points.size(), 0));

		for (auto& segment : segments)
			segment = segment + glm::ivec2(m_segment_bufs.endpoint_indices.element_count());
		m_segment_bufs.endpoint_indices.append_to_buffer(segments);
		m_segment_bufs.inserted.append_to_buffer(std::vector<int>(points.size(), 0));
		// uppdating ubo containing sizes
		auto buff_size = m_sizes.get_buffer_data<BufferSizes>();
		buff_size.front().num_points += points.size();
		int num_new_tri = points.size() * 2;
		buff_size.front().num_tris += num_new_tri;
		// TODO, fix setting number of triangles: buff_size.front().num_tris 
		m_sizes.bind_buffer();
		m_sizes.update_buffer(buff_size);
		m_sizes.unbind_buffer();

		// fix new sizes of triangle buffers
		m_triangle_bufs.symedge_indices.append_to_buffer(std::vector<glm::ivec4>(num_new_tri, { -1, -1, -1, -1 }));
		m_triangle_bufs.ins_point_index.append_to_buffer(std::vector<int>(num_new_tri, -1));

		// fix new sizes of edge buffers 
		// TODO: fix so it can handle repeated insertions
		int num_new_edges = points.size() * 3;
		m_edge_bufs.is_constrained.append_to_buffer(std::vector<int>(num_new_edges, 0));
		m_edge_bufs.label.append_to_buffer(std::vector<int>(num_new_edges, -1));
		// fix new size of symedges buffer
		// TODO: fix so it can handle repeated insertions
		int num_new_sym_edges = points.size() * 6;
		m_sym_edges.append_to_buffer(std::vector<SymEdge>(num_new_sym_edges));
		// TODO, maybe need to check if triangle buffers needs to grow

		// Bind all ssbo's
		m_point_bufs.positions.bind_buffer();
		m_point_bufs.inserted.bind_buffer();
		m_point_bufs.tri_index.bind_buffer();

		m_edge_bufs.label.bind_buffer();
		m_edge_bufs.is_constrained.bind_buffer();

		m_segment_bufs.endpoint_indices.bind_buffer();
		m_segment_bufs.inserted.bind_buffer();

		m_triangle_bufs.vertex_indices.bind_buffer();
		m_triangle_bufs.symedge_indices.bind_buffer();
		m_triangle_bufs.ins_point_index.bind_buffer();
		m_triangle_bufs.seg_inters_index.bind_buffer();
		m_triangle_bufs.edge_flip_index.bind_buffer();

		m_sym_edges.bind_buffer();

		// Bind all ubo's
		m_sizes.bind_buffer();
		//while (false)
		{
			// Locate Step
			glUseProgram(m_location_program);
			glDispatchCompute((GLuint)256, 1, 1);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);

			glUseProgram(m_location_tri_program);
			glDispatchCompute((GLuint)256, 1, 1);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);




			// Insert Step
			glUseProgram(m_insertion_program);
			glDispatchCompute((GLuint)256, 1, 1);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);

			// Retrieve GPU arrays for debugging
			auto data_points = m_point_bufs.positions.get_buffer_data<glm::vec2>();
			auto data_inserted = m_point_bufs.inserted.get_buffer_data<int>();
			auto data_tri_index = m_point_bufs.tri_index.get_buffer_data<int>();
			auto data_symedges = m_sym_edges.get_buffer_data<SymEdge>();
			auto data_triangles = m_triangle_bufs.symedge_indices.get_buffer_data<glm::ivec4>();
			auto data_tri_point_index = m_triangle_bufs.ins_point_index.get_buffer_data<int>();
			auto data_edge_label = m_edge_bufs.label.get_buffer_data<int>();
			auto data_size = m_sizes.get_buffer_data<BufferSizes>();

			// Marking Step
			//glUseProgram(m_marking_program);
			//glDispatchCompute((GLuint)number, 1, 1);

			// Fliping Step
			//glUseProgram(m_flip_edges_program);
			//glDispatchCompute((GLuint)number, 1, 1);
		}
	}

	std::vector<glm::vec2> GPUMesh::get_vertices()
	{
		return m_point_bufs.positions.get_buffer_data<glm::vec2>();
	}

	glm::vec2 GPUMesh::get_vertex(int index)
	{
		std::vector<glm::vec2> vertices = m_point_bufs.positions.get_buffer_data<glm::vec2>(index, 1);
		return vertices[0];
	}

	SymEdge GPUMesh::get_symedge(int index)
	{
		std::vector<SymEdge> sym_edge_list = m_sym_edges.get_buffer_data<SymEdge>(index, 1);
		return sym_edge_list[0];
	}

	std::vector<std::pair<glm::ivec2, bool>> GPUMesh::get_edges()
	{
		std::vector<SymEdge> sym_edge_list = m_sym_edges.get_buffer_data<SymEdge>();
		std::vector<int> is_constrained_edge_list = m_edge_bufs.is_constrained.get_buffer_data<int>();

		std::vector<std::pair<glm::ivec2, bool>> edge_list;
		std::vector<glm::ivec2> found_edges;

		for (SymEdge& symedge : sym_edge_list)
		{
			glm::ivec2 edge = { symedge.vertex, sym_edge_list[symedge.nxt].vertex };
			if (std::find(found_edges.begin(), found_edges.end(), edge) == found_edges.end())
			{
				found_edges.push_back(edge);
				if (is_constrained_edge_list[symedge.edge])
					edge_list.push_back({ edge, true });
				else
					edge_list.push_back({ edge, false });
			}
		}
		return edge_list;
	}

	std::vector<glm::ivec3> GPUMesh::get_faces()
	{
		return m_triangle_bufs.vertex_indices.get_buffer_data<glm::ivec3>();
	}

	int GPUMesh::locate_face(glm::vec2 p)
	{
		p = p - glm::vec2(2.f, 0.f);
		std::vector<glm::ivec3> triangle_indices = m_triangle_bufs.vertex_indices.get_buffer_data<glm::ivec3>();
		std::vector<glm::vec2> vertices = m_point_bufs.positions.get_buffer_data<glm::vec2>();

		for (int i = 0; i < triangle_indices.size(); i++)
		{
			if (point_triangle_test(p, vertices[triangle_indices[i].x], vertices[triangle_indices[i].y], vertices[triangle_indices[i].z]))
				return i;
		}
		return -1;
	}

	void GPUMesh::setup_compute_shaders()
	{
		// TODO, specify paths
		compile_cs(m_location_program, "GPU/location_step.glsl");
		compile_cs(m_location_tri_program, "GPU/location_step_triangle.glsl");
		compile_cs(m_insertion_program, "GPU/insertion_step.glsl");
		compile_cs(m_marking_part_one_program, "GPU/marking_step_part_one.glsl");
		compile_cs(m_marking_part_two_program, "GPU/marking_step_part_two.glsl");
		compile_cs(m_flip_edges_part_one_program, "GPU/flipping_part_one.glsl");
		//compile_cs(m_flip_edges_part_two_program, "GPU/.glsl");
		//compile_cs(m_flip_edges_part_three_program, "GPU/.glsl");
	}

	void GPUMesh::compile_cs(GLuint & program, const char * path)
	{
		if (path == "")
			return;
		std::ifstream shader_file;
		std::string str;
		shader_file.open(path);
		while (!shader_file.eof())
		{
			std::string tmp;
			getline(shader_file, tmp);
			str += tmp + '\n';
		}
		shader_file.close();
		const char* c = str.c_str();

		GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(shader, 1, &c, NULL);
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
