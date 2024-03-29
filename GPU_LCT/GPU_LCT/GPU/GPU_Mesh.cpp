#include "GPU_Mesh.hpp"
#include <fstream>

namespace GPU
{
	GPUMesh::GPUMesh()
	{
		setup_compute_shaders();
	}


	GPUMesh::~GPUMesh()
	{
	}

	GPUMesh& GPUMesh::operator=(GPUMesh& other)
	{
		if (!m_buffers_initated)
			initiate_buffers(other.m_scale);
		
		this->m_point_bufs.positions.clear();
		this->m_point_bufs.inserted.clear();
		this->m_point_bufs.tri_index.clear();

		this->m_edge_bufs.label.clear();
		this->m_edge_bufs.is_constrained.clear();

		this->m_segment_bufs.endpoint_indices.clear();
		this->m_segment_bufs.inserted.clear();

		this->m_triangle_bufs.symedge_indices.clear();
		this->m_triangle_bufs.ins_point_index.clear();
		this->m_triangle_bufs.seg_inters_index.clear();
		this->m_triangle_bufs.edge_flip_index.clear();

		this->m_new_points.clear();
		this->m_refine_points.clear();
		this->m_sym_edges.clear();
		// copy point data

		auto vec2_data = other.m_point_bufs.positions.get_buffer_data<glm::vec2>();
		this->m_point_bufs.positions.append_to_buffer(vec2_data);
		vec2_data.clear();

		auto int_data = other.m_point_bufs.inserted.get_buffer_data<int>();
		this->m_point_bufs.inserted.append_to_buffer(int_data);
		int_data.clear();

		int_data = other.m_point_bufs.tri_index.get_buffer_data<int>();
		this->m_point_bufs.tri_index.append_to_buffer(int_data);
		int_data.clear();

		// copy edge data

		int_data = other.m_edge_bufs.label.get_buffer_data<int>();
		this->m_edge_bufs.label.append_to_buffer(int_data);
		int_data.clear();

		int_data = other.m_edge_bufs.is_constrained.get_buffer_data<int>();
		this->m_edge_bufs.is_constrained.append_to_buffer(int_data);
		int_data.clear();

		// copy segment data

		auto ivec2_data = other.m_segment_bufs.endpoint_indices.get_buffer_data<glm::ivec2>();
		this->m_segment_bufs.endpoint_indices.append_to_buffer(ivec2_data);
		ivec2_data.clear();

		int_data = other.m_segment_bufs.inserted.get_buffer_data<int>();
		this->m_segment_bufs.inserted.append_to_buffer(int_data);
		int_data.clear();

		// copy triangle data

		auto ivec4_data = other.m_triangle_bufs.symedge_indices.get_buffer_data<glm::ivec4>();
		this->m_triangle_bufs.symedge_indices.append_to_buffer(ivec4_data);
		ivec4_data.clear();

		int_data = other.m_triangle_bufs.ins_point_index.get_buffer_data<int>();
		this->m_triangle_bufs.ins_point_index.append_to_buffer(int_data);
		int_data.clear();

		int_data = other.m_triangle_bufs.seg_inters_index.get_buffer_data<int>();
		this->m_triangle_bufs.seg_inters_index.append_to_buffer(int_data);
		int_data.clear();

		int_data = other.m_triangle_bufs.edge_flip_index.get_buffer_data<int>();
		this->m_triangle_bufs.edge_flip_index.append_to_buffer(int_data);
		int_data.clear();

		// copy misc data

		vec2_data = other.m_new_points.get_buffer_data<glm::vec2>();
		this->m_new_points.append_to_buffer(vec2_data);
		vec2_data.clear();

		auto new_point_data = other.m_refine_points.get_buffer_data<NewPoint>();
		this->m_refine_points.append_to_buffer(new_point_data);
		new_point_data.clear();

		auto sym_data = other.m_sym_edges.get_buffer_data<SymEdge>();
		this->m_sym_edges.append_to_buffer(sym_data);
		sym_data.clear();

		m_nr_of_symedges.update_buffer<int>({ m_sym_edges.element_count() });
		m_status.update_buffer<int>({ 0 });

		return *this;
	}

	void GPUMesh::initiate_buffers(glm::vec2 scale)
	{
		m_scale = scale;
		m_buffers_initated = true;
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
		edge_constraints[0] = 0;
		edge_constraints[1] = 1;
		edge_constraints[2] = 2;
		edge_constraints[3] = 3;
		edge_constraints[4] = -1;

		m_edge_bufs.is_constrained.create_buffer(type, edge_constraints, usage, 4, n);

		// Fill constraint buffers
		std::vector<glm::ivec2> starting_contraints = { {0, 1}, {1, 2}, {2, 3}, {3, 0} };
		m_segment_bufs.endpoint_indices.create_buffer(type, starting_contraints, usage, 5, n);
		m_segment_bufs.inserted.create_buffer(type, std::vector<int>(4, 1), usage, 6, n);

		std::vector<glm::ivec4> sym_edge_indices;
		sym_edge_indices.push_back({ 0, 1, 2, -1 });
		sym_edge_indices.push_back({ 3, 4 ,5, -1 });
		m_triangle_bufs.symedge_indices.create_buffer(type, sym_edge_indices, usage, 7, n);

		m_triangle_bufs.ins_point_index.create_buffer(type, std::vector<int>(2, -1), usage, 8, n);
		m_triangle_bufs.seg_inters_index.create_buffer(type, std::vector<int>(2, -1), usage, 9, n);
		m_triangle_bufs.edge_flip_index.create_buffer(type, std::vector<int>(2, -1), usage, 10, n);


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

		m_refine_points.create_buffer(type, std::vector<NewPoint>(sym_edges.size()), usage, 13, n);

		m_new_points.create_buffer(type, std::vector<glm::vec2>(0), usage, 14, 100);


		m_sym_edges.create_buffer(type, sym_edges, usage, 11, n);

		m_nr_of_symedges.create_uniform_buffer<int>({ m_sym_edges.element_count() }, usage);

		m_epsilon_buff.create_uniform_buffer<float>({ m_epsilon }, usage, 1);

		m_status.create_buffer(type, std::vector<int>(1, 1), GL_DYNAMIC_DRAW, 12, 1);

		m_find_dist_status.create_buffer(type, std::vector<Find_Disturbance_Status>(1, { 0,0,0,0 }), GL_DYNAMIC_DRAW, 15, 1);

		m_version_buff.create_uniform_buffer<int>({ version }, usage, 2);

		m_semaphores.create_buffer(type, std::vector<GLuint>(2, 0), usage, 16);
	}

