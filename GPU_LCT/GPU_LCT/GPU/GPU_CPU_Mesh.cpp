#include "GPU_CPU_Mesh.hpp"
#include <fstream>

namespace GPU
{
	GCMesh::GCMesh(glm::ivec2 screen_res)
	{
		setup_compute_shaders();
	}


	GCMesh::~GCMesh()
	{
	}

	void GCMesh::initiate_buffers(glm::vec2 scale)
	{
		// Creates a starting rectangle

		// Fill point buffers
		point_positions = { {-1.f * scale.x, 1.f * scale.y}, {-1.f * scale.x, -1.f * scale.y}, {1.f * scale.x, -1.f * scale.y}, {1.f * scale.x, 1.f * scale.y} };
		//int type = GL_SHADER_STORAGE_BUFFER;
		//int usage = GL_DYNAMIC_DRAW | GL_DYNAMIC_READ;
		//int n = 100000;

		//point_positions.create_buffer(type, starting_vertices, usage, 0, n);

		//std::vector<GLuint> vert_indices(4, true);
		point_inserted = std::vector<int>(4, 1);
		//point_inserted.create_buffer(type, std::vector<int>(4, 1), usage, 1, n);
		//point_tri_index.create_buffer(type, std::vector<int>(4, 0), usage, 2, n);
		point_tri_index = std::vector<int>(4, 0);
		// Fill edge buffers
		//edge_label.create_buffer(type, std::vector<int>(5, 0), usage, 3, n);
		edge_label = std::vector<int>(5, 0);
		std::vector<int> edge_constraints(5, 1);
		edge_constraints[0] = 0;
		edge_constraints[1] = 1;
		edge_constraints[2] = 2;
		edge_constraints[3] = 3;
		edge_constraints[4] = -1;
		edge_is_constrained = edge_constraints;
		//edge_is_constrained.create_buffer(type, edge_constraints, usage, 4, n);

		// Fill constraint buffers
		seg_endpoint_indices = { {0, 1}, {1, 2}, {2, 3}, {3, 0} };
		seg_inserted = std::vector<int>(4, 1);
		//seg_endpoint_indices.create_buffer(type, starting_contraints, usage, 5, n);
		//seg_inserted.create_buffer(type, std::vector<int>(4, 1), usage, 6, n);

		//std::vector<glm::ivec4> sym_edge_indices;
		tri_symedges.push_back({ 0, 1, 2, -1 });
		tri_symedges.push_back({ 3, 4 ,5, -1 });
		//tri_symedges.create_buffer(type, sym_edge_indices, usage, 7, n);

		//tri_ins_point_index.create_buffer(type, std::vector<int>(2, -1), usage, 8, n);
		tri_ins_point_index = std::vector<int>(2, -1);
		//tri_seg_inters_index.create_buffer(type, std::vector<int>(2, -1), usage, 9, n);
		tri_seg_inters_index = std::vector<int>(2, -1);
		//tri_edge_flip_index.create_buffer(type, std::vector<int>(2, -1), usage, 10, n);
		tri_edge_flip_index = std::vector<int>(2, -1);
		//tri_insert_points.create_buffer(type, std::vector<NewPoint>(2), usage, 13, n);
		tri_insert_points = std::vector<NewPoint>(2);

		// Separate sym edge list

		// left triangle
		m_sym_edges.push_back({ 1, -1, 0, 0, 0 });
		m_sym_edges.push_back({ 2, -1, 1, 4, 0 });
		m_sym_edges.push_back({ 0,  3, 3, 3, 0 });

		// right triangle
		m_sym_edges.push_back({ 4, -1, 3, 4, 1 });
		m_sym_edges.push_back({ 5,  1, 1, 1, 1 });
		m_sym_edges.push_back({ 3, -1, 2, 2, 1 });


		//m_nr_of_symedges.create_uniform_buffer<int>({ m_sym_edges.element_count() }, usage);
		m_nr_of_symedges = m_sym_edges.size();
		m_status = 1;
		//m_status.create_buffer(type, std::vector<int>(1, 1), GL_DYNAMIC_DRAW, 12, 1);
	}

