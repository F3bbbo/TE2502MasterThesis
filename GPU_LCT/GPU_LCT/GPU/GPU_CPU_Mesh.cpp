#include "GPU_CPU_Mesh.hpp"
#include <fstream>
#include "../trig_functions.hpp"

namespace GPU
{
	GCMesh::GCMesh()
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
		refine_points = std::vector<NewPoint>(sym_edges.size());
		status = 1;
		//m_status.create_buffer(type, std::vector<int>(1, 1), GL_DYNAMIC_DRAW, 12, 1);
	}

	void GCMesh::add_frame_points(std::vector<glm::vec2> points)
	{
		int num_old_points = point_positions.size();
		new_points.clear();
		new_points.insert(new_points.end(), point_positions.begin(), point_positions.end());
		new_points.insert(new_points.end(), points.begin(), points.end());
		remove_duplicate_points();
		point_positions.insert(point_positions.end(), new_points.begin() + num_old_points, new_points.end());
		int num_new_points = new_points.size() - num_old_points;
		new_points.clear();
		append_vec(point_inserted, std::vector<int>(num_new_points, 0));
		append_vec(point_tri_index, std::vector<int>(num_new_points, 0));

		// increase sizes of arrays, 
		// based on how many new points are inserted
		//append_vec(point_positions, std::vector<glm::vec2>(num_new_points));
		//append_vec(point_inserted, std::vector<int>(num_new_points));
		//append_vec(point_tri_index, std::vector<int>(num_new_points));
		// segments
		int num_new_tri = num_new_points * 2;
		int num_new_segs = num_new_points;

		// fix new sizes of triangle buffers
		append_vec(tri_symedges, std::vector<glm::ivec4>(num_new_tri, { -1, -1, -1, -1 }));
		append_vec(tri_ins_point_index, std::vector<int>(num_new_tri, -1));
		append_vec(tri_edge_flip_index, std::vector<int>(num_new_tri, -1));
		append_vec(tri_seg_inters_index, std::vector<int>(num_new_tri, -1));


		// fix new size of segment buffers
		append_vec(seg_endpoint_indices, std::vector<glm::ivec2>(num_new_segs, glm::ivec2(-1)));
		append_vec(seg_inserted, std::vector<int>(num_new_segs, 0));
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
		append_vec(refine_points, std::vector<NewPoint>(num_new_sym_edges));
		new_points.clear();
		new_points.resize(num_new_points);
		// TODO, maybe need to check if triangle buffers needs to grow
		symedge_buffer_size = sym_edges.size();

		//m_nr_of_symedges.update_buffer<int>({ m_sym_edges.element_count() });
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
		//LOG(std::string("Frame insertion number of iterations: ") + std::to_string(counter));
	}

	long long GCMesh::build_CDT(std::vector<glm::vec2> points, std::vector<glm::ivec2> segments)
	{
		int num_old_points = point_positions.size();
		append_vec(point_positions, points);
		append_vec(point_inserted, std::vector<int>(points.size(), 0));
		append_vec(point_tri_index, std::vector<int>(points.size(), 0));

		for (auto& segment : segments)
			segment = segment + glm::ivec2(num_old_points);
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


		// fix new sizes of edge buffers 
		// TODO: fix so it can handle repeated insertions
		int num_new_edges = points.size() * 3;
		append_vec(edge_is_constrained, std::vector<int>(num_new_edges, -1));
		append_vec(edge_label, std::vector<int>(num_new_edges, -1));
		// fix new size of symedges buffer
		// TODO: fix so it can handle repeated insertions
		int num_new_sym_edges = points.size() * 6;
		append_vec(sym_edges, std::vector<SymEdge>(num_new_sym_edges));
		append_vec(refine_points, std::vector<NewPoint>(num_new_sym_edges));
		// TODO, maybe need to check if triangle buffers needs to grow

		//m_nr_of_symedges.update_buffer<int>({ m_sym_edges.element_count() });
		symedge_buffer_size = sym_edges.size();


		int counter = 0;

		Timer timer;
		timer.start();

		int cont = 1;
		while (cont)
		{
			counter++;
			status = 0;
			//m_status.update_buffer<int>({ 0 });
			//if (counter == 4)
			//	break;
			//// Locate Step
			location_program();
			//glUseProgram(m_location_program);
			//glDispatchCompute((GLuint)256, 1, 1);
			//glMemoryBarrier(GL_ALL_BARRIER_BITS);

			//location_tri_program();
			//glUseProgram(m_location_tri_program);
			//glDispatchCompute((GLuint)256, 1, 1);
			//glMemoryBarrier(GL_ALL_BARRIER_BITS);

			//// Insert Step
			insertion_program();
			//glUseProgram(m_insertion_program);
			//glDispatchCompute((GLuint)256, 1, 1);
			//glMemoryBarrier(GL_ALL_BARRIER_BITS);
			//if (counter == 10)
			//	break;
			//// Marking Step

			marking_part_one_program();
			//glUseProgram(m_marking_part_one_program);
			//glDispatchCompute((GLuint)256, 1, 1);
			//glMemoryBarrier(GL_ALL_BARRIER_BITS);

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

		//LOG(std::string("Number of iterations: ") + std::to_string(counter));
		//LOG(std::string("Elapsed time in ms: ") + std::to_string(timer.elapsed_time()));
		//auto labels_3 = find_equal(edge_label, 3);
		//LOG("Label_3: " + std::to_string(labels_3.size()));
		//auto labels_2 = find_equal(edge_label, 2);
		//LOG("Label_2: " + std::to_string(labels_2.size()));
		//auto labels_1 = find_equal(edge_label, 1);
		//LOG("Label_1: " + std::to_string(labels_1.size()));
		//LOG("Num points: " + std::to_string(point_inserted.size()));
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
		return timer.elapsed_time();
	}

	long long GCMesh::refine_LCT()
	{
		//reset find disturbance status
		find_dist_status = { 0,0,0,0 };

		//reset timer
		Timer timer;
		timer.start();

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
				new_points.clear();
				new_points.resize(num_new_points, vec2(FLT_MAX));
				// add new points to the new_points buffer
				add_new_points_program();
				// remove duplicate points
				remove_duplicate_points();
				// add the the points without duplicates to the point buffers
				append_vec(point_positions, new_points);
				num_new_points = new_points.size();
				append_vec(point_inserted, std::vector<int>(num_new_points, 0));
				append_vec(point_tri_index, std::vector<int>(num_new_points, 0));

				// increase sizes of arrays, 
				// based on how many new points are inserted
				//append_vec(point_positions, std::vector<glm::vec2>(num_new_points));
				//append_vec(point_inserted, std::vector<int>(num_new_points));
				//append_vec(point_tri_index, std::vector<int>(num_new_points));
				// segments
				int num_new_tri = num_new_points * 2;
				int num_new_segs = num_new_points;

				// fix new sizes of triangle buffers
				append_vec(tri_symedges, std::vector<glm::ivec4>(num_new_tri, { -1, -1, -1, -1 }));
				append_vec(tri_ins_point_index, std::vector<int>(num_new_tri, -1));
				append_vec(tri_edge_flip_index, std::vector<int>(num_new_tri, -1));
				append_vec(tri_seg_inters_index, std::vector<int>(num_new_tri, -1));


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
				append_vec(refine_points, std::vector<NewPoint>(num_new_sym_edges));
				new_points.clear();
				new_points.resize(num_new_points);
				// TODO, maybe need to check if triangle buffers needs to grow
				symedge_buffer_size = sym_edges.size();

				//m_nr_of_symedges.update_buffer<int>({ m_sym_edges.element_count() });
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
				//LOG(std::string("LCT Number of iterations: ") + std::to_string(counter));
			}
			else
			{
				break;
			}
		} while (true);
		timer.stop();
		//LOG(std::string("Find_dist.const_list_status: ") + std::to_string(find_dist_status.const_list_status));
		//LOG(std::string("Find_dist.const_queue_status: ") + std::to_string(find_dist_status.const_queue_status));
		//LOG(std::string("Find_dist.dist_list_status: ") + std::to_string(find_dist_status.dist_list_status));
		//LOG(std::string("Find_dist.dist_queue_status: ") + std::to_string(find_dist_status.dist_queue_status));

		return timer.elapsed_time();
	}

	std::vector<glm::vec2> GCMesh::get_vertices()
	{
		return point_positions;
	}



	size_t GCMesh::get_num_vertices()

	{

		return point_positions.size();

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

	std::string GCMesh::save_to_file(bool upload, int inserted_objects)
	{
		std::string filename;

		if (upload)
			filename = "Output files/GC";
		else
			filename = "Output files/throwGC";

		filename += '_' + std::to_string(inserted_objects) + '_' + std::to_string(get_num_vertices());

		std::string str = "";
		std::ofstream output(filename.c_str(), std::ofstream::out | std::ofstream::binary);
		int size;
		if (output.is_open())
		{
			// save point data
			size = (int)point_positions.size() * (int)sizeof(glm::vec2);
			output.write((char*)&size, sizeof(int));
			output.write((char*)point_positions.data(), size);

			size = (int)point_inserted.size() * (int)sizeof(int);
			output.write((char*)&size, sizeof(int));
			output.write((char*)point_inserted.data(), size);

			size = (int)point_tri_index.size() * (int)sizeof(int);
			output.write((char*)&size, sizeof(int));
			output.write((char*)point_tri_index.data(), size);

			// save edge data
			size = (int)edge_label.size() * (int)sizeof(int);
			output.write((char*)&size, sizeof(int));
			output.write((char*)edge_label.data(), size);

			size = (int)edge_is_constrained.size() * (int)sizeof(int);
			output.write((char*)&size, sizeof(int));
			output.write((char*)edge_is_constrained.data(), size);

			// save segment data
			size = (int)seg_endpoint_indices.size() * (int)sizeof(glm::ivec2);
			output.write((char*)&size, sizeof(int));
			output.write((char*)seg_endpoint_indices.data(), size);

			size = (int)seg_inserted.size() * (int)sizeof(int);
			output.write((char*)&size, sizeof(int));
			output.write((char*)seg_inserted.data(), size);

			// save triangle data

			size = (int)tri_symedges.size() * (int)sizeof(glm::ivec4);
			output.write((char*)&size, sizeof(int));
			output.write((char*)tri_symedges.data(), size);

			size = (int)tri_ins_point_index.size() * (int)sizeof(int);
			output.write((char*)&size, sizeof(int));
			output.write((char*)tri_ins_point_index.data(), size);

			size = (int)tri_seg_inters_index.size() * (int)sizeof(int);
			output.write((char*)&size, sizeof(int));
			output.write((char*)tri_seg_inters_index.data(), size);

			size = (int)tri_edge_flip_index.size() * (int)sizeof(int);
			output.write((char*)&size, sizeof(int));
			output.write((char*)tri_edge_flip_index.data(), size);

			// save misc data

			size = (int)this->new_points.size() * (int)sizeof(glm::vec2);
			output.write((char*)&size, sizeof(int));
			output.write((char*)new_points.data(), size);

			size = (int)refine_points.size() * (int)sizeof(NewPoint);
			output.write((char*)&size, sizeof(int));
			output.write((char*)refine_points.data(), size);

			size = (int)sym_edges.size() * (int)sizeof(SymEdge);
			output.write((char*)&size, sizeof(int));
			output.write((char*)sym_edges.data(), size);

			output.close();
		}

		return filename;
	}

	void GCMesh::load_from_file(std::string filename)
	{
		std::ifstream input(filename.c_str(), std::ifstream::in | std::ifstream::binary);
		int value;
		if (input.is_open())
		{
			point_positions.clear();
			point_inserted.clear();
			point_tri_index.clear();

			edge_label.clear();
			edge_is_constrained.clear();

			seg_endpoint_indices.clear();
			seg_inserted.clear();

			tri_symedges.clear();
			tri_ins_point_index.clear();
			tri_seg_inters_index.clear();
			tri_edge_flip_index.clear();

			new_points.clear();
			refine_points.clear();
			sym_edges.clear();

			// read points data
			input.read((char*)&value, sizeof(int));
			point_positions.resize(value / sizeof(glm::vec2));
			input.read((char*)point_positions.data(), value);

			input.read((char*)&value, sizeof(int));
			point_inserted.resize(value / sizeof(int));
			input.read((char*)point_inserted.data(), value);

			input.read((char*)&value, sizeof(int));
			point_tri_index.resize(value / sizeof(int));
			input.read((char*)point_tri_index.data(), value);

			// read edge data
			input.read((char*)&value, sizeof(int));
			edge_label.resize(value / sizeof(int));
			input.read((char*)edge_label.data(), value);

			input.read((char*)&value, sizeof(int));
			edge_is_constrained.resize(value / sizeof(int));
			input.read((char*)edge_is_constrained.data(), value);

			// read segment data
			input.read((char*)&value, sizeof(int));
			seg_endpoint_indices.resize(value / sizeof(glm::ivec2));
			input.read((char*)seg_endpoint_indices.data(), value);

			input.read((char*)&value, sizeof(int));
			seg_inserted.resize(value / sizeof(int));
			input.read((char*)seg_inserted.data(), value);

			// read triangle data
			input.read((char*)&value, sizeof(int));
			tri_symedges.resize(value / sizeof(glm::ivec4));
			input.read((char*)tri_symedges.data(), value);

			input.read((char*)&value, sizeof(int));
			tri_ins_point_index.resize(value / sizeof(int));
			input.read((char*)tri_ins_point_index.data(), value);

			input.read((char*)&value, sizeof(int));
			tri_seg_inters_index.resize(value / sizeof(int));
			input.read((char*)tri_seg_inters_index.data(), value);

			input.read((char*)&value, sizeof(int));
			tri_edge_flip_index.resize(value / sizeof(int));
			input.read((char*)tri_edge_flip_index.data(), value);

			// read misc data

			input.read((char*)&value, sizeof(int));
			new_points.resize(value / sizeof(glm::vec2));
			input.read((char*)new_points.data(), value);

			input.read((char*)&value, sizeof(int));
			refine_points.resize(value / sizeof(NewPoint));
			input.read((char*)refine_points.data(), value);

			input.read((char*)&value, sizeof(int));
			sym_edges.resize(value / sizeof(SymEdge));
			input.read((char*)sym_edges.data(), value);

			symedge_buffer_size = value / sizeof(SymEdge);
			status = 0;

			input.close();
		}
		else
			LOG_T(WARNING, "can not open file:" + filename);
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
	// Access functions
	//-----------------------------------------------------------
	int GCMesh::nxt(int edge)
	{
		return sym_edges[edge].nxt;
	}

	SymEdge GCMesh::nxt(SymEdge s)
	{
		return get_symedge(s.nxt);
	}

	int GCMesh::rot(int edge)
	{
		return sym_edges[edge].rot;
	}

	SymEdge GCMesh::rot(SymEdge s)
	{
		return get_symedge(s.rot);
	}

	int GCMesh::sym(int edge)
	{
		return rot(nxt(edge));
	}

	SymEdge GCMesh::sym(SymEdge s)
	{
		return rot(nxt(s));
	}

	int GCMesh::prev(int edge)
	{
		return nxt(nxt(edge));
	}

	SymEdge GCMesh::prev(SymEdge s)
	{
		return nxt(nxt(s));
	}

	int GCMesh::crot(int edge)
	{
		int sym_i = sym(edge);
		return (sym_i != -1) ? nxt(sym_i) : -1;
	}

	void GCMesh::get_face(int face_i, std::array<vec2, 3>& face_v)
	{
		face_v[0] = point_positions[sym_edges[tri_symedges[face_i].x].vertex];
		face_v[1] = point_positions[sym_edges[sym_edges[tri_symedges[face_i].x].nxt].vertex];
		face_v[2] = point_positions[sym_edges[sym_edges[sym_edges[tri_symedges[face_i].x].nxt].nxt].vertex];
	}

	SymEdge GCMesh::get_symedge(int index)
	{
		return sym_edges[index];
	}

	SymEdge GCMesh::prev_symedge(SymEdge s)
	{
		return get_symedge(get_symedge(s.nxt).nxt);
	}

	SymEdge GCMesh::sym_symedge(SymEdge s)
	{
		return get_symedge(get_symedge(s.nxt).rot);
	}

	int GCMesh::crot_symedge_i(SymEdge s)
	{
		int index = get_symedge(s.nxt).rot;
		return index != -1 ? get_symedge(index).nxt : -1;
	}

	glm::vec2 GCMesh::get_vertex(int index)
	{
		return point_positions[index];
	}

	int GCMesh::get_label(int index)
	{
		return edge_label[index];
	}

	void GCMesh::set_version(int vers)
	{
		version = vers;
	}

	vec2 GCMesh::get_face_center(int face_i)
	{
		vec2 face_v[3];
		face_v[0] = point_positions[sym_edges[tri_symedges[face_i].x].vertex];
		face_v[1] = point_positions[sym_edges[sym_edges[tri_symedges[face_i].x].nxt].vertex];
		face_v[2] = point_positions[sym_edges[sym_edges[sym_edges[tri_symedges[face_i].x].nxt].nxt].vertex];

		return (face_v[0] + face_v[1] + face_v[2]) / 3.f;
	}

	int GCMesh::get_index(SymEdge s)
	{
		return prev_symedge(s).nxt;
	}

	int GCMesh::get_face_vertex_symedge(int face, int vertex)
	{
		SymEdge s = sym_edges[tri_symedges[face].x];
		if (s.vertex == vertex)
			return get_index(s);
		else if (get_symedge(s.nxt).vertex == vertex)
			return s.nxt;
		else if (prev_symedge(s).vertex == vertex)
			return get_index(prev_symedge(s));
		else
			return -1;
	}

	bool GCMesh::face_contains_vertice(int face, int vertex)
	{
		SymEdge s = sym_edges[tri_symedges[face].x];
		return s.vertex == vertex || get_symedge(s.nxt).vertex == vertex || prev_symedge(s).vertex == vertex;
	}

	bool GCMesh::face_contains_vertex(int vert, SymEdge s_triangle)
	{
		ivec3 tri = ivec3(s_triangle.vertex, get_symedge(s_triangle.nxt).vertex, prev_symedge(s_triangle).vertex);
		return vert == tri.x || vert == tri.y || vert == tri.z;
	}

	std::array<vec2, 2> GCMesh::get_segment(int index)
	{
		ivec2 seg_edge_i = seg_endpoint_indices[index];
		std::array<vec2, 2> s;
		s[0] = point_positions[seg_edge_i[0]];
		s[1] = point_positions[seg_edge_i[1]];
		return s;
	}

	std::array<vec2, 2> GCMesh::get_edge(int s_edge)
	{
		std::array<vec2, 2> edge;
		edge[0] = point_positions[sym_edges[s_edge].vertex];
		edge[1] = point_positions[sym_edges[nxt(s_edge)].vertex];
		return edge;
	}

	Find_Disturbance_Status GCMesh::get_find_dist_status()
	{
		return find_dist_status;
	}




	//-----------------------------------------------------------
	// Program functions
	//-----------------------------------------------------------
	void GCMesh::location_program()
	{
		for (int index = 0; index < point_positions.size(); index++)
		{
			if (point_inserted[index] == 0)
			{
				bool on_edge;
				// find out which triangle the point is now
				int curr_e = tri_symedges[point_tri_index[index]].x;;
				oriented_walk(
					curr_e,
					index,
					on_edge);
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

				int face = sym_edges[curr_e].face;
				std::array<glm::vec2, 3> tri_points;
				get_face(face, tri_points);
				vec2 triangle_center = (tri_points[0] + tri_points[1] + tri_points[2]) / 3.0f;
				float len = line_length(point_positions[index] - triangle_center);

				if (!point_ray_test(tri_points[0], tri_points[1], tri_points[2]) && valid_point_into_face(face, point_positions[index]))
				{
					if (tri_ins_point_index[face] == -1 || len < line_length(point_positions[tri_ins_point_index[face]] - triangle_center))
					{
						tri_ins_point_index[face] = index;
					}
				}

				// old solution
				//point_tri_index[index] = sym_edges[curr_e].face;
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
				// check so the triangle is not a degenerate triangle
				if (!point_ray_test(tri_points[0], tri_points[1], tri_points[2]))
				{
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
									// Check so point is not on an edge with label 3
									if (valid_point_into_face(index, pos))
									{
										best_dist = dist;
										point_index = i;
									}
								}
							}
						}
					}
					tri_ins_point_index[index] = point_index;
				}
			}
		}
	}
	void GCMesh::insertion_program()
	{
		for (int index = 0; index < tri_seg_inters_index.size(); index++)
		{
			// If triangle has a point assigned to it add the point to it
			int point_index = tri_ins_point_index[index];
			if (point_index > -1 /*&& is_valid_face(index)*/)
			{
				status = 1;
				//if (!is_valid_face(index))
				//	break;

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
					sym = nxt(sym);
					sym = rot(sym);
					orig_sym[i] = sym;
					// move curr_e to next edge of triangle
					curr_e = nxt(curr_e);
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
					inner_edge = nxt(inner_edge);
					int inner_edge_sym = orig_face[next_id];
					inner_edge_sym = nxt(inner_edge_sym);
					inner_edge_sym = nxt(inner_edge_sym);
					// set same edge index to both symedges
					sym_edges[inner_edge].edge = new_edge;
					sym_edges[inner_edge_sym].edge = new_edge;
					// connect the edges syms together
					int rot_connect_edge = inner_edge;
					rot_connect_edge = nxt(rot_connect_edge);
					sym_edges[rot_connect_edge].rot = inner_edge_sym;
					int rot_connect_edge_sym = inner_edge_sym;
					rot_connect_edge_sym = nxt(rot_connect_edge_sym);
					sym_edges[rot_connect_edge_sym].rot = inner_edge;
					// connect original edge with its sym
					sym_edges[inner_edge].rot = orig_sym[i];
				}
				// Mark original edges as potential not delaunay 
				// or as point on edge if the point is on any of the edges 
				for (int i = 0; i < 3; i++)
				{
					if (edge_is_constrained[sym_edges[orig_face[i]].edge] < 0)
					{
						// Check if the point is on the edge
						vec2 s1 = point_positions[sym_edges[orig_face[i]].vertex];
						vec2 s2 = point_positions[sym_edges[orig_face[(i + 1) % 3]].vertex];
						vec2 p = point_positions[point_index];
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
				tri_ins_point_index[index] = -1;
			}
		}
	}
	void GCMesh::marking_part_one_program()
	{
		for (int index = 0; index < seg_inserted.size(); index++)
		{
			if (seg_inserted[index] == 0)
			{
				if (seg_endpoint_indices[index].x > -1)
				{
					int endpoints_inserted = point_inserted[seg_endpoint_indices[index].x] * point_inserted[seg_endpoint_indices[index].y];
					if (endpoints_inserted == 1)
					{
						int start_index = tri_symedges[point_tri_index[seg_endpoint_indices[index].x]].x;
						int starting_symedge = oriented_walk_point(start_index, seg_endpoint_indices[index].x);
						int ending_symedge = oriented_walk_point(starting_symedge, seg_endpoint_indices[index].y);
						// update the points triangle indexes
						point_tri_index[sym_edges[starting_symedge].vertex] = sym_edges[starting_symedge].face;
						point_tri_index[sym_edges[ending_symedge].vertex] = sym_edges[ending_symedge].face;
						int connecting_edge = points_connected(starting_symedge, ending_symedge);
						if (connecting_edge != -1)
						{
							edge_is_constrained[connecting_edge] = index;
							edge_label[connecting_edge] = 0;
							seg_inserted[index] = 1;
							status = 1;
						}
						else
						{
							straight_walk(index, get_symedge(starting_symedge), seg_endpoint_indices[index].y);
							straight_walk(index, get_symedge(ending_symedge), seg_endpoint_indices[index].x);
						}
					}
				}
			}
		}
	}
	void GCMesh::marking_part_two_program()
	{
		for (unsigned int index = 0; index < tri_seg_inters_index.size(); index++)
		{
			int tri_sym = tri_symedges[index].x;
			if (tri_sym > -1)
			{
				bool no_point_in_edges = edge_label[get_symedge(tri_sym).edge] != 3 &&
					edge_label[get_symedge(nxt(tri_sym)).edge] != 3 &&
					edge_label[get_symedge(prev(tri_sym)).edge] != 3;
				// go through edges checking so edges can be flipped
				// TODO: move this to be done only if there is no edge with label 2 in the triangle
				// Problem: A problem would occur with the flipping when a triangle with a label 2 
				// would discard that label and then would not process the label 1 appropriately, 
				// so now label ones are processed all the time.
				for (int i = 0; i < 3; i++)
				{
					if (edge_label[get_symedge(tri_sym).edge] == 1 && ((is_delaunay(tri_sym)) || edge_is_constrained[get_symedge(tri_sym).edge] > -1))
						edge_label[get_symedge(tri_sym).edge] = 0;

					tri_sym = nxt(tri_sym);
				}
				if (no_point_in_edges)
				{
					if (tri_seg_inters_index[index] == -1)
					{
						for (int i = 0; i < 3; i++)
						{
							if (edge_label[get_symedge(tri_sym).edge] == 1 && ((is_delaunay(tri_sym)) || edge_is_constrained[get_symedge(tri_sym).edge] > -1))
								edge_label[get_symedge(tri_sym).edge] = 0;

							tri_sym = nxt(tri_sym);
						}
					}
					else
					{
						for (int i = 0; i < 3; i++)
						{
							int adjacent_triangle = get_symedge(nxt(tri_sym)).rot;
							if (adjacent_triangle != -1 && edge_label[get_symedge(tri_sym).edge] == 2)
							{
								vec2 segment_vertices[2];
								segment_vertices[0] = get_vertex(seg_endpoint_indices[tri_seg_inters_index[index]].x);
								segment_vertices[1] = get_vertex(seg_endpoint_indices[tri_seg_inters_index[index]].y);

								std::array<vec2, 3> face_vertices;
								get_face(get_symedge(rot(nxt(tri_sym))).face, face_vertices);

								if (!segment_triangle_test(segment_vertices[0], segment_vertices[1], face_vertices[0], face_vertices[1], face_vertices[2]))
									edge_label[get_symedge(tri_sym).edge] = 0;
							}
							tri_sym = nxt(tri_sym);
						}
					}
				}
			}
		}
	}
	void GCMesh::flip_edges_part_one_program()
	{
		for (unsigned int index = 0; index < tri_seg_inters_index.size(); index++)
		{
			if (tri_symedges[index].x > -1)
			{
				int highest_priority_s_edge = -1;
				int h = -1;

				SymEdge edge_sym = get_symedge(tri_symedges[index].x);
				for (int i = 0; i < 3; i++)
				{
					highest_priority_s_edge = h < edge_label[edge_sym.edge] ? get_index(edge_sym) : highest_priority_s_edge;
					h = max(edge_label[edge_sym.edge], h);
					edge_sym = nxt(edge_sym);
				}

				if (h > 0)
				{
					// calculate nh to know how many other edges the that can be tested
					int nh = 0;
					for (int i = 0; i < 3; i++)
					{
						nh = h == edge_label[edge_sym.edge] ? nh + 1 : nh;
						edge_sym = nxt(edge_sym);
					}
					int num_iter = nh;
					while (num_iter > 0)
					{
						int sym_symedge = nxt(get_symedge(highest_priority_s_edge)).rot;
						if (sym_symedge != -1)
						{
							int o_label1 = edge_label[nxt(get_symedge(sym_symedge)).edge];
							int o_label2 = edge_label[prev_symedge(get_symedge(sym_symedge)).edge];

							if (o_label1 != h && o_label2 != h && o_label1 < h && o_label2 < h)
							{
								if (nh >= 2 || (nh == 1 && index < get_symedge(sym_symedge).face))
									tri_edge_flip_index[index] = get_symedge(highest_priority_s_edge).edge;
								else
									tri_edge_flip_index[index] = -1;
							}
						}
						// find next edge with label h
						edge_sym = sym_edges[highest_priority_s_edge];
						for (int i = 0; i < 3; i++)
						{
							edge_sym = nxt(edge_sym);
							if (h == edge_label[edge_sym.edge])
							{
								highest_priority_s_edge = get_index(edge_sym);
								break;
							}
						}
						num_iter--;
					}
				}
			}
		}
	}
	void GCMesh::flip_edges_part_two_program()
	{
		for (unsigned int index = 0; index < tri_seg_inters_index.size(); index++)
		{
			if (tri_symedges[index].x > -1)
			{
				if (tri_edge_flip_index[index] == -1)
				{
					SymEdge edge_sym = get_symedge(tri_symedges[index].x);
					for (int i = 0; i < 3; i++)
					{
						int sym = nxt(edge_sym).rot;
						if (sym != -1 && tri_edge_flip_index[get_symedge(sym).face] == edge_sym.edge)
						{
							// This feels like bullshit
							// tri_edge_flip_index[index] = edge_label[edge_sym.edge];
							set_quad_edges_label(1, edge_sym);
							break;
						}
						edge_sym = nxt(edge_sym);
					}
				}
			}
		}
	}
	void GCMesh::flip_edges_part_three_program()
	{
		for (unsigned int index = 0; index < tri_seg_inters_index.size(); index++)
		{
			if (tri_edge_flip_index[index] != -1 && edge_label[tri_edge_flip_index[index]] != -1)
			{
				status = 1;

				// find the symedge that constains the edge that should get flipped
				SymEdge edge_to_be_flipped = get_symedge(tri_symedges[index].x);
				SymEdge cur_edge = edge_to_be_flipped;
				for (int i = 0; i < 3; i++)
				{
					edge_to_be_flipped = cur_edge.edge == tri_edge_flip_index[index] ? cur_edge : edge_to_be_flipped;
					cur_edge = nxt(cur_edge);
				}

				edge_label[edge_to_be_flipped.edge] = 0;
				tri_edge_flip_index[index] = -1;
				flip_edge(edge_to_be_flipped);
			}
		}
	}

	void GCMesh::locate_disturbances_program()
	{
		for (int index = 0; index < tri_seg_inters_index.size(); index++)
		{
			if (tri_symedges[index].x > -1)
			{
				int num_constraints = 0;
				int c_edge_i[3] = { -1, -1, -1 };
				int tri_edge_i[3];
				// loop checking for constraints in triangle
				for (int i = 0; i < 3; i++)
				{
					// Checking if edge of triangle is constrained.
					if (edge_is_constrained[sym_edges[tri_symedges[index][i]].edge] > -1)
					{
						c_edge_i[num_constraints] = tri_symedges[index][i];
						tri_edge_i[num_constraints] = tri_symedges[index][i];
						num_constraints++;
					}
				}
				// if no constraints where found in triangle look for nearby constraints
				if (num_constraints == 0)
				{
					// Loop through edges of the triangles finding the closest constraints
					// to each traversal.
					ivec4 tri_symedge_i = tri_symedges[index];
					std::array<vec2, 3> tri;
					get_face(sym_edges[tri_symedge_i.x].face, tri);
					int curr_ac = tri_symedge_i.x;
					//			tri_insert_points[index].pos = tri[0];
					for (int i = 0; i < 3; i++)
					{
						int cc;
						if(version == 1)
							cc = find_closest_constraint(tri[(i + 1) % 3], tri[(i + 2) % 3], tri[i]);
						else
							cc = find_closest_constraint_v2(curr_ac, tri[(i + 1) % 3], tri[(i + 2) % 3], tri[i]);
						// Check if a segment was found
						if (cc > -1)
						{
							if(version == 1)
								cc = find_segment_symedge(tri_symedge_i[i], cc);
							// Check if corresponding constraint to segment was found
							if (cc > -1)
							{
								c_edge_i[num_constraints] = cc;
								tri_edge_i[num_constraints] = tri_symedge_i[i];
								num_constraints++;
							}
						}
						curr_ac = nxt(curr_ac);
					}
				}

				// find disturbances
				for (int i = 0; i < num_constraints; i++)
				{
					if (c_edge_i[i] > -1)
					{
						// test right side
						// chose function based on which version is running
						int disturb;
						if(version == 1)
							disturb = find_constraint_disturbance(c_edge_i[i], tri_edge_i[i], true);
						else if(version == 2)
							disturb = find_constraint_disturbance_v2(c_edge_i[i], tri_edge_i[i], true);
						else 
							disturb = find_constraint_disturbance_v2(c_edge_i[i], tri_edge_i[i], true);

						if (disturb >= 0)
						{
							bool success;
							vec2 calc_pos = calculate_refinement(c_edge_i[i], disturb, success);
							if (success)
							{
								NewPoint tmp;
								tmp.pos = calc_pos;
								//tmp.index = atomicAdd(status, 1);
								tmp.index = status++;
								tmp.face_i = sym_edges[c_edge_i[i]].face;
								refine_points[disturb] = tmp;
							}
						}
						// test left side
						// chose function based on which version is running
						if(version == 1)
							disturb = find_constraint_disturbance(c_edge_i[i], tri_edge_i[i], false);
						else if(version == 2) 
							disturb = find_constraint_disturbance_v2(c_edge_i[i], tri_edge_i[i], false);
						else
							disturb = find_constraint_disturbance_v2(c_edge_i[i], tri_edge_i[i], false);

						if (disturb >= 0)
						{
							bool success;
							vec2 calc_pos = calculate_refinement(c_edge_i[i], disturb, success);
							if (success)
							{
								NewPoint tmp;
								tmp.pos = calc_pos;
								//tmp.index = atomicAdd(status, 1);
								tmp.index = status++;
								tmp.face_i = sym_edges[c_edge_i[i]].face;
								refine_points[disturb] = tmp;
							}
						}
					}

				}

			}
		}
	}

	void GCMesh::add_new_points_program()
	{
		for (int index = 0; index < symedge_buffer_size; index++)
		{
			NewPoint new_point = refine_points[index];
			if (new_point.index >= 0)
			{
				new_points[new_point.index] = new_point.pos;
				// reset the insert point data structure
				new_point.pos = vec2(0.0f);
				new_point.index = -1;
				new_point.face_i = -1;
				refine_points[index] = new_point;
			}
		}
	}

	void GCMesh::remove_duplicate_points()
	{
		// loops backwards to remove as far back as possible
		int i = 0;
		int num_valid_points = new_points.size();
		while (i < num_valid_points)
		{
			if (point_equal(new_points[i], vec2(FLT_MAX)))
			{
				vec2 tmp = new_points[i];
				new_points[i] = new_points[--num_valid_points];
				new_points[num_valid_points] = tmp;
				continue;
			}
			int j = i + 1;
			while (j < num_valid_points)
			{
				if (point_equal(new_points[i], new_points[j]) )
				{
					// swap away point behind the valid pointsto be removed later.
					vec2 tmp = new_points[j];
					new_points[j] = new_points[--num_valid_points];
					new_points[num_valid_points] = tmp;
					j--;
				}
				j++;
			}
			i++;
		}
		// removed the last points
		new_points.erase(new_points.begin() + num_valid_points, new_points.end());

	}

	void GCMesh::insert_in_edge_program()
	{
		for (int index = 0; index < tri_seg_inters_index.size(); index++)
		{
			if (tri_ins_point_index[index] >= 0)
			{
				status = 1;
				int point_index = tri_ins_point_index[index];
				point_inserted[point_index] = 1;
				tri_ins_point_index[index] = -1;

				int segment = tri_symedges[index].x;

				for (int i = 0; i < 2; i++)
				{
					if (point_line_test(get_vertex(point_index),
						get_vertex(get_symedge(segment).vertex),
						get_vertex(get_symedge(nxt(segment)).vertex)))
						break;
					segment = nxt(segment);
				}

				int e1 = nxt(segment);
				int e2 = prev(segment);

				int e1_sym = sym(e1);
				int e2_sym = sym(e2);

				ivec2 segment_symedges = ivec2(segment, rot(nxt(segment)));
				int new_symedges[6];

				new_symedges[0] = symedge_buffer_size - 6 * (point_positions.size() - point_index);
				new_symedges[1] = symedge_buffer_size - 6 * (point_positions.size() - point_index) + 1;
				new_symedges[2] = symedge_buffer_size - 6 * (point_positions.size() - point_index) + 2;
				new_symedges[3] = symedge_buffer_size - 6 * (point_positions.size() - point_index) + 3;
				new_symedges[4] = symedge_buffer_size - 6 * (point_positions.size() - point_index) + 4;
				new_symedges[5] = symedge_buffer_size - 6 * (point_positions.size() - point_index) + 5;

				int t0 = get_symedge(segment_symedges[0]).face;
				int t1 = tri_seg_inters_index.size() - 2 * (point_positions.size() - point_index);
				int t2 = tri_seg_inters_index.size() - 2 * (point_positions.size() - point_index) + 1;


				int edge1 = edge_label.size() - 3 * (point_positions.size() - point_index);
				int edge2 = edge_label.size() - 3 * (point_positions.size() - point_index) + 1;
				int edge3 = edge_label.size() - 3 * (point_positions.size() - point_index) + 2;

				edge_label[edge1] = 0;
				edge_label[edge2] = 0;
				edge_label[edge3] = 0;

				// e1
				sym_edges[e1].nxt = new_symedges[2];
				sym_edges[e1].rot = segment_symedges[1];

				// e2
				sym_edges[e2].nxt = new_symedges[0];
				sym_edges[e2].rot = new_symedges[2];
				sym_edges[e2].face = t1;

				// new 0
				sym_edges[new_symedges[0]].nxt = new_symedges[1];
				sym_edges[new_symedges[0]].rot = e2_sym;
				sym_edges[new_symedges[0]].vertex = get_symedge(segment_symedges[0]).vertex;
				sym_edges[new_symedges[0]].edge = edge1;
				sym_edges[new_symedges[0]].face = t1;

				// new 1
				sym_edges[new_symedges[1]].nxt = e2;
				sym_edges[new_symedges[1]].rot = -1; // this will be fixed later if sym(seg) is not null
				sym_edges[new_symedges[1]].vertex = point_index;
				sym_edges[new_symedges[1]].edge = edge2;
				sym_edges[new_symedges[1]].face = t1;

				// new 2
				sym_edges[new_symedges[2]].nxt = segment_symedges[0];
				sym_edges[new_symedges[2]].rot = e1_sym;
				sym_edges[new_symedges[2]].vertex = get_symedge(e2).vertex;
				sym_edges[new_symedges[2]].edge = edge2;
				sym_edges[new_symedges[2]].face = t0;

				// old segment 0
				sym_edges[segment_symedges[0]].nxt = e1;
				sym_edges[segment_symedges[0]].rot = new_symedges[1];
				sym_edges[segment_symedges[0]].vertex = point_index;

				tri_symedges[t0] = ivec4(segment_symedges[0], e1, new_symedges[2], -1);
				tri_symedges[t1] = ivec4(new_symedges[1], e2, new_symedges[0], -1);

				int new_segment_index = seg_inserted.size() - (seg_inserted.size() - point_index);
				int old_segment_index = edge_is_constrained[get_symedge(segment).edge];
				seg_endpoint_indices[old_segment_index] = ivec2(point_index, get_symedge(e1).vertex);	// reused segment
				seg_endpoint_indices[new_segment_index] = ivec2(get_symedge(new_symedges[0]).vertex, point_index);		// new segment

				edge_is_constrained[edge1] = new_segment_index;
				//edge_is_constrained[edge3] = old_segment_index;

				// mark as maybe non delauney
				seg_inserted[new_segment_index] = 1;

				edge_label[get_symedge(e1).edge] = edge_is_constrained[get_symedge(e1).edge] == -1 ? 1 : edge_label[get_symedge(e1).edge];
				edge_label[get_symedge(e2).edge] = edge_is_constrained[get_symedge(e2).edge] == -1 ? 1 : edge_label[get_symedge(e2).edge];

				if (rot(nxt(segment)) != -1)
				{
					int t3 = get_symedge(segment_symedges[1]).face;
					int e3 = nxt(sym(segment));
					int e4 = nxt(nxt(sym(segment)));

					int e3_sym = sym(e3);
					int e4_sym = sym(e4);

					// new 1 fix
					sym_edges[new_symedges[1]].rot = new_symedges[5];

					// e3
					sym_edges[e3].nxt = new_symedges[4];
					sym_edges[e3].rot = new_symedges[0];
					sym_edges[e3].face = t2;

					// e4
					sym_edges[e4].nxt = segment_symedges[1];
					sym_edges[e4].rot = new_symedges[4];

					// new 3
					sym_edges[new_symedges[3]].nxt = e4;
					sym_edges[new_symedges[3]].rot = segment_symedges[0];
					sym_edges[new_symedges[3]].vertex = point_index;
					sym_edges[new_symedges[3]].edge = edge3;
					sym_edges[new_symedges[3]].face = t3;

					// new 4
					sym_edges[new_symedges[4]].nxt = new_symedges[5];
					sym_edges[new_symedges[4]].rot = e3_sym;
					sym_edges[new_symedges[4]].vertex = get_symedge(e4).vertex;
					sym_edges[new_symedges[4]].edge = edge3;
					sym_edges[new_symedges[4]].face = t2;

					// new 5
					sym_edges[new_symedges[5]].nxt = e3;
					sym_edges[new_symedges[5]].rot = new_symedges[3];
					sym_edges[new_symedges[5]].vertex = point_index;
					sym_edges[new_symedges[5]].edge = edge1;
					sym_edges[new_symedges[5]].face = t2;

					// old segment 1
					sym_edges[segment_symedges[1]].nxt = new_symedges[3];
					sym_edges[segment_symedges[1]].rot = e4_sym;

					tri_symedges[t2] = ivec4(new_symedges[5], e3, new_symedges[4], -1);
					tri_symedges[t3] = ivec4(new_symedges[3], e4, segment_symedges[1], -1);



					// mark as maybe non delauney
					edge_label[get_symedge(e3).edge] = edge_is_constrained[get_symedge(e3).edge] == -1 ? 1 : edge_label[get_symedge(e3).edge];
					edge_label[get_symedge(e4).edge] = edge_is_constrained[get_symedge(e4).edge] == -1 ? 1 : edge_label[get_symedge(e4).edge];
				}
			}
		}
	}

	void GCMesh::locate_point_triangle_program()
	{
		for (int index = 0; index < point_positions.size(); index++)
		{
			if (point_inserted[index] == 0)
			{
				int curr_e = point_tri_index[index];
				bool on_edge = false;
				oriented_walk(curr_e, index, on_edge);
				point_tri_index[index] = curr_e;
				tri_ins_point_index[sym_edges[curr_e].face] = index;
			}
		}
	}

	void GCMesh::validate_edges_program()
	{
		for (int index = 0; index < tri_symedges.size(); index++)
		{
			if (tri_ins_point_index[index] != -1)
			{
				// find the symedge which the point should be inserted into

				vec2 p = get_vertex(tri_ins_point_index[index]);
				SymEdge curr_insertion_symedge = get_symedge(tri_symedges[index].x);

				for (int i = 0; i < 3; i++)
				{
					if (point_line_test(p,
						get_vertex(curr_insertion_symedge.vertex),
						get_vertex(nxt(curr_insertion_symedge).vertex)))
						break;
					curr_insertion_symedge = nxt(curr_insertion_symedge);
				}

				// Check adjacent triangles for if they want to insert their point into one of their edges
				int adjacent_face_index = -1;

				for (int adjacent_triangle = 0; adjacent_triangle < 3; adjacent_triangle++)
				{
					if (adjacent_tri_point_intersects_edge(curr_insertion_symedge, adjacent_face_index) && index > adjacent_face_index)
					{
						tri_ins_point_index[index] = -1;
						return;
					}
					curr_insertion_symedge = nxt(curr_insertion_symedge);
				}

				// check other side
				if (nxt(curr_insertion_symedge).rot != -1)
				{
					curr_insertion_symedge = nxt(sym(curr_insertion_symedge));

					for (int adjacent_triangle = 0; adjacent_triangle < 2; adjacent_triangle++)
					{
						if (adjacent_tri_point_intersects_edge(curr_insertion_symedge, adjacent_face_index) && index > adjacent_face_index)
						{
							tri_ins_point_index[index] = -1;
							return;
						}
						curr_insertion_symedge = nxt(curr_insertion_symedge);
					}
				}
			}
		}
	}


	//-----------------------------------------------------------
	// Shader functions
	//-----------------------------------------------------------
	void GCMesh::oriented_walk(int & curr_e, int point_i, bool & on_edge)
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
			vec2 tri_cent = (tri_points[0] + tri_points[1] + tri_points[2]) / 3.0f;
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
				curr_e = nxt(sym_edges[sym_edges[curr_e].nxt].rot); // sym
			}
			else
			{
				return;
			}
		}
	}
	bool GCMesh::is_valid_face(int face_i)
	{
		ivec4 syms = tri_symedges[face_i];
		//for (int i = 0; i < 3; i++)
		//{
		//	if (edge_label[sym_edges[syms[i]].edge] == 3)
		//		return false;
		//}
		vec2 a = point_positions[sym_edges[syms.x].vertex];
		vec2 b = point_positions[sym_edges[syms.y].vertex];
		vec2 c = point_positions[sym_edges[syms.z].vertex];
		vec2 ab = b - a;
		vec2 ac = c - a;

		return (abs(dot(normalize(ab), normalize(ac))) < (1.0f - EPSILON)) ? true : false;
	}
	bool GCMesh::valid_point_into_face(int face, vec2 p)
	{
		int curr_e = tri_symedges[face].x;
		for (int i = 0; i < 3; i++)
		{
			SymEdge curr_sym = sym_edges[curr_e];
			vec2 s1 = point_positions[curr_sym.vertex];
			vec2 s2 = point_positions[sym_edges[nxt(curr_e)].vertex];
			if (point_line_test(p, s1, s2) && edge_label[curr_sym.edge] == 3)
			{
				int e_sym = sym(curr_e);
				if (e_sym > -1)
				{
					std::array<vec2, 3> tri_points;
					get_face(sym_edges[e_sym].face, tri_points);
					if (point_ray_test(tri_points[0], tri_points[1], tri_points[2]))
					{
						return false;
					}
				}
			}
		}
		return true;
	}
	int GCMesh::oriented_walk_point(int start_e, int goal_point_i)
	{
		int curr_e = start_e;
		vec2 tri_cent;
		vec2 goal_point = get_vertex(goal_point_i);
		float epsi = EPSILON;
		int counter = 0;
		while (true)
		{
			counter++;
			//if (counter > 1001)
			//{
			//	edge_label[sym_edges[curr_e].edge] = -2;
			//	edge_is_constrained[sym_edges[curr_e].edge] = goal_point_i;
			//	return -1;
			//}
			if (face_contains_vertice(sym_edges[curr_e].face, goal_point_i))
				return get_face_vertex_symedge(sym_edges[curr_e].face, goal_point_i);
			//check if current triangle is a sliver triangle
			bool sliver_tri = check_for_sliver_tri(curr_e);
			if (sliver_tri)
			{
				float dir;
				do {
					// continue until the edge of triangle is found that is facing the other
					// compared to the intial edge, so we are checking when the edges start going the other
					// direction by doing the dot product between the last edge and next edge, when the dot is 
					// negative it implies that the next edge is facing another direction than the previous ones.
					curr_e = nxt(curr_e);
					vec2 ab = point_positions[sym_edges[curr_e].vertex] - point_positions[sym_edges[prev(curr_e)].vertex];
					vec2 bc = point_positions[sym_edges[nxt(curr_e)].vertex] - point_positions[sym_edges[curr_e].vertex];
					dir = dot(ab, bc);
				} while (dir > 0.0f);
				int last_e = curr_e;
				do
				{
					curr_e = sym(last_e);
					last_e = nxt(last_e);
				} while (curr_e < 0);
			}
			else
			{
				curr_e = nxt(curr_e);
				// No degenerate triangles detected
				tri_cent = get_face_center(sym_edges[curr_e].face);

				for (int i = 0; i < 3; i++)
				{
					if (sym(curr_e) < 0)
					{
						curr_e = nxt(curr_e);
						continue;
					}
					vec2 s1 = point_positions[sym_edges[curr_e].vertex];
					vec2 s2 = point_positions[sym_edges[sym_edges[curr_e].nxt].vertex];


					bool line_line_hit = point_line_test(goal_point, s1, s2) || line_line_test(
						tri_cent,
						goal_point,
						s1,
						s2);
					if (line_line_hit)
					{
						curr_e = sym(curr_e);
						break;
					}
					else
					{
						curr_e = nxt(curr_e);
					}
				}
			}
		}
		return -1;
	}

	int GCMesh::points_connected(int e1, int e2)
	{
		int curr_e = e1;
		bool reverse_direction = false;
		int other_vertex = get_symedge(e2).vertex;

		while (true)
		{
			if (get_symedge(sym_edges[curr_e].nxt).vertex == other_vertex)
				return sym_edges[curr_e].edge;

			else
			{
				if (!reverse_direction)
				{
					if (sym_edges[curr_e].rot == e1)
						return -1;

					if (sym_edges[curr_e].rot != -1)
						curr_e = get_symedge(curr_e).rot;
					else
					{
						reverse_direction = true;
						curr_e = crot_symedge_i(get_symedge(e1));
						if (curr_e == -1)
							return -1;
					}
				}
				else
				{
					curr_e = crot_symedge_i(get_symedge(curr_e));
					if (curr_e == -1)
						return -1;
				}
			}
		}
	}

	bool GCMesh::check_for_sliver_tri(int sym_edge)
	{
		// first find the longest edge
		float best_dist = 0.0f;
		int best_i = -1;
		std::array<vec2, 3> tri;
		get_face(sym_edges[sym_edge].face, tri);
		for (int i = 0; i < 3; i++)
		{
			float dist = distance(tri[i], tri[(i + 1) % 3]);
			if (dist > best_dist)
			{
				best_i = i;
				best_dist = dist;
			}
		}
		// then check if the third point is on the ray of that line.
		vec2 p1 = tri[(best_i + 2) % 3];
		vec2 s1 = tri[best_i];
		vec2 s2 = tri[(best_i + 1) % 3];
		return point_ray_test(p1, s1, s2);
	}

	void GCMesh::straight_walk(int segment_index, SymEdge s_starting_point, int ending_point_i)
	{
		SymEdge cur_edge = s_starting_point;
		SymEdge prev_intersecting_edge = s_starting_point;
		vec2 constraint_edge[2];
		constraint_edge[0] = get_vertex(s_starting_point.vertex);
		constraint_edge[1] = get_vertex(ending_point_i);
		vec2 normalized_constrained_edge = normalize(constraint_edge[1] - constraint_edge[0]);

		while (true)
		{
			vec2 v0 = get_vertex(get_symedge(cur_edge.nxt).vertex);
			vec2 v1 = get_vertex(prev_symedge(cur_edge).vertex);
			if (line_seg_intersection_ccw(constraint_edge[0], constraint_edge[1], v0, v1))
			{
				cur_edge = get_symedge(cur_edge.nxt);

				// Check if the sym triangle contains the segment endpoint
				if (face_contains_vertex(ending_point_i, sym_symedge(cur_edge)))
				{
					process_triangle(segment_index, cur_edge);
					process_triangle(segment_index, sym_symedge(cur_edge));

					// check if the edge satisfies to conditions and mark it to be flipped if needed, should always return here.
					if (pre_candidate_check(cur_edge))
						will_be_flipped(segment_index, cur_edge);
					return;
				}

				prev_intersecting_edge = cur_edge;
				process_triangle(segment_index, cur_edge);
				if (pre_candidate_check(cur_edge) && will_be_flipped(segment_index, cur_edge))
					return;
				cur_edge = get_symedge(sym_symedge(cur_edge).nxt);
				break;
			}
			cur_edge = get_symedge(cur_edge.rot);
			if (cur_edge == s_starting_point)
				return;
		}

		// walk towards the constraint endpoins and stop if we reach the triangle that contains the segment endpoint
		while (true)
		{
			int checks;
			for (checks = 0; checks < 2; checks++)
			{
				// Check if the sym triangle contains the segment endpoint
				if (face_contains_vertex(ending_point_i, sym_symedge(cur_edge)))
				{
					process_triangle(segment_index, cur_edge);
					process_triangle(segment_index, sym_symedge(cur_edge));
					if (pre_candidate_check(cur_edge))
						will_be_flipped(segment_index, cur_edge);
					return;
				}

				// Checks if the segment intersects an edge
				vec2 v0 = get_vertex(cur_edge.vertex);
				vec2 v1 = get_vertex(get_symedge(cur_edge.nxt).vertex);

				if (line_line_test(constraint_edge[0], constraint_edge[1], v0, v1))
				{
					process_triangle(segment_index, cur_edge);
					if (Qi_check(segment_index, prev_intersecting_edge, cur_edge) && will_be_flipped(segment_index, cur_edge))
						return;
					prev_intersecting_edge = cur_edge;
					cur_edge = get_symedge(sym_symedge(cur_edge).nxt);
					break;
				}
				cur_edge = get_symedge(cur_edge.nxt);
			}


		}

	}

	void GCMesh::process_triangle(int segment_index, SymEdge triangle)
	{
		// Assumes that the provided symedge is the symedge between the trianlges T, T+1
		if (get_label(triangle.edge) != 3 &&
			get_label(get_symedge(triangle.nxt).edge) != 3 &&
			get_label(prev_symedge(triangle).edge) != 3 &&
			(tri_seg_inters_index[triangle.face] > segment_index ||
				tri_seg_inters_index[triangle.face] == -1))
			tri_seg_inters_index[triangle.face] = segment_index;
	}

	bool GCMesh::will_be_flipped(int segment_index, SymEdge triangle)
	{
		if (tri_seg_inters_index[triangle.face] == segment_index && tri_seg_inters_index[sym_symedge(triangle).face] == segment_index && edge_label[triangle.edge] < 3)
		{
			edge_label[triangle.edge] = 2;
			return true;
		}

		return false;
	}

	bool GCMesh::pre_candidate_check(SymEdge s)
	{
		vec2 p1 = get_vertex(get_symedge(get_symedge(s.nxt).nxt).vertex);
		vec2 e1 = get_vertex(s.vertex);
		vec2 e2 = get_vertex(get_symedge(s.nxt).vertex);
		vec2 p2 = get_vertex(get_symedge(get_symedge(sym_symedge(s).nxt).nxt).vertex);
		return polygonal_is_strictly_convex(4, p1, e1, p2, e2);
	}

	bool GCMesh::polygonal_is_strictly_convex(int num, vec2 p1, vec2 p2, vec2 p3, vec2 p4, vec2 p5)
	{
		// Definition of a stricly convex set from wikipedia
		// https://en.wikipedia.org/wiki/Convex_set

		// Let S be a vector space over the real numbers, or, more generally, over some ordered field. This includes Euclidean spaces. 
		// A set C in S is said to be convex if, for all x and y in C and all t in the interval (0, 1), the point (1 − t)x + ty also belongs to C.
		// In other words, every point on the line segment connecting x and y is in C
		// This implies that a convex set in a real or complex topological vector space is path-connected, thus connected.
		// Furthermore, C is strictly convex if every point on the line segment connecting x and y other than the endpoints is inside the interior of C.

		// My made up solution
		// Definition of a stricly convex set from wikipedia
		// https://en.wikipedia.org/wiki/Convex_set

		// Let S be a vector space over the real numbers, or, more generally, over some ordered field. This includes Euclidean spaces. 
		// A set C in S is said to be convex if, for all x and y in C and all t in the interval (0, 1), the point (1 − t)x + ty also belongs to C.
		// In other words, every point on the line segment connecting x and y is in C
		// This implies that a convex set in a real or complex topological vector space is path-connected, thus connected.
		// Furthermore, C is strictly convex if every point on the line segment connecting x and y other than the endpoints is inside the interior of C.

		// My made up solution
		const vec2 point_array[5] = { p1, p2, p3, p4, p5 };

		for (int i = 0; i < num; i++)
		{
			vec2 line = point_array[(i + 1) % num] - point_array[i];

			// rotate vector by 90 degrees
			{
				float tmp = line.x;
				line.x = line.y;
				line.y = -tmp;
			}

			for (int j = 0; j < num - 2; j++)
			{
				if (!check_side(line, point_array[(i + 2 + j) % num] - point_array[(i + 1) % num]))
					return false;
			}
		}
		return true;
	}

	bool GCMesh::check_side(vec2 direction, vec2 other)
	{
		return dot(direction, other) >= 0.f ? true : false;
	}

	bool GCMesh::Qi_check(int segment_index, SymEdge ei1, SymEdge ei)
	{
		vec2 s1 = get_vertex(seg_endpoint_indices[segment_index].x);
		vec2 s2 = get_vertex(seg_endpoint_indices[segment_index].y);

		vec2 vertices[5];

		vertices[0] = get_vertex(prev_symedge(ei1).vertex);
		bool edges_connected = ei1.vertex == ei.vertex;
		if (edges_connected)
		{
			vertices[1] = get_vertex(ei.vertex);
			vertices[2] = get_vertex(prev_symedge(sym_symedge(ei)).vertex);
			vertices[3] = get_vertex(get_symedge(ei.nxt).vertex);
			vertices[4] = get_vertex(get_symedge(ei1.nxt).vertex);
		}
		else
		{
			vertices[1] = get_vertex(ei1.vertex);
			vertices[2] = get_vertex(ei.vertex);
			vertices[3] = get_vertex(prev_symedge(sym_symedge(ei)).vertex);
			vertices[4] = get_vertex(get_symedge(ei.nxt).vertex);
		}

		if (polygonal_is_strictly_convex(4, vertices[1], vertices[2], vertices[3], vertices[4]))
		{
			if (first_candidate_check(s1, s2, ei) ||
				second_candidate_check(vertices[0], vertices[1], vertices[2], vertices[3], vertices[4]) ||
				third_candidate_check(edges_connected, vertices[0], vertices[1], vertices[2], vertices[3], vertices[4]))
			{
				return true;
			}
			return false;
		}
		return false;
	}

	bool GCMesh::first_candidate_check(vec2 s1, vec2 s2, SymEdge ei)
	{
		vec2 p1 = get_vertex(prev_symedge(ei).vertex);
		vec2 e1 = get_vertex(ei.vertex);
		vec2 e2 = get_vertex(get_symedge(ei.nxt).vertex);
		vec2 p2 = get_vertex(prev_symedge(sym_symedge(ei)).vertex);

		bool intersects_first = segment_triangle_test(s1, s2, p1, e1, p2);
		bool intersects_second = segment_triangle_test(s1, s2, p1, p2, e2);
		return intersects_first ^ intersects_second;
	}

	bool GCMesh::second_candidate_check(vec2 p1, vec2 p2, vec2 p3, vec2 p4, vec2 p5)
	{
		// input parameters forms a pentagonal polygonal
		return polygonal_is_strictly_convex(5, p1, p2, p3, p4, p5);
	}

	bool GCMesh::third_candidate_check(bool edges_connected, vec2 p1, vec2 p2, vec2 p3, vec2 p4, vec2 p5)
	{
		if (edges_connected)
			return !polygonal_is_strictly_convex(4, p1, p2, p4, p5) && polygonal_is_strictly_convex(4, p1, p2, p3, p5) == true ? true : false;
		else
			return !polygonal_is_strictly_convex(4, p1, p2, p3, p5) && polygonal_is_strictly_convex(4, p1, p2, p4, p5) == true ? true : false;
	}
	bool GCMesh::is_delaunay(int sym)
	{
		int index = rot(nxt(sym));

		if (index != -1)
		{
			vec2 face_vertices[3];
			face_vertices[0] = get_vertex(get_symedge(sym).vertex);
			face_vertices[1] = get_vertex(get_symedge(nxt(sym)).vertex);
			face_vertices[2] = get_vertex(get_symedge(prev(sym)).vertex);

			vec2 ab = face_vertices[1] - face_vertices[0];
			vec2 bc = face_vertices[2] - face_vertices[1];

			vec2 mid_point1 = face_vertices[0] + ab / 2.f;
			vec2 mid_point2 = face_vertices[1] + bc / 2.f;

			// rotate vectors 90 degrees
			vec2 normal1 = vec2(-ab.y, ab.x);
			vec2 normal2 = vec2(-bc.y, bc.x);

			bool degenerate_triangle;
			vec2 circle_center = line_line_intersection_point(mid_point1, mid_point1 + normal1, mid_point2, mid_point2 + normal2, degenerate_triangle);
			if (degenerate_triangle == true)
				return false;

			vec2 other = get_vertex(get_symedge(prev(index)).vertex);
			if (length(circle_center - other) < length(face_vertices[0] - circle_center) - EPSILON)
				return false;
		}
		return true;
	}

	void GCMesh::set_quad_edges_label(int label, SymEdge edge)
	{
		edge_label[nxt(edge).edge] = edge_is_constrained[nxt(edge).edge] == -1 ? max(label, edge_label[nxt(edge).edge]) : edge_label[nxt(edge).edge];
		edge_label[prev(edge).edge] = edge_is_constrained[prev(edge).edge] == -1 ? max(label, edge_label[prev(edge).edge]) : edge_label[prev(edge).edge];

		edge = sym(edge);

		edge_label[nxt(edge).edge] = edge_is_constrained[nxt(edge).edge] == -1 ? max(label, edge_label[nxt(edge).edge]) : edge_label[nxt(edge).edge];
		edge_label[prev(edge).edge] = edge_is_constrained[prev(edge).edge] == -1 ? max(label, edge_label[prev(edge).edge]) : edge_label[prev(edge).edge];
	}

	void GCMesh::flip_edge(SymEdge edge)
	{
		// flips clockwise according to figure 8 in the paper.

		int t_prim = edge.face;
		int t = sym(edge).face;

		int curr = get_index(edge);
		int curr_sym = get_index(sym(edge));

		int e1 = edge.nxt;
		int e2 = nxt(edge).nxt;
		int e3 = sym(edge).nxt;
		int e4 = nxt(sym(edge)).nxt;

		int e1_sym = nxt(get_symedge(e1)).rot;
		int e2_sym = nxt(get_symedge(e2)).rot;
		int e3_sym = nxt(get_symedge(e3)).rot;
		int e4_sym = nxt(get_symedge(e4)).rot;

		//e1
		sym_edges[e1].nxt = curr;
		sym_edges[e1].rot = e4_sym;

		sym_edges[e1].face = t_prim;

		//e2
		sym_edges[e2].nxt = e3;
		sym_edges[e2].rot = curr;

		sym_edges[e2].face = t;

		//e3
		sym_edges[e3].nxt = curr_sym;
		sym_edges[e3].rot = e2_sym;

		sym_edges[e3].face = t;

		//e4
		sym_edges[e4].nxt = e1;
		sym_edges[e4].rot = curr_sym;

		sym_edges[e4].face = t_prim;

		// curr
		sym_edges[curr].nxt = e4;
		sym_edges[curr].rot = e1_sym;

		sym_edges[curr].vertex = get_symedge(e2).vertex;
		sym_edges[curr].face = t_prim;

		// curr_sym
		sym_edges[curr_sym].nxt = e2;
		sym_edges[curr_sym].rot = e3_sym;

		sym_edges[curr_sym].vertex = get_symedge(e4).vertex;
		sym_edges[curr_sym].face = t;

		// update face symedges
		tri_symedges[t_prim] = ivec4(curr, e4, e1, -1);
		tri_symedges[t] = ivec4(curr_sym, e2, e3, -1);

		// reset
		tri_seg_inters_index[t_prim] = -1;
		tri_seg_inters_index[t] = -1;
	}

	int GCMesh::find_closest_constraint(vec2 a, vec2 b, vec2 c)
	{
		float dist = FLT_MAX;
		int ret = -2;
		for (int i = 0; i < seg_endpoint_indices.size(); i++)
		{
			ivec2 seg_i = seg_endpoint_indices[i];
			//if (seg_i.x < 0 || seg_i.y < 0)
			//	continue;
			vec2 s[2];
			s[0] = point_positions[seg_i[0]];
			s[1] = point_positions[seg_i[1]];
			if (possible_disturbance(a, b, c, s))
			{
				bool projectable;
				vec2 b_prim = project_point_on_segment(b, s[0], s[1], projectable);
				if (projectable)
				{
					float b_dist = length(b_prim - b);
					if (b_dist < dist && !(point_equal(b_prim, a) || point_equal(b_prim, c)))
					{
						dist = b_dist;
						ret = i;
					}
				}
			}
		}
		return ret;
	}

	int GCMesh::find_closest_constraint_v2(int ac_sym, vec2 a, vec2 b, vec2 c)
	{
		float dist = FLT_MAX;
		int ret = -1;
		// list containing visited triangles
		int face_list[CONSTRAINT_TRI_LIST_SIZE];
		face_list[0] = sym_edges[ac_sym].face;
		int face_list_size = 1;
		// create stack needed for traversal
		int sym_stack[CONSTRAINT_STACK_SIZE];
		int top = 0;
		sym_stack[top] = sym(ac_sym);
		if (sym_stack[top] != -1)
		{
			while (top > -1)
			{
				//pop symedge from stack
				int curr_e = sym_stack[top];
				top--;
				// explore curr_e
				for (int i = 0; i < 2; i++)
				{
					curr_e = nxt(curr_e);
					// first check if edge is a possible disturbance
					vec2 s[2];
					s[0] = point_positions[sym_edges[curr_e].vertex];
					s[1] = point_positions[sym_edges[nxt(curr_e)].vertex];
					if (possible_disturbance(a, b, c, s))
					{
						// check if edge is constrained
						if (edge_is_constrained[sym_edges[curr_e].edge] > -1)
						{
							bool projectable;
							vec2 b_prim = project_point_on_segment(b, s[0], s[1], projectable);
							if (projectable)
							{
								float b_dist = length(b_prim - b);
								if (b_dist < dist && !(point_equal(b_prim, a) || point_equal(b_prim, c)))
								{
									dist = b_dist;
									ret = curr_e;
								}
							}
						}
						else // otherwise we check if we should continue exploring in the edges direction
						{
							int sym_e = sym(curr_e);
							if (sym_e > -1)
							{
								int face_e = sym_edges[sym_e].face;
								// first check so face is not in the face list
								bool new_triangle = true;
								for (int j = 0; j < face_list_size; j++)
								{
									if (face_list[j] == face_e)
										new_triangle = false;
								}
								// check so list still has room.
								if (face_list_size < CONSTRAINT_TRI_LIST_SIZE)
								{
									face_list[face_list_size] = face_e;
									face_list_size++;
								}
								else 
								{
									find_dist_status.const_list_status = 1;
									return -1;
								}
								// check so the triangle on other side of edge has not yet been explored.
								top++;
								if(top < CONSTRAINT_STACK_SIZE)
								{ 
									largest_stack = max(top, largest_stack);
									sym_stack[top] = sym_e;
								}
								else
								{
									find_dist_status.const_queue_status = 1;
									return -1;
								}
							}
						}
					}
				}
			}
		}
		return ret;
	}

	bool GCMesh::possible_disturbance(vec2 a, vec2 b, vec2 c, vec2 s[2])
	{
		// first check if b is projectable on ac
		bool projectable;
		project_point_on_segment(b, a, c, projectable);
		if (projectable)
		{
			float len_ab = distance(a, b);
			float len_bc = distance(b, c);
			// if edge bc is shorter than ab then swap a and b
			if (len_ab > len_bc)
			{
				vec2 tmp = a;
				a = c;
				c = tmp;
			}

			vec2 sector_c = project_point_on_line(b, a, c);
			float dist = 2.f * length(sector_c - a);
			sector_c = a + normalize(c - a) * dist;
			if (edge_intersects_sector(a, b, sector_c, s))
				return true;
			vec2 p = get_symmetrical_corner(a, b, c);
			sector_c = c + normalize(a - c) * dist;

			if (edge_intersects_sector(sector_c, p, c, s))
				return true;
		}
		return false;
	}

	int GCMesh::find_segment_symedge(int start, int segment)
	{
		std::array<vec2, 2> seg_p = get_segment(segment);
		vec2 goal = (seg_p[0] + seg_p[1]) / 2.0f;
		bool on_edge;
		oriented_walk_edge(start, goal, on_edge);
		if (on_edge)
			return start;
		else
			return -1;
	}

	void GCMesh::oriented_walk_edge(int & curr_e, vec2 point, bool & on_edge)
	{
		bool done = false;
		vec2 goal = point;
		int iter = 0;
		on_edge = false;
		while (!done) {
			on_edge = false;
			// Loop through triangles edges to check if point is on the edge 
			for (int i = 0; i < 3; i++)
			{
				bool hit;
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
			vec2 tri_cent;
			tri_cent = (tri_points[0] + tri_points[1] + tri_points[2]) / 3.0f;
			// Loop through edges to see if we should continue through the edge
			// to the neighbouring triangle 
			bool line_line_hit = false;
			for (int i = 0; i < 3; i++)
			{
				line_line_hit = line_line_test(
					tri_cent,
					goal,
					point_positions[sym_edges[curr_e].vertex],
					point_positions[sym_edges[sym_edges[curr_e].nxt].vertex]
				);

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

	int GCMesh::find_constraint_disturbance(int constraint_sym_e, int edge_ac, bool right)
	{
		vec2 R[3];
		std::array<vec2, 2> s = get_edge(constraint_sym_e);
		vec2 a;
		// Set variables needed to calculate R
		if (right)
		{
			R[0] = point_positions[sym_edges[edge_ac].vertex];
			R[1] = point_positions[sym_edges[prev(edge_ac)].vertex];
			a = point_positions[sym_edges[nxt(edge_ac)].vertex];
		}
		else {
			R[0] = point_positions[sym_edges[nxt(edge_ac)].vertex];
			R[1] = point_positions[sym_edges[prev(edge_ac)].vertex];
			a = point_positions[sym_edges[edge_ac].vertex];
		}
		// Calculate R
		vec2 dir = normalize(s[0] - s[1]);
		//vec2 ac = R[0] - a;
		//vec2 dir = normalize(ac);
		//vec2 ab = R[1] - a;
		float c_prim = dot(dir, R[0]);
		float b_prim = dot(dir, R[1]);
		R[2] = R[1] + (dir * (c_prim - b_prim));
		// Loop through points trying to find disturbance to current traversal
		float best_dist = FLT_MAX;
		int first_disturb = -1;
		float best_dist_b = 0.0f;
		std::array<vec2, 3> tri;
		get_face(sym_edges[edge_ac].face, tri);
		for (int i = 0; i < point_positions.size(); i++)
		{
			vec2 point = point_positions[i];

			// check if point is inside of R
			if (point_triangle_test(point, R))
			{
				// If point is one of the triangles point continue to next point
				if (point_equal_tri_vert(point, tri) > -1)
					continue;
				// TODO: Change oriented walk to start from last point instead of the constraint
				int v_edge = oriented_walk_point(constraint_sym_e, i);
				float dist = is_disturbed(constraint_sym_e, prev(edge_ac), v_edge);
				if (dist > 0.0f)
				{
					if (dist < best_dist)
					{
						first_disturb = v_edge;
						best_dist = dist;
						best_dist_b = distance(point_positions[sym_edges[v_edge].vertex],
							point_positions[sym_edges[prev(edge_ac)].vertex]);
					}
					else if (dist < (best_dist + EPSILON))
					{
						// if new point has the same distance as the previous one
						// check if it is closer to b
						float dist_b = distance(point_positions[sym_edges[v_edge].vertex],
							point_positions[sym_edges[prev(edge_ac)].vertex]);
						if (dist_b < best_dist_b)
						{
							first_disturb = v_edge;
							best_dist = dist;
							best_dist_b = dist_b;
						}
					}
				}

			}
		}

		return first_disturb;
	}

	int GCMesh::find_constraint_disturbance_v2(int constraint_sym_e, int edge_ac, bool right)
	{
		vec2 R[3];
		std::array<vec2, 2> s = get_edge(constraint_sym_e);
		vec2 a;
		int first_edge;
		int edge_bc = prev(edge_ac);
		// Set variables needed to calculate R
		if (right)
		{
			R[0] = point_positions[sym_edges[edge_ac].vertex];
			R[1] = point_positions[sym_edges[prev(edge_ac)].vertex];
			a = point_positions[sym_edges[nxt(edge_ac)].vertex];
			first_edge = sym(prev(edge_ac));
		}
		else {
			R[0] = point_positions[sym_edges[nxt(edge_ac)].vertex];
			R[1] = point_positions[sym_edges[prev(edge_ac)].vertex];
			a = point_positions[sym_edges[edge_ac].vertex];
			first_edge = sym(nxt(edge_ac));
		}
		// Calculate R
		vec2 dir = normalize(s[0] - s[1]);
		//vec2 ac = R[0] - a;
		//vec2 dir = normalize(ac);
		//vec2 ab = R[1] - a;
		float c_prim = dot(dir, R[0]);
		float b_prim = dot(dir, R[1]);
		R[2] = R[1] + (dir * (c_prim - b_prim));
		int first_disturb = -1;
		if (!point_equal(R[2], R[0]))
		{
			// Loop through points trying to find disturbance to current traversal
			float best_dist = FLT_MAX;
			float best_dist_b = 0.0f;
			std::array<vec2, 3> tri;
			get_face(sym_edges[edge_ac].face, tri);
			// find disturbance points
			int face_list[DISTURBANCE_TRI_LIST_SIZE];
			face_list[0] = sym_edges[edge_ac].face;
			int face_list_size = 1;
			int sym_stack[DISTURBANCE_STACK_SIZE];
			int top = 0;
			sym_stack[top] = first_edge;
			if (sym_stack[top] != -1)
			{
				while (top > -1)
				{
					//pop symedge from stack
					int curr_e = sym_stack[top];
					top--;
					// check if next point is a disturbance.
					int v_edge = prev(curr_e);
					if (point_triangle_test(point_positions[sym_edges[v_edge].vertex], R[0], R[1], R[2]))
					{
						float dist = is_disturbed(constraint_sym_e, edge_bc, v_edge);
						if (dist > 0.0f)
						{
							if (dist < best_dist)
							{
								first_disturb = v_edge;
								best_dist = dist;
								best_dist_b = distance(point_positions[sym_edges[v_edge].vertex],
									point_positions[sym_edges[prev(edge_ac)].vertex]);
							}
							else if (dist < (best_dist + EPSILON))
							{
								// if new point has the same distance as the previous one
								// check if it is closer to b
								float dist_b = distance(point_positions[sym_edges[v_edge].vertex],
									point_positions[sym_edges[prev(edge_ac)].vertex]);
								if (dist_b < best_dist_b)
								{
									first_disturb = v_edge;
									best_dist = dist;
									best_dist_b = dist_b;
								}
							}
						}
					}
					// explore which of the edges 
					for (int i = 0; i < 2; i++)
					{
						curr_e = nxt(curr_e);
						// first check if edge is a possible disturbance
						vec2 s[2];
						s[0] = point_positions[sym_edges[curr_e].vertex];
						s[1] = point_positions[sym_edges[nxt(curr_e)].vertex];
						if (edge_is_constrained[sym_edges[curr_e].edge] < 0 && segment_triangle_test(s[0], s[1], R[0], R[1], R[2]))
						{
							int sym_e = sym(curr_e);
							if (sym_e > -1)
							{
								bool new_triangle = true;
								int face_e = sym_edges[sym_e].face;
								// first check if face has been explored before
								for (int j = 0; j < face_list_size; j++)
								{
									if (face_list[j] == face_e)
										new_triangle = false;
								}

								if (new_triangle)
								{
									// check so list still has room.
									
									if (face_list_size < DISTURBANCE_TRI_LIST_SIZE)
									{
										face_list[face_list_size] = face_e;
										face_list_size++;
									}
									else 
									{
										find_dist_status.dist_list_status = 1;
										return first_disturb;
									}
									// check so the stack still has empty room left
									top++;
									if (top < DISTURBANCE_STACK_SIZE)
									{
										largest_stack = max(top, largest_stack);
										sym_stack[top] = sym_e;
									}
									else
									{
										find_dist_status.dist_queue_status = 1;
										return first_disturb;
									}
								}
							}
						}
					}
				}
			}
		}
		return first_disturb;
	}

	float GCMesh::is_disturbed(int constraint, int b_sym, int & v_sym)
	{
		// 1
		if (!no_collinear_constraints(v_sym))
			return -1.0f;

		// 2
		vec2 v = point_positions[sym_edges[v_sym].vertex];
		vec2 a = point_positions[sym_edges[prev(b_sym)].vertex];
		vec2 b = point_positions[sym_edges[b_sym].vertex];
		vec2 c = point_positions[sym_edges[nxt(b_sym)].vertex];

		if (!is_orthogonally_projectable(v, a, c))
			return -1.0f;

		// 3
		std::array<vec2, 2> c_endpoints = get_edge(constraint);
		vec2 v_prim = project_point_on_line(v, c_endpoints[0], c_endpoints[1]);
		if (!(line_line_test(v, v_prim, a, c) && line_line_test(v, v_prim, b, c)))
			return -1.0f;

		// 4
		float dist_v_segment = length(v_prim - v);
		if (!(dist_v_segment < local_clearance(b, c_endpoints)))
			return -1.0f;

		// 5 
		vec2 e = find_e_point(v_sym, v, v_prim);
		if (!(dist_v_segment < length(v - e)))
			return -1.0f;

		return dist_v_segment;
	}

	bool GCMesh::no_collinear_constraints(int v)
	{
		int edge;
		bool reverse = false;
		int curr_constraint = v;
		int last_constraint = -1;
		vec2 point = point_positions[sym_edges[v].vertex];
		// first check if initial value is an constraint
		if (edge_is_constrained[sym_edges[v].edge] != -1)
		{
			curr_constraint = v;
		}
		else {
			curr_constraint = find_next_rot_constraint(v, v, reverse);
		}
		while (curr_constraint != -1)
		{
			vec2 curr_vec = normalize(point_positions[sym_edges[nxt(curr_constraint)].vertex] - point);
			// explore if current constraint is collinear with another constraint
			bool explore_rd = reverse;
			edge = find_next_rot_constraint(v, curr_constraint, explore_rd);
			while (edge != -1)
			{

				//if it is a constraint check if it is collinear with curr_constraint
				vec2 other_vec = normalize(point_positions[sym_edges[nxt(edge)].vertex] - point);
				if (dot(curr_vec, other_vec) < -1 + EPSILON)
				{
					return false;
				}
				// find next constraint
				edge = find_next_rot_constraint(v, edge, explore_rd);
			}

			// find next constraint to explore
			curr_constraint = find_next_rot_constraint(v, curr_constraint, reverse);
		}
		return true;
	}

	int GCMesh::find_next_rot(int start, int curr, bool & reverse)
	{
		int edge = curr;
		//Move to nxt edge to check if it is a constraint
		if (reverse) // reverse
		{
			edge = crot(edge);
		}
		else // forward
		{
			edge = rot(edge);
			if (edge == start)
			{
				return -1;
			}
			else if (edge == -1)
			{
				reverse = true;
				edge = start;
				edge = crot(edge);
			}
		}
		return edge;
	}

	int GCMesh::find_next_rot_constraint(int start, int curr, bool & reverse)
	{
		curr = find_next_rot(start, curr, reverse);
		while (curr != -1)
		{
			if (edge_is_constrained[sym_edges[curr].edge] != -1)
			{
				return curr;
			}
			curr = find_next_rot(start, curr, reverse);
		}
		return curr;
	}

	bool GCMesh::is_orthogonally_projectable(vec2 v, vec2 a, vec2 b)
	{
		vec2 line = b - a;
		vec2 projected_point = project_point_on_line(v, a, b);

		float projected_length = dot(normalize(line), projected_point - a);

		if (projected_length < 0.f || projected_length * projected_length > dot(line, line))
			return false;

		return true;
	}

	float GCMesh::local_clearance(vec2 b, std::array<vec2, 2>& segment)
	{
		vec2 b_prim = project_point_on_line(b, segment[0], segment[1]);
		return length(b - b_prim);
	}

	vec2 GCMesh::find_e_point(int & v_sym, vec2 v, vec2 v_prim)
	{
		vec2 e;
		int edge = v_sym;
		bool reverse = false;
		while (true)
		{
			// Check if edge leads to finding point e
			e = point_positions[sym_edges[nxt(edge)].vertex];
			vec2 d = point_positions[sym_edges[prev(edge)].vertex];
			if (line_line_test(v, v_prim, e, d))
			{
				v_sym = edge;
				return e;
			}

			// Move to next edge
			if (reverse)
			{
				edge = crot(edge);
				if (edge == -1)
				{
					break;
				}
			}
			else
			{
				edge = rot(edge);
				if (edge == v_sym)
				{
					break;
				}
				else if (edge == -1)
				{
					reverse = true;
					edge = v_sym;
				}
			}
		}
		// should not happen, error
		e = vec2(FLT_MAX);
		return e;
	}

	vec2 GCMesh::calculate_refinement(int c, int v_sym, bool & success)
	{
		std::array<vec2, 3> tri;
		get_face(sym_edges[v_sym].face, tri);
		vec2 circle_center = circle_center_from_points(tri[0], tri[1], tri[2]);
		float radius = distance(circle_center, tri[0]);
		std::array<vec2, 2> constraint_edge = get_edge(c);
		std::array<vec2, 2> inter_points = ray_circle_intersection(constraint_edge[1], constraint_edge[0], circle_center, radius, success);
		if (success)
		{
			return (inter_points[0] + inter_points[1]) / 2.0f;
		}
		else
		{
			return vec2(0.0f);
		}
	}


	bool GCMesh::adjacent_tri_point_intersects_edge(SymEdge curr_edge, int & face_index)
	{
		// checks if the adjacent triangle wants to insert its point in the provided edge

		// if there exists an adjacent triangle that wants to insert a point
		if (nxt(curr_edge).rot != -1 && tri_ins_point_index[sym(curr_edge).face] != -1)
		{
			SymEdge other_insertion_symedge = sym(curr_edge);

			vec2 other_point = get_vertex(tri_ins_point_index[other_insertion_symedge.face]);

			// Check if the adjacent triangle wants to insert into same edge, if true: let the triangle with the lowest index do its insertion
			if (point_line_test(other_point,
				get_vertex(other_insertion_symedge.vertex),
				get_vertex(nxt(other_insertion_symedge).vertex)))
			{
				face_index = other_insertion_symedge.face;
				return true;
			}
			else
			{
				face_index = -1;
				return false;
			}
		}
		else
		{
			face_index = -1;
			return false;
		}
	}
}