	void GPUMesh::add_frame_points(std::vector<glm::vec2> points)
	{
		// add the the points without duplicates to the point buffers
		auto potential_points = m_point_bufs.positions.get_buffer_data<glm::vec2>();
		int num_old_points = potential_points.size();
		potential_points.insert(potential_points.end(), points.begin(), points.end());
		remove_duplicate_points(potential_points);
		points.clear();
		points.insert(points.begin(), potential_points.begin() + num_old_points, potential_points.end());

		int num_new_points = points.size();

		m_point_bufs.positions.append_to_buffer(points);
		m_point_bufs.inserted.append_to_buffer(std::vector<int>(num_new_points, 0));
		m_point_bufs.tri_index.append_to_buffer(std::vector<int>(num_new_points, 0));


		// increase sizes of arrays, 
		// based on how many new points are inserted
		// segments
		int num_new_tri = num_new_points * 2;
		int num_new_segs = num_new_points;

		// fix new sizes of triangle buffers
		m_triangle_bufs.symedge_indices.append_to_buffer(std::vector<glm::ivec4>(num_new_tri, { -1, -1, -1, -1 }));
		m_triangle_bufs.ins_point_index.append_to_buffer(std::vector<int>(num_new_tri, -1));
		m_triangle_bufs.edge_flip_index.append_to_buffer(std::vector<int>(num_new_tri, -1));
		m_triangle_bufs.seg_inters_index.append_to_buffer(std::vector<int>(num_new_tri, -1));


		// fix new size of segment buffers
		m_segment_bufs.endpoint_indices.append_to_buffer(std::vector<glm::ivec2>(num_new_segs));
		m_segment_bufs.inserted.append_to_buffer(std::vector<int>(num_new_segs));
		// fix new sizes of edge buffers 
		// TODO: fix so it can handle repeated insertions
		int num_new_edges = num_new_points * 3;
		m_edge_bufs.is_constrained.append_to_buffer(std::vector<int>(num_new_edges, -1));
		m_edge_bufs.label.append_to_buffer(std::vector<int>(num_new_edges, -1));
		// fix new size of symedges buffer
		// TODO: fix so it can handle repeated insertions
		int num_new_sym_edges = num_new_points * 6;
		m_sym_edges.append_to_buffer(std::vector<SymEdge>(num_new_sym_edges));
		m_refine_points.append_to_buffer(std::vector<NewPoint>(num_new_sym_edges));
		// TODO, maybe need to check if triangle buffers needs to grow
		// resize new points array

		m_nr_of_symedges.update_buffer<int>({ m_sym_edges.element_count() });
		m_semaphores.append_to_buffer(std::vector<GLuint>(num_new_tri, 0));

		// then rebind the buffers that has been changed
		m_point_bufs.positions.bind_buffer();
		m_point_bufs.inserted.bind_buffer();
		m_point_bufs.tri_index.bind_buffer();

		m_edge_bufs.label.bind_buffer();
		m_edge_bufs.is_constrained.bind_buffer();

		m_segment_bufs.endpoint_indices.bind_buffer();
		m_segment_bufs.inserted.bind_buffer();

		m_triangle_bufs.symedge_indices.bind_buffer();
		m_triangle_bufs.ins_point_index.bind_buffer();
		m_triangle_bufs.seg_inters_index.bind_buffer();
		m_triangle_bufs.edge_flip_index.bind_buffer();
		m_refine_points.bind_buffer();


		m_sym_edges.bind_buffer();
		m_nr_of_symedges.bind_buffer();
		m_epsilon_buff.bind_buffer();
		m_semaphores.bind_buffer();
		m_status.bind_buffer();
		m_version_buff.bind_buffer();

		// Perform insertion of points untill all has been inserted 
		// and triangulation is CDT
		int counter = 0;
		int cont = 1;
		do
		{
			m_status.update_buffer<int>({ 0 });
			counter++;
			//// Find out which triangle the point is on the edge of
			glUseProgram(m_locate_point_triangle_program);
			glDispatchCompute((GLuint)256, 1, 1);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);

			glUseProgram(m_validate_edges_program);
			glDispatchCompute((GLuint)256, 1, 1);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);