	void GCMesh::build_CDT(std::vector<glm::vec2> points, std::vector<glm::ivec2> segments)
	{
		append_vec(point_positions, points);
		append_vec(point_inserted, std::vector<int>(points.size(), 0));
		append_vec(point_tri_index, std::vector<int>(points.size(), 0));

		for (auto& segment : segments)
			segment = segment + glm::ivec2(seg_endpoint_indices.size());
		append_vec(seg_endpoint_indices, segments);
		append_vec(seg_inserted, std::vector<int>(segments.size(), 0));
		// uppdating ubo containing sizes

		int num_new_tri = points.size() * 2;
		int num_new_segs = segments.size();

		// fix new sizes of triangle buffers
		append_vec(tri_symedges, std::vector<glm::ivec4>(num_new_tri, { -1, -1, -1, -1 }));
		append_vec(tri_ins_point_index, std::vector<int>(num_new_tri, -1));
		append_vec(tri_edge_flip_index, std::vector<int>(num_new_tri, -1));
		append_vec(tri_seg_inters_index, std::vector<int>(num_new_tri, -1));
		append_vec(tri_insert_points, std::vector<NewPoint>(num_new_tri));

		// fix new sizes of edge buffers 
		// TODO: fix so it can handle repeated insertions
		int num_new_edges = points.size() * 3;
		append_vec(edge_is_constrained, std::vector<int>(num_new_edges, -1));
		append_vec(edge_label, std::vector<int>(num_new_edges, -1));
		// fix new size of symedges buffer
		// TODO: fix so it can handle repeated insertions
		int num_new_sym_edges = points.size() * 6;
		append_vec(m_sym_edges, std::vector<SymEdge>(num_new_sym_edges));
		// TODO, maybe need to check if triangle buffers needs to grow

		//m_nr_of_symedges.update_buffer<int>({ m_sym_edges.element_count() });
		m_nr_of_symedges = m_sym_edges.size();
		// Bind all ssbo's
		//point_positions.bind_buffer();
		//point_inserted.bind_buffer();
		//point_tri_index.bind_buffer();

		//edge_label.bind_buffer();
		//edge_is_constrained.bind_buffer();

		//seg_endpoint_indices.bind_buffer();
		//seg_inserted.bind_buffer();

		//tri_symedges.bind_buffer();
		//tri_ins_point_index.bind_buffer();
		//tri_seg_inters_index.bind_buffer();
		//tri_edge_flip_index.bind_buffer();
		//tri_insert_points.bind_buffer();

		//m_sym_edges.bind_buffer();
		//m_nr_of_symedges.bind_buffer();

		//m_status.bind_buffer();

		int counter = 0;

		Timer timer;
		timer.start();

		int cont = m_status;
		while (cont)
		{
			counter++;
			m_status = 0;
			//m_status.update_buffer<int>({ 0 });

			//// Locate Step
			location_program();
			//glUseProgram(m_location_program);
			//glDispatchCompute((GLuint)256, 1, 1);
			//glMemoryBarrier(GL_ALL_BARRIER_BITS);

			location_tri_program();
			//glUseProgram(m_location_tri_program);
			//glDispatchCompute((GLuint)256, 1, 1);
			//glMemoryBarrier(GL_ALL_BARRIER_BITS);

			//// Insert Step
			insertion_program();
			//glUseProgram(m_insertion_program);
			//glDispatchCompute((GLuint)256, 1, 1);
			//glMemoryBarrier(GL_ALL_BARRIER_BITS);



			//// Marking Step
			marking_part_one_program();
			//glUseProgram(m_marking_part_one_program);
			//glDispatchCompute((GLuint)256, 1, 1);
			//glMemoryBarrier(GL_ALL_BARRIER_BITS);
			if (counter == 3)
				break;
			marking_part_two_program();
			//glUseProgram(m_marking_part_two_program);
			//glDispatchCompute((GLuint)256, 1, 1);
			//glMemoryBarrier(GL_ALL_BARRIER_BITS);

			//// Flipping Step
			flip_edges_part_one_program();
			//glUseProgram(m_flip_edges_part_one_program);
			//glDispatchCompute((GLuint)256, 1, 1);
			//glMemoryBarrier(GL_ALL_BARRIER_BITS);
			flip_edges_part_two_program();
			//glUseProgram(m_flip_edges_part_two_program);
			//glDispatchCompute((GLuint)256, 1, 1);
			//glMemoryBarrier(GL_ALL_BARRIER_BITS);
			flip_edges_part_three_program();
			//glUseProgram(m_flip_edges_part_three_program);
			//glDispatchCompute((GLuint)256, 1, 1);
			//glMemoryBarrier(GL_ALL_BARRIER_BITS);


			cont = m_status;

		}
		// TODO: remove this creation of lct
		//refine_LCT();
		// points
		timer.stop();

		LOG(std::string("Number of iterations: ") + std::to_string(counter));
		LOG(std::string("Elapsed time in ms: ") + std::to_string(timer.elapsed_time()));

		/*glUseProgram(m_insert_in_edge_program);
		glDispatchCompute((GLuint)256, 1, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);*/

		//auto point_data_pos = point_positions;
		//auto point_data_inserted = point_inserted;
		//auto point_data_triangle_index = point_tri_index;

		//// symedges
		//auto symedges = m_sym_edges;

		//// edges
		//auto edge_data_labels = edge_label;
		//auto edge_data_is_constrained = edge_is_constrained;

		//// segments
		//auto segment_data_inserted = seg_inserted;
		//auto segment_data_endpoint_indices = seg_endpoint_indices.get_buffer_data<glm::ivec2>();

		//// triangles
		//auto triangle_data_symedge_indices = tri_symedges;
		//auto triangle_data_insert_point_index = tri_ins_point_index;
		//auto triangle_data_edge_flip_index = tri_edge_flip_index;
		//auto triangle_data_intersecting_segment = tri_seg_inters_index;
		//auto triangle_data_new_points = tri_insert_points.get_buffer_data<NewPoint>();

		auto status_data = m_status;
	}

