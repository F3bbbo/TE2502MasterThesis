#include "Mesh.hpp"
#include "DebugObject.hpp"
#include "DebugPipeline.hpp"
#include "DelaunayDebugObject.hpp"
#include "DelaunayDebugPipeline.hpp"
#include "Renderer.hpp"
#include <array>
#include "Shapes.hpp"
#include "TestMap.hpp"

#include "GPU/GPU_Mesh.hpp"
#include "GPU/GPU_CPU_Mesh.hpp"

void lct_example(CPU::Mesh &m, GPU::GPUMesh &g_m)
{
	/*glm::vec2 point = { 0.4f, -0.4f };
	LocateRes lr = m.Locate_point(point);
	m.insert_point_in_face(point, lr.sym_edge);
	point = { -0.2f, 0.1f };
	lr = m.Locate_point(point);
	m.insert_point_in_face(point, lr.sym_edge);
	point = { -0.2f, 0.2f };
	lr = m.Locate_point(point);
	m.insert_point_in_face(point, lr.sym_edge);

	point = { 0.f, -0.5f };
	lr = m.Locate_point(point);
	m.insert_point_in_edge(point, lr.sym_edge);*/

	std::vector<glm::vec2> points = { {-0.5f, -0.5f}, {0.5f, 0.5f} };

	//m.insert_constraint(std::move(points), 0);

	points = {
	{ -0.35f, 0.2f},
	{ 0.35f, 0.2f},
	{ 0.4f, 0.1f},
	{ -0.4f, 0.1f},
	{ -0.35f, 0.2f} };

	m.insert_constraint(std::move(points), 1);

	/*point = {0.f, -0.2f};
	lr = m.Locate_point(point);
	m.Insert_point_in_edge(point, lr.sym_edge);

	point = {0.f, 0.2f};
	lr = m.Locate_point(point);
	m.Insert_point_in_edge(point, lr.sym_edge);*/

	points = {
	{ -0.1f, -0.06f },
	{ -0.0f, -0.06f },
	{ -0.0f, -0.4f },
	{ -0.1f, -0.379f },
	{ -0.1f, -0.06f } };

	m.insert_constraint(std::move(points), 2);

	points = {
		{ 0.25f, -0.05f },
	{ 0.15f, -0.05f },
	{ 0.15f, -0.3f },
	{ 0.25f, -0.3f },
	{ 0.25f, -0.05f } };

	m.insert_constraint(std::move(points), 3);

	// GPU
	std::vector<glm::ivec2> segments;
	points = {
		{ -0.35f, 0.2f},
	{ 0.35f, 0.2f},
	{ 0.4f, 0.1f},
	{ -0.4f, 0.1f}, //end
	{ -0.1f, -0.06f },
	{ -0.0f, -0.06f },
	{ -0.0f, -0.4f },
	{ -0.1f, -0.379f }, // end
	{ 0.25f, -0.05f },
	{ 0.15f, -0.05f },
	{ 0.15f, -0.3f },
	{ 0.25f, -0.3f }
	};
	segments = {
		{0,1},
	{1,2},
	{2,3},
	{3,0}, // end
	{4,5},
	{5,6},
	{6,7},
	{7,4}, //end
	{8,9},
	{9,10},
	{10,11},
	{11,8}
	};
	g_m.build_CDT(points, segments);

}

void test_test_map(GPU::GCMesh &m, GPU::GPUMesh &g_m, glm::vec2 dims)
{
	TestMap test_map;
	test_map.set_map_size(dims, -dims);
	test_map.set_num_obsticles({ 20, 20 });

	//auto obsticles = test_map.get_CPU_obsticles();
	//for (unsigned int i = 0; i < obsticles.size(); i++)
	//{
	//	m.insert_constraint(std::move(obsticles[i]), i);
	//}

	// create GPU data
	auto gpu_map = test_map.get_GPU_obstacles();
	m.build_CDT(gpu_map.first, gpu_map.second);
	//g_m.build_CDT(gpu_map.first, gpu_map.second);
}