			// Insert point into the edge
			glUseProgram(m_insert_in_edge_program);
			glDispatchCompute((GLuint)256, 1, 1);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);

			// Perform marking
			glUseProgram(m_marking_part_two_program);
			glDispatchCompute((GLuint)256, 1, 1);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);

			//Perform flipping to ensure that mesh is CDT
			glUseProgram(m_flip_edges_part_one_program);
			glDispatchCompute((GLuint)256, 1, 1);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);

			glUseProgram(m_flip_edges_part_two_program);
			glDispatchCompute((GLuint)256, 1, 1);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);

			glUseProgram(m_flip_edges_part_three_program);
			glDispatchCompute((GLuint)256, 1, 1);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);
			cont = m_status.get_buffer_data<int>()[0];
		} while (cont == 1);
		auto point_data_inserted = m_point_bufs.inserted.get_buffer_data<int>();
		//LOG(std::string("Frame insertion number of iterations: ") + std::to_string(counter));
}

	long long GPUMesh::build_CDT(std::vector<glm::vec2> points, std::vector<glm::ivec2> segments)
	{
		int num_old_points = m_point_bufs.positions.element_count();
		m_point_bufs.positions.append_to_buffer(points);
		m_point_bufs.inserted.append_to_buffer(std::vector<int>(points.size(), 0));
		m_point_bufs.tri_index.append_to_buffer(std::vector<int>(points.size(), 0));

		for (auto& segment : segments)
			segment = segment + glm::ivec2(num_old_points);
		m_segment_bufs.endpoint_indices.append_to_buffer(segments);
		m_segment_bufs.inserted.append_to_buffer(std::vector<int>(segments.size(), 0));
		// uppdating ubo containing sizes

		int num_new_tri = points.size() * 2;
		int num_new_segs = segments.size();

		// fix new sizes of triangle buffers
		m_triangle_bufs.symedge_indices.append_to_buffer(std::vector<glm::ivec4>(num_new_tri, { -1, -1, -1, -1 }));
		m_triangle_bufs.ins_point_index.append_to_buffer(std::vector<int>(num_new_tri, -1));
		m_triangle_bufs.edge_flip_index.append_to_buffer(std::vector<int>(num_new_tri, -1));
		m_triangle_bufs.seg_inters_index.append_to_buffer(std::vector<int>(num_new_tri, -1));


		// fix new sizes of edge buffers 
		// TODO: fix so it can handle repeated insertions
		int num_new_edges = points.size() * 3;
		m_edge_bufs.is_constrained.append_to_buffer(std::vector<int>(num_new_edges, -1));
		m_edge_bufs.label.append_to_buffer(std::vector<int>(num_new_edges, -1));
		// fix new size of symedges buffer
		// TODO: fix so it can handle repeated insertions
		int num_new_sym_edges = points.size() * 6;
		m_sym_edges.append_to_buffer(std::vector<SymEdge>(num_new_sym_edges));
		m_refine_points.append_to_buffer(std::vector<NewPoint>(num_new_sym_edges));
		// TODO, maybe need to check if triangle buffers needs to grow

		m_nr_of_symedges.update_buffer<int>({ m_sym_edges.element_count() });

		m_semaphores.append_to_buffer(std::vector<GLuint>(num_new_tri, 0));
		
		// Bind all ssbo's
		m_point_bufs.positions.bind_buffer();
		m_point_bufs.inserted.bind_buffer();
		m_point_bufs.tri_index.bind_buffer();

		m_edge_bufs.label.bind_buffer();
		m_edge_bufs.is_constrained.bind_buffer();

		m_segment_bufs.endpoint_indices.bind_buffer();
		m_segment_bufs.inserted.bind_buffer();

		m_triangle_bufs.symedge_indices.bind_buffer();
		m_triangle_bufs.ins_point_index.bind_buffer();
		m_triangle_bufs.seg_inters_index.bind_buffer();
		m_triangle_bufs.edge_flip_index.bind_buffer();
		m_refine_points.bind_buffer();

		m_sym_edges.bind_buffer();
		m_nr_of_symedges.bind_buffer();
		m_epsilon_buff.bind_buffer();
		m_semaphores.bind_buffer();

		m_status.bind_buffer();
		int counter = 0;

		Timer timer;
		timer.start();
		int cont = 1;
		while (cont)
		{
			counter++;
			m_status.update_buffer<int>({ 0 });

			// Locate Step
			glUseProgram(m_location_program);
			glDispatchCompute((GLuint)256, 1, 1);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);

			/*glUseProgram(m_location_tri_program);
			glDispatchCompute((GLuint)256, 1, 1);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);*/

			// Insert Step
			glUseProgram(m_insertion_program);
			glDispatchCompute((GLuint)256, 1, 1);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);

			// Marking Step
			glUseProgram(m_marking_part_one_program);
			glDispatchCompute((GLuint)256, 1, 1);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);

			glUseProgram(m_marking_part_two_program);
			glDispatchCompute((GLuint)256, 1, 1);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);
		
			// Flipping Step
			glUseProgram(m_flip_edges_part_one_program);
			glDispatchCompute((GLuint)256, 1, 1);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);

			glUseProgram(m_flip_edges_part_two_program);
			glDispatchCompute((GLuint)256, 1, 1);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);

			glUseProgram(m_flip_edges_part_three_program);
			glDispatchCompute((GLuint)256, 1, 1);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);

			cont = m_status.get_buffer_data<int>()[0];
		}
		// TODO: remove this creation of lct
		//refine_LCT();
		// points
		timer.stop();

		//LOG(std::string("Number of iterations: ") + std::to_string(counter));
		//LOG(std::string("Elapsed time in ms: ") + std::to_string(timer.elapsed_time()));

		//return timer.elapsed_time();
		/*glUseProgram(m_insert_in_edge_program);
		glDispatchCompute((GLuint)256, 1, 1);
		glMemoryBarrier(GL_ALL_BARRIER_BITS);*/

		//auto point_data_pos = m_point_bufs.positions.get_buffer_data<glm::vec2>();
		//auto point_data_inserted = m_point_bufs.inserted.get_buffer_data<int>();
		////auto point_data_triangle_index = m_point_bufs.tri_index.get_buffer_data<int>();

		//// symedges
		//auto symedges = m_sym_edges.get_buffer_data<SymEdge>();

		////// edges
		//auto edge_data_labels = m_edge_bufs.label.get_buffer_data<int>();
		//auto edge_data_is_constrained = m_edge_bufs.is_constrained.get_buffer_data<int>();

		//auto labels_3 = find_equal(edge_data_labels, 3);
		//LOG("Label_3: " + std::to_string(labels_3.size()));
		//auto labels_2 = find_equal(edge_data_labels, 2);
		//LOG("Label_2: " + std::to_string(labels_2.size()));
		//auto labels_1 = find_equal(edge_data_labels, 1);
		//LOG("Label_1: " + std::to_string(labels_1.size()));
		//labels_3 = find_equal(edge_data_labels, -3);
		//LOG("Label_-3: " + std::to_string(labels_3.size()));
		//labels_2 = find_equal(edge_data_labels, -2);
		//LOG("Label_-2: " + std::to_string(labels_2.size()));
		//labels_1 = find_equal(edge_data_labels, -4);
		//LOG("Label_-4: " + std::to_string(labels_1.size()));
		//for (int i = 0; i < std::min(int(labels_2.size()), 2); i++)
		//{
		//	LOG("Label_1_" + std::to_string(i) + ": " + std::to_string(labels_2[i]));
		//	int sym_edge_i = -1;
		//	for (int j = 0; j < symedges.size(); j++)
		//	{
		//		if (symedges[j].edge == labels_2[i])
		//		{
		//			LOG("Symedge_" + std::to_string(i) + ": " + std::to_string(j));
		//			break;
		//		}				
		//	}
		//	LOG("Goal" + std::to_string(i) + ": " + std::to_string(edge_data_is_constrained[labels_2[i]]));
		//}

		//// segments
		//auto segment_data_inserted = m_segment_bufs.inserted.get_buffer_data<int>();
		//auto segment_data_endpoint_indices = m_segment_bufs.endpoint_indices.get_buffer_data<glm::ivec2>();

		//// triangles
		//auto triangle_data_symedge_indices = m_triangle_bufs.symedge_indices.get_buffer_data<glm::ivec4>();
		//auto triangle_data_insert_point_index = m_triangle_bufs.ins_point_index.get_buffer_data<int>();
		//auto triangle_data_edge_flip_index = m_triangle_bufs.edge_flip_index.get_buffer_data<int>();
		//auto triangle_data_intersecting_segment = m_triangle_bufs.seg_inters_index.get_buffer_data<int>();
		//auto triangle_data_new_points = m_refine_points.get_buffer_data<NewPoint>();

		//auto status_data = m_status.get_buffer_data<int>();
		return timer.elapsed_time();
	}

	long long GPUMesh::refine_LCT()
	{
		int i = 0;
		int num_new_points;
		// bind all buffers
		m_point_bufs.positions.bind_buffer();
		m_point_bufs.inserted.bind_buffer();
		m_point_bufs.tri_index.bind_buffer();

		m_edge_bufs.label.bind_buffer();
		m_edge_bufs.is_constrained.bind_buffer();

		m_segment_bufs.endpoint_indices.bind_buffer();
		m_segment_bufs.inserted.bind_buffer();

		m_triangle_bufs.symedge_indices.bind_buffer();
		m_triangle_bufs.ins_point_index.bind_buffer();
		m_triangle_bufs.seg_inters_index.bind_buffer();
		m_triangle_bufs.edge_flip_index.bind_buffer();
		m_refine_points.bind_buffer();

		m_sym_edges.bind_buffer();
		m_nr_of_symedges.bind_buffer();
		m_epsilon_buff.bind_buffer();

		m_status.bind_buffer();
		// reset m_find_dist_status buffer and then bind it
		m_find_dist_status.update_buffer<Find_Disturbance_Status>({ Find_Disturbance_Status({0,0,0,0}) });
		m_find_dist_status.bind_buffer();
		m_version_buff.bind_buffer();

		Timer timer;
		timer.start();

		do
		{
			// Locate disturbances
			glUseProgram(m_locate_disturbances_program);
			glDispatchCompute((GLuint)256, 1, 1);
			glMemoryBarrier(GL_ALL_BARRIER_BITS);
			// Check how many new points are going to get inserted
			auto status = m_status.get_buffer_data<int>();
			num_new_points = status.front();
			if (num_new_points > 0)
			{
				m_new_points.set_used_element_count<vec2>(num_new_points);
				m_new_points.update_buffer(std::vector<vec2>(num_new_points, vec2(FLT_MAX)));
				//m_new_points.set_used_element_count<glm::vec2>(num_new_points);
				m_new_points.bind_buffer();
				// add new points to the point buffers
				glUseProgram(m_add_new_points_program);
				glDispatchCompute((GLuint)256, 1, 1);
				glMemoryBarrier(GL_ALL_BARRIER_BITS);

				// get new points from GPU and remove duplicates
				auto new_points = m_new_points.get_buffer_data<vec2>();
				remove_duplicate_points(new_points);
				// add the the points without duplicates to the point buffers
				m_point_bufs.positions.append_to_buffer(new_points);
				num_new_points = new_points.size();
				m_point_bufs.inserted.append_to_buffer(std::vector<int>(num_new_points, 0));
				m_point_bufs.tri_index.append_to_buffer(std::vector<int>(num_new_points, 0));


				// increase sizes of arrays, 
				// based on how many new points are inserted
				// segments
				int num_new_tri = num_new_points * 2;
				int num_new_segs = num_new_points;

				// fix new sizes of triangle buffers
				m_triangle_bufs.symedge_indices.append_to_buffer(std::vector<glm::ivec4>(num_new_tri, { -1, -1, -1, -1 }));
				m_triangle_bufs.ins_point_index.append_to_buffer(std::vector<int>(num_new_tri, -1));
				m_triangle_bufs.edge_flip_index.append_to_buffer(std::vector<int>(num_new_tri, -1));
				m_triangle_bufs.seg_inters_index.append_to_buffer(std::vector<int>(num_new_tri, -1));


				// fix new size of segment buffers
				m_segment_bufs.endpoint_indices.append_to_buffer(std::vector<glm::ivec2>(num_new_segs));
				m_segment_bufs.inserted.append_to_buffer(std::vector<int>(num_new_segs));
				// fix new sizes of edge buffers 
				// TODO: fix so it can handle repeated insertions
				int num_new_edges = num_new_points * 3;
				m_edge_bufs.is_constrained.append_to_buffer(std::vector<int>(num_new_edges, -1));
				m_edge_bufs.label.append_to_buffer(std::vector<int>(num_new_edges, -1));
				// fix new size of symedges buffer
				// TODO: fix so it can handle repeated insertions
				int num_new_sym_edges = num_new_points * 6;
				m_sym_edges.append_to_buffer(std::vector<SymEdge>(num_new_sym_edges));
				m_refine_points.append_to_buffer(std::vector<NewPoint>(num_new_sym_edges));
				// TODO, maybe need to check if triangle buffers needs to grow
				// resize new points array

				m_nr_of_symedges.update_buffer<int>({ m_sym_edges.element_count() });

				// then rebind the buffers that has been changed
				m_point_bufs.positions.bind_buffer();
				m_point_bufs.inserted.bind_buffer();
				m_point_bufs.tri_index.bind_buffer();

				m_edge_bufs.label.bind_buffer();
				m_edge_bufs.is_constrained.bind_buffer();

				m_segment_bufs.endpoint_indices.bind_buffer();
				m_segment_bufs.inserted.bind_buffer();

				m_triangle_bufs.symedge_indices.bind_buffer();
				m_triangle_bufs.ins_point_index.bind_buffer();
				m_triangle_bufs.seg_inters_index.bind_buffer();
				m_triangle_bufs.edge_flip_index.bind_buffer();
				m_refine_points.bind_buffer();


				m_sym_edges.bind_buffer();
				m_nr_of_symedges.bind_buffer();


				// Perform insertion of points untill all has been inserted 
				// and triangulation is CDT
				int counter = 0;
				int cont = 1;
				do
				{
					m_status.update_buffer<int>({ 0 });
					counter++;
					//// Find out which triangle the point is on the edge of
					glUseProgram(m_locate_point_triangle_program);
					glDispatchCompute((GLuint)256, 1, 1);
					glMemoryBarrier(GL_ALL_BARRIER_BITS);

					glUseProgram(m_validate_edges_program);
					glDispatchCompute((GLuint)256, 1, 1);
					glMemoryBarrier(GL_ALL_BARRIER_BITS);

					// Insert point into the edge
					glUseProgram(m_insert_in_edge_program);
					glDispatchCompute((GLuint)256, 1, 1);
					glMemoryBarrier(GL_ALL_BARRIER_BITS);

					// Perform marking
					glUseProgram(m_marking_part_two_program);
					glDispatchCompute((GLuint)256, 1, 1);
					glMemoryBarrier(GL_ALL_BARRIER_BITS);

					//Perform flipping to ensure that mesh is CDT
					glUseProgram(m_flip_edges_part_one_program);
					glDispatchCompute((GLuint)256, 1, 1);
					glMemoryBarrier(GL_ALL_BARRIER_BITS);

					glUseProgram(m_flip_edges_part_two_program);
					glDispatchCompute((GLuint)256, 1, 1);
					glMemoryBarrier(GL_ALL_BARRIER_BITS);

					glUseProgram(m_flip_edges_part_three_program);
					glDispatchCompute((GLuint)256, 1, 1);
					glMemoryBarrier(GL_ALL_BARRIER_BITS);
					cont = m_status.get_buffer_data<int>()[0];
				} while (cont == 1);
				//LOG(std::string("LCT Number of iterations: ") + std::to_string(counter));
			}
			else
			{
				break;
			}
		} while (true);
		timer.stop();
		return timer.elapsed_time();
	}

	std::vector<long long> GPUMesh::measure_shaders(std::vector<glm::vec2> points, std::vector<glm::ivec2> segments)
	{
		// CDT
		m_point_bufs.positions.append_to_buffer(points);
		m_point_bufs.inserted.append_to_buffer(std::vector<int>(points.size(), 0));
		m_point_bufs.tri_index.append_to_buffer(std::vector<int>(points.size(), 0));

		for (auto& segment : segments)
			segment = segment + glm::ivec2(m_segment_bufs.endpoint_indices.element_count());
		m_segment_bufs.endpoint_indices.append_to_buffer(segments);
		m_segment_bufs.inserted.append_to_buffer(std::vector<int>(segments.size(), 0));
		// uppdating ubo containing sizes

		int num_new_tri = points.size() * 2;
		int num_new_segs = segments.size();

		// fix new sizes of triangle buffers
		m_triangle_bufs.symedge_indices.append_to_buffer(std::vector<glm::ivec4>(num_new_tri, { -1, -1, -1, -1 }));
		m_triangle_bufs.ins_point_index.append_to_buffer(std::vector<int>(num_new_tri, -1));
		m_triangle_bufs.edge_flip_index.append_to_buffer(std::vector<int>(num_new_tri, -1));
		m_triangle_bufs.seg_inters_index.append_to_buffer(std::vector<int>(num_new_tri, -1));

		// fix new sizes of edge buffers 
		int num_new_edges = points.size() * 3;
		m_edge_bufs.is_constrained.append_to_buffer(std::vector<int>(num_new_edges, -1));
		m_edge_bufs.label.append_to_buffer(std::vector<int>(num_new_edges, -1));
		// fix new size of symedges buffer
		int num_new_sym_edges = points.size() * 6;
		m_sym_edges.append_to_buffer(std::vector<SymEdge>(num_new_sym_edges));
		m_refine_points.append_to_buffer(std::vector<NewPoint>(num_new_sym_edges));

		m_nr_of_symedges.update_buffer<int>({ m_sym_edges.element_count() });

		// Bind all ssbo's
		m_point_bufs.positions.bind_buffer();
		m_point_bufs.inserted.bind_buffer();
		m_point_bufs.tri_index.bind_buffer();

		m_edge_bufs.label.bind_buffer();
		m_edge_bufs.is_constrained.bind_buffer();

		m_segment_bufs.endpoint_indices.bind_buffer();
		m_segment_bufs.inserted.bind_buffer();

		m_triangle_bufs.symedge_indices.bind_buffer();
		m_triangle_bufs.ins_point_index.bind_buffer();
		m_triangle_bufs.seg_inters_index.bind_buffer();
		m_triangle_bufs.edge_flip_index.bind_buffer();
		m_refine_points.bind_buffer();

		m_sym_edges.bind_buffer();
		m_nr_of_symedges.bind_buffer();
		m_epsilon_buff.bind_buffer();

		m_status.bind_buffer();

		int counter = 0;

		Timer timer;

		std::vector<long long> build_times;
		build_times.resize(16, 0);

		int cont = 1;
		while (cont)
		{
			counter++;
			m_status.update_buffer<int>({ 0 });
			// Locate Step
			glUseProgram(m_location_program);
			timer.start();
			glDispatchCompute((GLuint)256, 1, 1);
			glFinish();
			timer.stop();
			build_times[0] += timer.elapsed_time();

			// Insert Step
			glUseProgram(m_insertion_program);
			timer.start();
			glDispatchCompute((GLuint)256, 1, 1);
			glFinish();
			timer.stop();
			build_times[1] += timer.elapsed_time();

			// Marking Step
			glUseProgram(m_marking_part_one_program);
			timer.start();
			glDispatchCompute((GLuint)256, 1, 1);
			glFinish();
			timer.stop();
			build_times[2] += timer.elapsed_time();

			glUseProgram(m_marking_part_two_program);
			timer.start();
			glDispatchCompute((GLuint)256, 1, 1);
			glFinish();
			timer.stop();
			build_times[3] += timer.elapsed_time();

			// Flipping Step
			glUseProgram(m_flip_edges_part_one_program);
			timer.start();
			glDispatchCompute((GLuint)256, 1, 1);
			glFinish();
			timer.stop();
			build_times[4] += timer.elapsed_time();

			glUseProgram(m_flip_edges_part_two_program);
			timer.start();
			glDispatchCompute((GLuint)256, 1, 1);
			glFinish();
			timer.stop();
			build_times[5] += timer.elapsed_time();

			glUseProgram(m_flip_edges_part_three_program);
			timer.start();
			glDispatchCompute((GLuint)256, 1, 1);
			glFinish();
			timer.stop();
			build_times[6] += timer.elapsed_time();

			cont = m_status.get_buffer_data<int>()[0];
		}

		//------------------------------------
		// LCT

		int num_new_points;
		// bind all buffers
		m_point_bufs.positions.bind_buffer();
		m_point_bufs.inserted.bind_buffer();
		m_point_bufs.tri_index.bind_buffer();

		m_edge_bufs.label.bind_buffer();
		m_edge_bufs.is_constrained.bind_buffer();

		m_segment_bufs.endpoint_indices.bind_buffer();
		m_segment_bufs.inserted.bind_buffer();

		m_triangle_bufs.symedge_indices.bind_buffer();
		m_triangle_bufs.ins_point_index.bind_buffer();
		m_triangle_bufs.seg_inters_index.bind_buffer();
		m_triangle_bufs.edge_flip_index.bind_buffer();
		m_refine_points.bind_buffer();

		m_sym_edges.bind_buffer();
		m_nr_of_symedges.bind_buffer();
		m_epsilon_buff.bind_buffer();

		m_status.bind_buffer();
		// reset m_find_dist_status buffer and then bind it
		m_find_dist_status.update_buffer<Find_Disturbance_Status>({ Find_Disturbance_Status({0,0,0,0}) });
		m_find_dist_status.bind_buffer();
		m_version_buff.bind_buffer();

		do
		{
			// Locate disturbances
			glUseProgram(m_locate_disturbances_program);
			timer.start();
			glDispatchCompute((GLuint)256, 1, 1);
			glFinish();
			timer.stop();
			build_times[7] += timer.elapsed_time();
			// Check how many new points are going to get inserted
			auto status = m_status.get_buffer_data<int>();
			num_new_points = status.front();
			if (num_new_points > 0)
			{
				m_new_points.set_used_element_count<glm::vec2>(num_new_points);
				m_new_points.update_buffer(std::vector<vec2>(num_new_points, vec2(FLT_MAX)));
				//m_new_points.set_used_element_count<glm::vec2>(num_new_points);
				m_new_points.bind_buffer();
				// add new points to the point buffers
				glUseProgram(m_add_new_points_program);
				timer.start();
				glDispatchCompute((GLuint)256, 1, 1);
				glFinish();
				timer.stop();
				build_times[8] += timer.elapsed_time();

				// get new points from GPU and remove duplicates
				auto new_points = m_new_points.get_buffer_data<vec2>();
				remove_duplicate_points(new_points);
				// add the the points without duplicates to the point buffers
				m_point_bufs.positions.append_to_buffer(new_points);
				num_new_points = new_points.size();
				m_point_bufs.inserted.append_to_buffer(std::vector<int>(num_new_points, 0));
				m_point_bufs.tri_index.append_to_buffer(std::vector<int>(num_new_points, 0));


				// increase sizes of arrays, based on how many new points are inserted
				// segments
				int num_new_tri = num_new_points * 2;
				int num_new_segs = num_new_points;

				// fix new sizes of triangle buffers
				m_triangle_bufs.symedge_indices.append_to_buffer(std::vector<glm::ivec4>(num_new_tri, { -1, -1, -1, -1 }));
				m_triangle_bufs.ins_point_index.append_to_buffer(std::vector<int>(num_new_tri, -1));
				m_triangle_bufs.edge_flip_index.append_to_buffer(std::vector<int>(num_new_tri, -1));
				m_triangle_bufs.seg_inters_index.append_to_buffer(std::vector<int>(num_new_tri, -1));

				// fix new size of segment buffers
				m_segment_bufs.endpoint_indices.append_to_buffer(std::vector<glm::vec2>(num_new_segs));
				m_segment_bufs.inserted.append_to_buffer(std::vector<int>(num_new_segs));
				// fix new sizes of edge buffers 
				int num_new_edges = num_new_points * 3;
				m_edge_bufs.is_constrained.append_to_buffer(std::vector<int>(num_new_edges, -1));
				m_edge_bufs.label.append_to_buffer(std::vector<int>(num_new_edges, -1));
				// fix new size of symedges buffer
				int num_new_sym_edges = num_new_points * 6;
				m_sym_edges.append_to_buffer(std::vector<SymEdge>(num_new_sym_edges));
				m_refine_points.append_to_buffer(std::vector<NewPoint>(num_new_sym_edges));
				// resize new points array
				m_nr_of_symedges.update_buffer<int>({ m_sym_edges.element_count() });

				// then rebind the buffers that has been changed
				m_point_bufs.positions.bind_buffer();
				m_point_bufs.inserted.bind_buffer();
				m_point_bufs.tri_index.bind_buffer();

				m_edge_bufs.label.bind_buffer();
				m_edge_bufs.is_constrained.bind_buffer();

				m_segment_bufs.endpoint_indices.bind_buffer();
				m_segment_bufs.inserted.bind_buffer();

				m_triangle_bufs.symedge_indices.bind_buffer();
				m_triangle_bufs.ins_point_index.bind_buffer();
				m_triangle_bufs.seg_inters_index.bind_buffer();
				m_triangle_bufs.edge_flip_index.bind_buffer();
				m_refine_points.bind_buffer();

				m_sym_edges.bind_buffer();
				m_nr_of_symedges.bind_buffer();

				int counter = 0;
				int cont = 1;
				do
				{
					int shader_id = 0;
					m_status.update_buffer<int>({ 0 });
					counter++;
					Timer timer;
					
					timer.start();
					// Find out which triangle the point is on the edge of
					glUseProgram(m_locate_point_triangle_program);
					timer.start();
					glDispatchCompute((GLuint)256, 1, 1);
					glFinish();
					timer.stop();
					build_times[9] += timer.elapsed_time();

					glUseProgram(m_validate_edges_program);
					timer.start();
					glDispatchCompute((GLuint)256, 1, 1);
					glFinish();
					timer.stop();
					build_times[10] += timer.elapsed_time();

					// Insert point into the edge
					glUseProgram(m_insert_in_edge_program);
					timer.start();
					glDispatchCompute((GLuint)256, 1, 1);
					glFinish();
					timer.stop();
					build_times[11] += timer.elapsed_time();

					// Perform marking
					glUseProgram(m_marking_part_two_program);
					timer.start();
					glDispatchCompute((GLuint)256, 1, 1);
					glFinish();
					timer.stop();
					build_times[12] += timer.elapsed_time();

					//Perform flipping to ensure that mesh is CDT
					glUseProgram(m_flip_edges_part_one_program);
					timer.start();
					glDispatchCompute((GLuint)256, 1, 1);
					glFinish();
					timer.stop();
					build_times[13] += timer.elapsed_time();

					glUseProgram(m_flip_edges_part_two_program);
					timer.start();
					glDispatchCompute((GLuint)256, 1, 1);
					glFinish();
					timer.stop();
					build_times[14] += timer.elapsed_time();

					glUseProgram(m_flip_edges_part_three_program);
					timer.start();
					glDispatchCompute((GLuint)256, 1, 1);
					glFinish();
					timer.stop();
					build_times[15] += timer.elapsed_time();

					cont = m_status.get_buffer_data<int>()[0];
				} while (cont == 1);
			}
			else
			{
				break;
			}
		} while (true);
		return build_times;
	}

	std::vector<glm::vec2> GPUMesh::get_vertices()
	{
		return m_point_bufs.positions.get_buffer_data<glm::vec2>();
	}

	int GPUMesh::get_num_vertices()
	{
		return m_point_bufs.positions.element_count();
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

	std::vector<glm::ivec3> GPUMesh::get_faces()
	{
		std::vector<SymEdge> sym_edge_list = m_sym_edges.get_buffer_data<SymEdge>();
		std::vector<glm::ivec4> sym_edge_tri_indices = m_triangle_bufs.symedge_indices.get_buffer_data<glm::ivec4>();

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

	int GPUMesh::locate_face(glm::vec2 p)
	{
		p = p - glm::vec2(2.f, 0.f);

		std::vector<SymEdge> sym_edge_list = m_sym_edges.get_buffer_data<SymEdge>();
		std::vector<glm::ivec4> sym_edge_tri_indices = m_triangle_bufs.symedge_indices.get_buffer_data<glm::ivec4>();
		std::vector<glm::vec2> vertex_list = m_point_bufs.positions.get_buffer_data<glm::vec2>();

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

	void GPUMesh::set_epsilon(float epsi)
	{
		m_epsilon = epsi;
		m_epsilon_buff.update_buffer<float>({ m_epsilon });
	}

	void GPUMesh::set_version(int vers)
	{
		version = vers;
		m_version_buff.update_buffer<int>({ version });
	}

	std::string GPUMesh::save_to_file(bool upload, int inserted_objects)
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
			std::vector<glm::vec2> v2vec;
			std::vector<int> ivec;
			std::vector<glm::ivec2> iv2vec;
			std::vector<glm::ivec4> iv4vec;

			// save point data
			v2vec = m_point_bufs.positions.get_buffer_data<glm::vec2>();
			size = (int)v2vec.size() * (int)sizeof(glm::vec2);
			output.write((char*)&size, sizeof(int));
			output.write((char*)v2vec.data(), size);
			v2vec.clear();

			ivec =  m_point_bufs.inserted.get_buffer_data<int>();
			size = (int)ivec.size() * (int)sizeof(int);
			output.write((char*)&size, sizeof(int));
			output.write((char*)ivec.data(), size);
			ivec.clear();

			ivec =  m_point_bufs.tri_index.get_buffer_data<int>();
			size = (int)ivec.size() * (int)sizeof(int);
			output.write((char*)&size, sizeof(int));
			output.write((char*)ivec.data(), size);
			ivec.clear();

			// save edge data
			ivec =  m_edge_bufs.label.get_buffer_data<int>();
			size = (int)ivec.size() * (int)sizeof(int);
			output.write((char*)&size, sizeof(int));
			output.write((char*)ivec.data(), size);
			ivec.clear();

			ivec =  m_edge_bufs.is_constrained.get_buffer_data<int>();
			size = (int)ivec.size() * (int)sizeof(int);
			output.write((char*)&size, sizeof(int));
			output.write((char*)ivec.data(), size);
			ivec.clear();

			// save segment data
			iv2vec = m_segment_bufs.endpoint_indices.get_buffer_data<glm::ivec2>();
			size = (int)iv2vec.size() * (int)sizeof(glm::ivec2);
			output.write((char*)&size, sizeof(int));
			output.write((char*)iv2vec.data(), size);
			iv2vec.clear();

			ivec = m_segment_bufs.inserted.get_buffer_data<int>();
			size = (int)ivec.size() * (int)sizeof(int);
			output.write((char*)&size, sizeof(int));
			output.write((char*)ivec.data(), size);
			ivec.clear();
			// save triangle data

			iv4vec = m_triangle_bufs.symedge_indices.get_buffer_data<glm::ivec4>();
			size = (int)iv4vec.size() * (int)sizeof(glm::ivec4);
			output.write((char*)&size, sizeof(int));
			output.write((char*)iv4vec.data(), size);
			iv4vec.clear();

			ivec = m_triangle_bufs.ins_point_index.get_buffer_data<int>();
			size = (int)ivec.size() * (int)sizeof(int);
			output.write((char*)&size, sizeof(int));
			output.write((char*)ivec.data(), size);
			ivec.clear();

			ivec = m_triangle_bufs.seg_inters_index.get_buffer_data<int>();
			size = (int)ivec.size() * (int)sizeof(int);
			output.write((char*)&size, sizeof(int));
			output.write((char*)ivec.data(), size);
			ivec.clear();

			ivec = m_triangle_bufs.edge_flip_index.get_buffer_data<int>();
			size = (int)ivec.size() * (int)sizeof(int);
			output.write((char*)&size, sizeof(int));
			output.write((char*)ivec.data(), size);
			ivec.clear();
			// save misc data

			auto npvec = m_new_points.get_buffer_data<glm::vec2>();
			size = (int)npvec.size() * (int)sizeof(glm::vec2);
			output.write((char*)&size, sizeof(int));
			output.write((char*)npvec.data(), size);

			auto refpvec = m_refine_points.get_buffer_data<NewPoint>();
			size = (int)refpvec.size() * (int)sizeof(NewPoint);
			output.write((char*)&size, sizeof(int));
			output.write((char*)refpvec.data(), size);

			auto symvec = m_sym_edges.get_buffer_data<SymEdge>();
			size = (int)symvec.size() * (int)sizeof(SymEdge);
			output.write((char*)&size, sizeof(int));
			output.write((char*)symvec.data(), size);

			output.close();
		}

		return filename;
	}

	void GPUMesh::load_from_file(std::string filename)
	{
		std::ifstream input(filename.c_str(), std::ifstream::in | std::ifstream::binary);
		int value;
		if (input.is_open())
		{
			// clear data
			m_point_bufs.positions.clear();
			m_point_bufs.inserted.clear();
			m_point_bufs.tri_index.clear();
			m_edge_bufs.is_constrained.clear();
			m_edge_bufs.label.clear();
			m_segment_bufs.endpoint_indices.clear();
			m_segment_bufs.inserted.clear();
			m_triangle_bufs.symedge_indices.clear();
			m_triangle_bufs.ins_point_index.clear();
			m_triangle_bufs.seg_inters_index.clear();
			m_triangle_bufs.edge_flip_index.clear();
			m_sym_edges.clear();
			m_new_points.clear();
			m_refine_points.clear();

			std::vector<glm::vec2> vec2_buff;
			std::vector<int> int_buff;
			std::vector<glm::ivec2> ivec2_buff;
			std::vector<glm::ivec4> ivec4_buff;
			std::vector<NewPoint> new_point_buff;
			std::vector<SymEdge> symedge_buff;

			// read points data
			input.read((char*)&value, sizeof(int));
			vec2_buff.resize(value / sizeof(glm::vec2));
			input.read((char*)vec2_buff.data(), value);
			m_point_bufs.positions.append_to_buffer(vec2_buff);
			vec2_buff.clear();

			input.read((char*)&value, sizeof(int));
			int_buff.resize(value / sizeof(int));
			input.read((char*)int_buff.data(), value);
			m_point_bufs.inserted.append_to_buffer(int_buff);
			int_buff.clear();

			input.read((char*)&value, sizeof(int));
			int_buff.resize(value / sizeof(int));
			input.read((char*)int_buff.data(), value);
			m_point_bufs.tri_index.append_to_buffer(int_buff);
			int_buff.clear();

			// read edge data
			input.read((char*)&value, sizeof(int));
			int_buff.resize(value / sizeof(int));
			input.read((char*)int_buff.data(), value);
			m_edge_bufs.label.append_to_buffer(int_buff);
			int_buff.clear();

			input.read((char*)&value, sizeof(int));
			int_buff.resize(value / sizeof(int));
			input.read((char*)int_buff.data(), value);
			m_edge_bufs.is_constrained.append_to_buffer(int_buff);
			int_buff.clear();

			// read segment data
			input.read((char*)&value, sizeof(int));
			ivec2_buff.resize(value / sizeof(glm::ivec2));
			input.read((char*)ivec2_buff.data(), value);
			m_segment_bufs.endpoint_indices.append_to_buffer(ivec2_buff);
			ivec2_buff.clear();

			input.read((char*)&value, sizeof(int));
			int_buff.resize(value / sizeof(int));
			input.read((char*)int_buff.data(), value);
			m_segment_bufs.inserted.append_to_buffer(int_buff);
			int_buff.clear();

			// read triangle data
			input.read((char*)&value, sizeof(int));
			ivec4_buff.resize(value / sizeof(glm::ivec4));
			input.read((char*)ivec4_buff.data(), value);
			m_triangle_bufs.symedge_indices.append_to_buffer(ivec4_buff);
			ivec4_buff.clear();

			input.read((char*)&value, sizeof(int));
			int_buff.resize(value / sizeof(int));
			input.read((char*)int_buff.data(), value);
			m_triangle_bufs.ins_point_index.append_to_buffer(int_buff);
			int_buff.clear();

			input.read((char*)&value, sizeof(int));
			int_buff.resize(value / sizeof(int));
			input.read((char*)int_buff.data(), value);
			m_triangle_bufs.seg_inters_index.append_to_buffer(int_buff);
			int_buff.clear();

			input.read((char*)&value, sizeof(int));
			int_buff.resize(value / sizeof(int));
			input.read((char*)int_buff.data(), value);
			m_triangle_bufs.edge_flip_index.append_to_buffer(int_buff);
			int_buff.clear();

			// read misc data
			input.read((char*)&value, sizeof(int));
			vec2_buff.resize(value / sizeof(glm::vec2));
			input.read((char*)vec2_buff.data(), value);
			m_new_points.append_to_buffer(vec2_buff);
			vec2_buff.clear();

			input.read((char*)&value, sizeof(int));
			new_point_buff.resize(value / sizeof(NewPoint));
			input.read((char*)new_point_buff.data(), value);
			m_refine_points.append_to_buffer(new_point_buff);
			new_point_buff.clear();

			input.read((char*)&value, sizeof(int));
			symedge_buff.resize(value / sizeof(SymEdge));
			input.read((char*)symedge_buff.data(), value);
			m_sym_edges.append_to_buffer(symedge_buff);
			symedge_buff.clear();

			m_nr_of_symedges.update_buffer<int>({ m_sym_edges.element_count() });
			m_status.update_buffer<int>({ 0 });

			input.close();
		}
		else
			LOG_T(WARNING, "can not open file:" + filename);
	}

	Find_Disturbance_Status GPUMesh::get_find_dist_status()
	{
		auto tmp_list = m_find_dist_status.get_buffer_data<Find_Disturbance_Status>();
		return tmp_list.front();
	}

	void GPUMesh::setup_compute_shaders()
	{
		// CDT
		compile_cs(m_location_program, "GPU/location_step.glsl");
		compile_cs(m_location_tri_program, "GPU/location_step_triangle.glsl");
		compile_cs(m_insertion_program, "GPU/insertion_step.glsl");
		compile_cs(m_marking_part_one_program, "GPU/marking_step_part_one.glsl");
		compile_cs(m_marking_part_two_program, "GPU/marking_step_part_two.glsl");
		compile_cs(m_flip_edges_part_one_program, "GPU/flipping_part_one.glsl");
		compile_cs(m_flip_edges_part_two_program, "GPU/flipping_part_two.glsl");
		compile_cs(m_flip_edges_part_three_program, "GPU/flipping_part_three.glsl");

		// LCT
		compile_cs(m_locate_disturbances_program, "GPU/locate_disturbances.glsl");
		compile_cs(m_add_new_points_program, "GPU/add_new_points.glsl");
		compile_cs(m_locate_point_triangle_program, "GPU/locate_point_triangle.glsl");
		compile_cs(m_validate_edges_program, "GPU/validate_edges.glsl");
		compile_cs(m_insert_in_edge_program, "GPU/insert_in_edge.glsl");
	}

	void GPUMesh::compile_cs(GLuint & program, const char * path, int work_group_size)
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
		GLint success;
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
	void GPUMesh::remove_duplicate_points(std::vector<vec2>& list)
	{
		// loops backwards to remove as far back as possible
		int i = 0;
		int num_valid_points = list.size();
		while (i < num_valid_points)
		{
			if (point_equal(list[i], vec2(FLT_MAX)))
			{
				vec2 tmp = list[i];
				list[i] = list[--num_valid_points];
				list[num_valid_points] = tmp;
				continue;
			}
			int j = i + 1;
			while (j < num_valid_points)
			{
				if (point_equal(list[i], list[j], m_epsilon))
				{
					// swap away point behind the valid pointsto be removed later.
					vec2 tmp = list[j];
					list[j] = list[--num_valid_points];
					list[num_valid_points] = tmp;
				}
				j++;
			}
			i++;
		}
		// removed the last points
		list.erase(list.begin() + num_valid_points, list.end());
	}
}