	void GCMesh::refine_LCT()
	{
		int i = 0;
		int num_new_points;
		do
		{
			// Locate disturbances
			locate_disturbances_program();
			//glUseProgram(m_locate_disturbances_program);
			//glDispatchCompute((GLuint)256, 1, 1);
			//glMemoryBarrier(GL_ALL_BARRIER_BITS);
			// Check how many new points are going to get inserted
			num_new_points = m_status;
			if (num_new_points > 0)
			{
				// increase sizes of arrays, 
				// based on how many new points are inserted
				append_vec(point_positions, std::vector<glm::vec2>(num_new_points));
				append_vec(point_inserted, std::vector<int>(num_new_points));
				append_vec(point_tri_index, std::vector<int>(num_new_points));
				// segments
				int num_new_tri = num_new_points * 2;
				int num_new_segs = num_new_points;

				// fix new sizes of triangle buffers
				append_vec(tri_symedges, std::vector<glm::ivec4>(num_new_tri, { -1, -1, -1, -1 }));
				append_vec(tri_ins_point_index, std::vector<int>(num_new_tri, -1));
				append_vec(tri_edge_flip_index, std::vector<int>(num_new_tri, -1));
				append_vec(tri_seg_inters_index, std::vector<int>(num_new_tri, -1));
				append_vec(tri_insert_points, std::vector<NewPoint>(num_new_tri));

				// fix new size of segment buffers
				append_vec(seg_endpoint_indices, std::vector<glm::ivec2>(num_new_segs));
				append_vec(seg_inserted, std::vector<int>(num_new_segs));
				// fix new sizes of edge buffers 
				// TODO: fix so it can handle repeated insertions
				int num_new_edges = num_new_points * 3;
				append_vec(edge_is_constrained, std::vector<int>(num_new_edges, -1));
				append_vec(edge_label, std::vector<int>(num_new_edges, -1));
				// fix new size of symedges buffer
				// TODO: fix so it can handle repeated insertions
				int num_new_sym_edges = num_new_points * 6;
				//m_sym_edges, );
				append_vec(m_sym_edges, std::vector<SymEdge>(num_new_sym_edges));
				// TODO, maybe need to check if triangle buffers needs to grow
				m_nr_of_symedges = m_sym_edges.size();

				//m_nr_of_symedges.update_buffer<int>({ m_sym_edges.element_count() });

				// then rebind the buffers that has been changed
	/*			point_positions.bind_buffer();
				point_inserted.bind_buffer();
				point_tri_index.bind_buffer();

				edge_label.bind_buffer();
				edge_is_constrained.bind_buffer();

				seg_endpoint_indices.bind_buffer();
				seg_inserted.bind_buffer();

				tri_symedges.bind_buffer();
				tri_ins_point_index.bind_buffer();
				tri_seg_inters_index.bind_buffer();
				tri_edge_flip_index.bind_buffer();
				tri_insert_points.bind_buffer();

				m_sym_edges.bind_buffer();
				m_nr_of_symedges.bind_buffer();*/

				// add new points to the point buffers
				add_new_points_program();
				//glUseProgram(m_add_new_points_program);
				//glDispatchCompute((GLuint)256, 1, 1);
				//glMemoryBarrier(GL_ALL_BARRIER_BITS);
				// Perform insertion of points untill all has been inserted 
				// and triangulation is CDT
				int counter = 0;
				int cont = 1;
				do
				{
					//m_status.update_buffer<int>({ 0 });
					m_status = 0;
					counter++;
					//// Find out which triangle the point is on the edge of
					locate_point_triangle_program();
					//glUseProgram(m_locate_point_triangle_program);
					//glDispatchCompute((GLuint)256, 1, 1);
					//glMemoryBarrier(GL_ALL_BARRIER_BITS);

					validate_edges_program();
					//glUseProgram(m_validate_edges_program);
					//glDispatchCompute((GLuint)256, 1, 1);
					//glMemoryBarrier(GL_ALL_BARRIER_BITS);

					//// Insert point into the edge
					insert_in_edge_program();
					//glUseProgram(m_insert_in_edge_program);
					//glDispatchCompute((GLuint)256, 1, 1);
					//glMemoryBarrier(GL_ALL_BARRIER_BITS);

					//// Perform marking
					marking_part_two_program();
					//glUseProgram(m_marking_part_two_program);
					//glDispatchCompute((GLuint)256, 1, 1);
					//glMemoryBarrier(GL_ALL_BARRIER_BITS);

					//// Perform flipping to ensure that mesh is CDT
					flip_edges_part_one_program();
					//glUseProgram(m_flip_edges_part_one_program);
					//glDispatchCompute((GLuint)256, 1, 1);
					//glMemoryBarrier(GL_ALL_BARRIER_BITS);
					flip_edges_part_two_program();
					//glUseProgram(m_flip_edges_part_two_program);
					//glDispatchCompute((GLuint)256, 1, 1);
					//glMemoryBarrier(GL_ALL_BARRIER_BITS);
					flip_edges_part_three_program();
					//glUseProgram(m_flip_edges_part_three_program);
					//glDispatchCompute((GLuint)256, 1, 1);
					//glMemoryBarrier(GL_ALL_BARRIER_BITS);
					//cont = m_status[0];
				} while (m_status == 1);
				LOG(std::string("LCT Number of iterations: ") + std::to_string(counter));
			}
			else
			{
				break;
			}
		} while (false);
	}