int main()
{
	// Important that the renderer is created first because it initializes OpenGL
	Renderer renderer({ 1600, 800 });

	glm::vec2 map_scaling = { 1.0f, 1.0f };

	GPU::GPUMesh g_mesh({ 1600, 800 });
	g_mesh.initiate_buffers(map_scaling);
	//g_mesh.build_CDT({ { -0.25f, -0.25f }, { -0.25f, 0.25f }, { 0.25f, 0.25f }, { 0.25f, -0.25f } }, { {0, 1}, {1, 2}, {2, 3}, {3, 0}, {0, 2} });

	renderer.set_camera_base_zoom(map_scaling, 2.3f);

	GPU::GCMesh gc_mesh({ 1600, 800 });
	gc_mesh.initiate_buffers(map_scaling);


	/*CPU::Mesh m;
	m.initialize_as_quad({ 0.5f, 0.5f }, { 0.f, 0.f });*/

	//lct_example(m, g_mesh);

	test_test_map(gc_mesh, g_mesh, map_scaling);

	//m.transform_into_LCT();

	//m.Locate_point({ 0.5f, 0.5f });
	//m.Locate_point({ 0.f, 0.f });

	// Left side objects
	DebugObject debug_faces(DRAW_FACES);
	debug_faces.set_color({ 1.0f, 0.5f, 0.2f });
	debug_faces.build(gc_mesh);

	DebugObject debug_edges(DRAW_EDGES);
	debug_edges.set_edge_thiccness(5.f);
	debug_edges.set_color({ 1.f, 0.246201f, 0.201556f });
	debug_edges.build(gc_mesh);

	DebugObject debug_edges_constraints(DRAW_EDGES);
	debug_edges_constraints.set_edge_thiccness(5.f);
	debug_edges_constraints.draw_constraints(true);
	debug_edges_constraints.set_color({ 0.1f, 0.6f, 0.1f });
	debug_edges_constraints.build(gc_mesh);

	DebugObject symedge_visualizer({ glm::vec2(-0.25f, 0.25f), glm::vec2(0.25f, -0.25f) }, DRAW_EDGES);
	symedge_visualizer.set_edge_thiccness(5.f);
	symedge_visualizer.set_color({ 0.f, 0.f, 0.8f });

	DebugObject debug_points(DRAW_POINTS);
	debug_points.set_point_thiccness(10.f);
	debug_points.set_color({ 1.f, 0.672443f, 0.201556f });
	debug_points.build(gc_mesh);

	// Right side objects

	DebugObject r_debug_faces(DRAW_FACES);
	r_debug_faces.set_color({ 1.0f, 0.5f, 0.2f });
	r_debug_faces.build(g_mesh);
	r_debug_faces.set_draw_left_side(false);

	DebugObject r_debug_edges(DRAW_EDGES);
	r_debug_edges.set_edge_thiccness(5.f);
	r_debug_edges.set_color({ 1.f, 0.246201f, 0.201556f });
	r_debug_edges.build(g_mesh);
	r_debug_edges.set_draw_left_side(false);

	DebugObject r_debug_edges_constraints(DRAW_EDGES);
	r_debug_edges_constraints.set_edge_thiccness(5.f);
	r_debug_edges_constraints.draw_constraints(true);
	r_debug_edges_constraints.set_color({ 0.1f, 0.6f, 0.1f });
	r_debug_edges_constraints.build(g_mesh);
	r_debug_edges_constraints.set_draw_left_side(false);

	DebugObject r_symedge_visualizer({ glm::vec2(-0.25f, 0.25f), glm::vec2(0.25f, -0.25f) }, DRAW_EDGES);
	r_symedge_visualizer.set_edge_thiccness(5.f);
	r_symedge_visualizer.set_color({ 0.f, 0.f, 0.8f });
	r_symedge_visualizer.set_draw_left_side(false);

	DebugObject r_debug_points(DRAW_POINTS);
	r_debug_points.set_point_thiccness(10.f);
	r_debug_points.set_color({ 1.f, 0.672443f, 0.201556f });
	r_debug_points.build(g_mesh);
	r_debug_points.set_draw_left_side(false);

	ShaderPath debug_draw_path;
	debug_draw_path[VS] = "debug_vertex_shader.glsl";
	debug_draw_path[FS] = "debug_fragment_shader.glsl";

	DebugPipeline debug_pass(renderer.get_screen_res());
	debug_pass.add_pass(DebugPipeline::DEBUG_PASS, std::move(debug_draw_path));
	debug_pass.add_drawable(std::move(debug_faces));
	debug_pass.add_drawable(std::move(debug_edges));
	debug_pass.add_drawable(std::move(debug_edges_constraints));
	debug_pass.add_drawable(std::move(symedge_visualizer));
	debug_pass.add_drawable(std::move(debug_points));

	debug_pass.add_drawable(std::move(r_debug_faces));
	debug_pass.add_drawable(std::move(r_debug_edges));
	debug_pass.add_drawable(std::move(r_debug_edges_constraints));
	debug_pass.add_drawable(std::move(r_symedge_visualizer));
	debug_pass.add_drawable(std::move(r_debug_points));

	DelaunayDebugObject ddo(gc_mesh);
	ddo.set_circle_color({ 1.f, 1.f, 0.f });
	ddo.set_circle_thiccness(0.005f);
	ddo.enable(false);

	DelaunayDebugObject r_ddo(g_mesh);
	r_ddo.set_circle_color({ 1.f, 1.f, 0.f });
	r_ddo.set_circle_thiccness(0.005f);
	r_ddo.enable(false);
	r_ddo.set_draw_left_side(false);

	ShaderPath delaunay_draw_path;
	delaunay_draw_path[VS] = "debug_delaunay_vertex_shader.glsl";
	delaunay_draw_path[FS] = "debug_delaunay_fragment_shader.glsl";

	DelaunayDebugPipeline ddp(renderer.get_screen_res());
	ddp.add_pass(DelaunayDebugPipeline::DELAUNAY_DEBUG_PASS, std::move(delaunay_draw_path));
	ddp.m_circles.push_back(ddo);
	ddp.m_circles.push_back(r_ddo);

	renderer.add_pipeline(std::move(debug_pass));
	renderer.add_pipeline(std::move(ddp));
	//set debug edge of renderer

	renderer.set_debug_edge(nullptr, gc_mesh.get_symedge(0));
	renderer.set_gc_mesh(&gc_mesh);
	renderer.set_gpu_mesh(&g_mesh);

	while (!renderer.shut_down)
	{
		symedge_visualizer.update_edge(renderer.get_gc_edge());
		r_symedge_visualizer.update_edge(renderer.get_GPU_edge());
		renderer.run();
		// handle mouse click
		if (renderer.mouse_clicked())
		{
			int result = gc_mesh.locate_face(renderer.get_mouse_pos());
			if (result != -1)
				std::cout << "Clicked CPU Triangle(index): " << result << std::endl;
			else
			{
				result = g_mesh.locate_face(renderer.get_mouse_pos());
				if (result != -1)
					std::cout << "Clicked GPU Triangle(index): " << result << std::endl;
				else
					std::cout << "Clicked Triangle(index): -1" << std::endl;
			}
			//std::cout << "Mouse Location: {" << renderer.get_mouse_pos().x << ", " << renderer.get_mouse_pos().y << "}\n";
		}
	}
	return 0;
}

