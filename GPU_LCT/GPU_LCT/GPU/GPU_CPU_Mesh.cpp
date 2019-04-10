#include "GPU_CPU_Mesh.hpp"
#include <fstream>
#include "../trig_functions.hpp"

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
		sym_edges.push_back({ 1, -1, 0, 0, 0 });
		sym_edges.push_back({ 2, -1, 1, 4, 0 });
		sym_edges.push_back({ 0,  3, 3, 3, 0 });

		// right triangle
		sym_edges.push_back({ 4, -1, 3, 4, 1 });
		sym_edges.push_back({ 5,  1, 1, 1, 1 });
		sym_edges.push_back({ 3, -1, 2, 2, 1 });


		//m_nr_of_symedges.create_uniform_buffer<int>({ m_sym_edges.element_count() }, usage);
		symedge_buffer_size = sym_edges.size();
		status = 1;
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
		append_vec(sym_edges, std::vector<SymEdge>(num_new_sym_edges));
		// TODO, maybe need to check if triangle buffers needs to grow

		//m_nr_of_symedges.update_buffer<int>({ m_sym_edges.element_count() });
		symedge_buffer_size = sym_edges.size();
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

		int cont = status;
		while (cont)
		{
			counter++;
			status = 0;
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


			cont = status;

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

		auto status_data = status;
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
			num_new_points = status;
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
				append_vec(sym_edges, std::vector<SymEdge>(num_new_sym_edges));
				// TODO, maybe need to check if triangle buffers needs to grow
				symedge_buffer_size = sym_edges.size();

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
					status = 0;
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
				} while (status == 1);
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
		return sym_edges[index];
	}

	std::vector<std::pair<glm::ivec2, bool>> GCMesh::get_edges()
	{
		std::vector<SymEdge> sym_edge_list = sym_edges;
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
		std::vector<SymEdge> sym_edge_list = sym_edges;
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

		std::vector<SymEdge> sym_edge_list = sym_edges;
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
	//-----------------------------------------------------------
	// Symedge functions
	//-----------------------------------------------------------
	int GCMesh::nxt(int edge)
	{
		return sym_edges[edge].nxt;
	}

	int GCMesh::rot(int edge)
	{
		return sym_edges[edge].rot;
	}

	int GCMesh::sym(int edge)
	{
		return rot(nxt(edge));
	}

	int GCMesh::prev(int edge)
	{
		return nxt(nxt(edge));
	}

	int GCMesh::crot(int edge)
	{
		int sym_i = sym(edge);
		return (sym_i != -1) ? nxt(sym_i) : -1;
	}

	void GCMesh::location_program()
	{
		for (int index = 0; index < point_positions.size(); index++)
		{
			if (point_inserted[index] == 0)
			{
				bool on_edge;
				vec2 tri_cent;
				// find out which triangle the point is now
				int curr_e = tri_symedges[point_tri_index[index]].x;;
				oriented_walk(
					curr_e,
					index,
					on_edge,
					tri_cent);
				if (on_edge)
				{
					int sym_e = sym(curr_e);
					if (sym_e > -1)
					{
						// if neighbour triangle has a lower index 
						// chose that one as the triangle for the point.
						if (sym_edges[sym_e].face < sym_edges[curr_e].face)
						{
							curr_e = sym_e;
						}
					}
				}
				point_tri_index[index] = sym_edges[curr_e].face;
				//			if(on_edge)
				//			{
				//				edge_label[sym_edges[curr_e].edge] = 3; // Priority 3 because point on the edge.	
				//			}		
			}
		}
	}
	void GCMesh::location_tri_program()
	{
		for (int index = 0; index < tri_seg_inters_index.size(); index++)
		{
			if (tri_symedges[index].x != -1)
			{
				std::array<vec2, 3> tri_points;
				get_face(index, tri_points);
				// calculate the centroid of the triangle
				vec2 tri_cent = (tri_points[0] + tri_points[1] + tri_points[2]) / 3.0f;
				int point_index = -1;
				float best_dist = FLT_MAX;
				// Figure out which point should be the new point of this triangle
				for (int i = 0; i < point_positions.size(); i++)
				{
					if (point_tri_index[i] == index)
					{
						// Check so it is an uninserted point.
						if (point_inserted[i] == 0)
						{
							vec2 pos = point_positions[i];
							float dist = distance(pos, tri_cent);
							if (dist < best_dist)
							{
								best_dist = dist;
								point_index = i;
							}
						}
					}
				}
				tri_ins_point_index[index] = point_index;
			}
		}
	}
	void GCMesh::insertion_program()
	{
		for (int index = 0; index < tri_seg_inters_index.size(); index++)
		{
			// If triangle has a point assigned to it add the point to it
			int point_index = tri_ins_point_index[index];
			if (point_index > -1)
			{
				status = 1;

				// Create array of the indices of the three new triangles
				int tri_indices[3];
				tri_indices[0] = index;
				tri_indices[1] = tri_seg_inters_index.size() - 2 * (point_positions.size() - point_index);
				tri_indices[2] = tri_seg_inters_index.size() - 2 * (point_positions.size() - point_index) + 1;
				int edge_indices[3];
				edge_indices[0] = edge_label.size() - 3 * (point_positions.size() - point_index);
				edge_indices[1] = edge_label.size() - 3 * (point_positions.size() - point_index) + 1;
				edge_indices[2] = edge_label.size() - 3 * (point_positions.size() - point_index) + 2;
				int sym_edge_indices[6];
				sym_edge_indices[0] = symedge_buffer_size - 6 * (point_positions.size() - point_index);
				sym_edge_indices[1] = symedge_buffer_size - 6 * (point_positions.size() - point_index) + 1;
				sym_edge_indices[2] = symedge_buffer_size - 6 * (point_positions.size() - point_index) + 2;
				sym_edge_indices[3] = symedge_buffer_size - 6 * (point_positions.size() - point_index) + 3;
				sym_edge_indices[4] = symedge_buffer_size - 6 * (point_positions.size() - point_index) + 4;
				sym_edge_indices[5] = symedge_buffer_size - 6 * (point_positions.size() - point_index) + 5;

				// start working on the new triangles
				int orig_face[3];
				int orig_sym[3];
				// save edges of original triangle
				int curr_e = tri_symedges[index].x;
				for (int i = 0; i < 3; i++)
				{
					orig_face[i] = curr_e;
					int sym = curr_e;
					// sym operations
					nxt(sym);
					rot(sym);
					orig_sym[i] = sym;
					// move curr_e to next edge of triangle
					nxt(curr_e);
				}
				int insert_point = tri_ins_point_index[index];
				// Create symedge structure of the new triangles
				for (int i = 0; i < 3; i++)
				{
					ivec4 tri_syms;
					int next_id = (i + 1) % 3;
					tri_syms.x = orig_face[i];
					tri_syms.y = sym_edge_indices[2 * i];
					tri_syms.z = sym_edge_indices[2 * i + 1];
					tri_syms.w = -1;
					// fix the first symedge of the triangle
					sym_edges[tri_syms.y].vertex = sym_edges[orig_face[next_id]].vertex;
					sym_edges[tri_syms.y].nxt = tri_syms.z;
					// fix the second symedge of the triangle
					sym_edges[tri_syms.z].vertex = insert_point;
					sym_edges[tri_syms.z].nxt = tri_syms.x;

					sym_edges[tri_syms.x].nxt = tri_syms.y;
					//sym_edges[tri_syms.x].nxt = 1;
					// add face index to symedges in this face
					sym_edges[tri_syms.x].face = tri_indices[i];
					sym_edges[tri_syms.y].face = tri_indices[i];
					sym_edges[tri_syms.z].face = tri_indices[i];

					// add symedges to current face
					tri_symedges[tri_indices[i]] = tri_syms;
				}
				// connect the new triangles together
				for (int i = 0; i < 3; i++)
				{
					curr_e = orig_face[i];
					int next_id = (i + 1) % 3;
					int new_edge = edge_indices[i];
					// get both symedges of one new inner edge
					int inner_edge = orig_face[i];
					nxt(inner_edge);
					int inner_edge_sym = orig_face[next_id];
					nxt(inner_edge_sym);
					nxt(inner_edge_sym);
					// set same edge index to both symedges
					sym_edges[inner_edge].edge = new_edge;
					sym_edges[inner_edge_sym].edge = new_edge;
					// connect the edges syms together
					int rot_connect_edge = inner_edge;
					nxt(rot_connect_edge);
					sym_edges[rot_connect_edge].rot = inner_edge_sym;
					int rot_connect_edge_sym = inner_edge_sym;
					nxt(rot_connect_edge_sym);
					sym_edges[rot_connect_edge_sym].rot = inner_edge;
					// connect original edge with its sym
					sym_edges[inner_edge].rot = orig_sym[i];
				}
				// Mark original edges as potential not delaunay 
				// or as point on edge if the point is on any of the edges 
				for (int i = 0; i < 3; i++)
				{
					if (edge_is_constrained[sym_edges[orig_face[i]].edge] > -1)
					{
						// Check if the point is on the edge
						vec2 s1 = point_positions[sym_edges[orig_face[i]].vertex];
						vec2 s2 = point_positions[sym_edges[orig_face[(i + 1) % 3]].vertex];
						vec2 p = point_positions[index];
						if (point_line_test(p, s1, s2))
						{
							edge_label[sym_edges[orig_face[i]].edge] = 3;
						}
						else if (edge_label[sym_edges[orig_face[i]].edge] < 1)
						{
							edge_label[sym_edges[orig_face[i]].edge] = 1; // candidate for not delaunay. 
						}
					}
				}
				// Set point as inserted
				point_inserted[point_index] = 1;

			}
		}
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
	void GCMesh::oriented_walk(int & curr_e, int point_i, bool & on_edge, vec2 & tri_cent)
	{
		bool done = false;
		vec2 goal = point_positions[point_i];
		int iter = 0;
		on_edge = false;
		while (!done) {
			on_edge = false;
			// Loop through triangles edges to check if point is on the edge 
			for (int i = 0; i < 3; i++)
			{
				bool hit = false;
				hit = point_line_test(goal,
					point_positions[sym_edges[curr_e].vertex],
					point_positions[sym_edges[sym_edges[curr_e].nxt].vertex]);
				if (hit)
				{
					on_edge = true;
					return;
				}
				curr_e = sym_edges[curr_e].nxt;
			}
			// calculate triangle centroid
			std::array<vec2, 3> tri_points;
			get_face(sym_edges[curr_e].face, tri_points);
			tri_cent = (tri_points[0] + tri_points[1] + tri_points[2]) / 3.0f;
			// Loop through edges to see if we should continue through the edge
			// to the neighbouring triangle 
			bool line_line_hit = false;
			for (int i = 0; i < 3; i++)
			{
				line_line_hit = line_seg_intersection_ccw(
					tri_cent,
					goal,
					point_positions[sym_edges[curr_e].vertex],
					point_positions[sym_edges[sym_edges[curr_e].nxt].vertex]);

				if (line_line_hit)
				{
					break;
				}
				curr_e = sym_edges[curr_e].nxt;
			}

			if (line_line_hit)
			{
				curr_e = sym_edges[sym_edges[curr_e].nxt].rot; // sym
			}
			else
			{
				return;
			}
		}
	}
	void GCMesh::get_face(int face_i, std::array<vec2, 3>& face_v)
	{
		face_v[0] = point_positions[sym_edges[tri_symedges[face_i].x].vertex];
		face_v[1] = point_positions[sym_edges[sym_edges[tri_symedges[face_i].x].nxt].vertex];
		face_v[2] = point_positions[sym_edges[sym_edges[sym_edges[tri_symedges[face_i].x].nxt].nxt].vertex];
	}
}