	std::vector<glm::vec2> GCMesh::get_vertices()
	{
		return point_positions;
	}

	glm::vec2 GCMesh::get_vertex(int index)
	{
		return point_positions[index];
	}

	SymEdge GCMesh::get_symedge(int index)
	{
		return m_sym_edges[index];
	}

	std::vector<std::pair<glm::ivec2, bool>> GCMesh::get_edges()
	{
		std::vector<SymEdge> sym_edge_list = m_sym_edges;
		std::vector<int> is_constrained_edge_list = edge_is_constrained;

		std::vector<std::pair<glm::ivec2, bool>> edge_list;
		std::vector<glm::ivec2> found_edges;

		for (SymEdge& symedge : sym_edge_list)
		{
			if (symedge.nxt == -1)
				continue;
			glm::ivec2 edge = { symedge.vertex, sym_edge_list[symedge.nxt].vertex };
			if (std::find(found_edges.begin(), found_edges.end(), edge) == found_edges.end())
			{
				found_edges.push_back(edge);
				if (is_constrained_edge_list[symedge.edge] != -1)
					edge_list.push_back({ edge, true });
				else
					edge_list.push_back({ edge, false });
			}
		}

		return edge_list;
	}

	std::vector<glm::ivec3> GCMesh::get_faces()
	{
		std::vector<SymEdge> sym_edge_list = m_sym_edges;
		std::vector<glm::ivec4> sym_edge_tri_indices = tri_symedges;

		std::vector<glm::ivec3> face_indices;
		for (int i = 0; i < sym_edge_tri_indices.size(); i++)
		{
			glm::ivec3 s_face_i = { sym_edge_tri_indices[i].x, sym_edge_tri_indices[i].y, sym_edge_tri_indices[i].z };
			if (s_face_i.x == -1)
				continue;
			face_indices.emplace_back(sym_edge_list[s_face_i.x].vertex, sym_edge_list[s_face_i.y].vertex, sym_edge_list[s_face_i.z].vertex);
		}

		return face_indices;
	}

	int GCMesh::locate_face(glm::vec2 p)
	{
		p = p - glm::vec2(2.f, 0.f);

		std::vector<SymEdge> sym_edge_list = m_sym_edges;
		std::vector<glm::ivec4> sym_edge_tri_indices = tri_symedges;
		std::vector<glm::vec2> vertex_list = point_positions;

		int ret_val = -1;

		for (int i = 0; i < sym_edge_tri_indices.size(); i++)
		{
			glm::ivec3 s_face_i = { sym_edge_tri_indices[i].x, sym_edge_tri_indices[i].y, sym_edge_tri_indices[i].z };
			std::array<glm::vec2, 3> vertices;
			if (s_face_i.x == -1)
				continue;
			vertices[0] = vertex_list[sym_edge_list[s_face_i.x].vertex];
			vertices[1] = vertex_list[sym_edge_list[s_face_i.y].vertex];
			vertices[2] = vertex_list[sym_edge_list[s_face_i.z].vertex];

			if (point_triangle_test(p, vertices[0], vertices[1], vertices[2]))
			{
				ret_val = i;
				break;
			}
		}

		return ret_val;
	}

	void GCMesh::setup_compute_shaders()
	{
		// CDT
		//compile_cs(m_location_program, "GPU/location_step.glsl");
		//compile_cs(m_location_tri_program, "GPU/location_step_triangle.glsl");
		//compile_cs(m_insertion_program, "GPU/insertion_step.glsl");
		//compile_cs(m_marking_part_one_program, "GPU/marking_step_part_one.glsl");
		//compile_cs(m_marking_part_two_program, "GPU/marking_step_part_two.glsl");
		//compile_cs(m_flip_edges_part_one_program, "GPU/flipping_part_one.glsl");
		//compile_cs(m_flip_edges_part_two_program, "GPU/flipping_part_two.glsl");
		//compile_cs(m_flip_edges_part_three_program, "GPU/flipping_part_three.glsl");

		//// LCT
		//compile_cs(m_locate_disturbances_program, "GPU/locate_disturbances.glsl");
		//compile_cs(m_add_new_points_program, "GPU/add_new_points.glsl");
		//compile_cs(m_locate_point_triangle_program, "GPU/locate_point_triangle.glsl");
		//compile_cs(m_validate_edges_program, "GPU/validate_edges.glsl");
		//compile_cs(m_insert_in_edge_program, "GPU/insert_in_edge.glsl");
	}

	void GCMesh::compile_cs(GLuint & program, const char * path, int work_group_size)
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
			// Check if line contains layout
			if (tmp.find("layout") != std::string::npos)
			{
				if (tmp.find("local_size_x") != std::string::npos)
				{
					str += "layout(local_size_x = " + std::to_string(work_group_size) + ", local_size_y = 1) in;\n";
					continue;
				}
			}
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
	void GCMesh::location_program()
	{
		for (int index = 0; index < point_positions.size(); index++)
		{

		}
	}
	void GCMesh::location_tri_program()
	{
	}
	void GCMesh::insertion_program()
	{
	}
	void GCMesh::marking_part_one_program()
	{
	}
	void GCMesh::marking_part_two_program()
	{
	}
	void GCMesh::flip_edges_part_one_program()
	{
	}
	void GCMesh::flip_edges_part_two_program()
	{
	}
	void GCMesh::flip_edges_part_three_program()
	{
	}
	void GCMesh::locate_disturbances_program()
	{
	}
	void GCMesh::add_new_points_program()
	{
	}
	void GCMesh::insert_in_edge_program()
	{
	}
	void GCMesh::locate_point_triangle_program()
	{
	}
	void GCMesh::validate_edges_program()
	{
	}
}
