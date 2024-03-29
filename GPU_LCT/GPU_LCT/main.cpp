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

#include "TestFunctions.hpp"

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

void test_test_map(GPU::GCMesh &m, GPU::GPUMesh &g_m, glm::vec2 dims, glm::ivec2 num_objects)
{
	TestMap test_map;
	test_map.set_map_size(dims, -dims);
	test_map.set_num_obsticles(num_objects);
	test_map.set_static_quota(1.0f);
	test_map.set_dynamic_quota(0.75f);

	//auto obsticles = test_map.get_CPU_obsticles();
	//for (unsigned int i = 0; i < obsticles.size(); i++)
	//{
	//	m.insert_constraint(std::move(obsticles[i]), i);
	//}

	// create GPU data
	g_m.set_epsilon(0.001f);
	auto gpu_frame = test_map.get_GPU_frame();
	auto gpu_map = test_map.get_GPU_static_obstacles();
	auto dynamic_obj = test_map.get_GPU_dynamic_obstacles();
	//m.build_CDT(gpu_map.first, gpu_map.second);
	//m.refine_LCT();
	int vers = 2;
	g_m.set_version(vers);
	g_m.add_frame_points(gpu_frame.first);
	g_m.build_CDT(gpu_map.first, gpu_map.second);
	g_m.refine_LCT();
	auto stat = g_m.get_find_dist_status();
	LOG("GPU const_list_status: " + std::to_string(stat.const_list_status));
	LOG("GPU const_queue_status: " + std::to_string(stat.const_queue_status));
	LOG("GPU dist_list_status: " + std::to_string(stat.dist_list_status));
	LOG("GPU dist_queue_status: " + std::to_string(stat.dist_queue_status));
	//m.set_version(vers);
	//m.add_frame_points(gpu_frame.first);
	//m.build_CDT(gpu_map.first, gpu_map.second);
	//m.refine_LCT();

	//g_m.build_CDT(dynamic_obj.first, dynamic_obj.second);

}
bool check_for_sliver_tri(std::array<vec2, 3> tri, float epsi = EPSILON)
{
	// first find the longest edge
	float best_dist = 0.0f;
	int best_i = -1;
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

	return point_ray_test(p1, s1, s2, epsi * 10.0f);
}
int main()
{
	// Important that the renderer is created first because it initializes OpenGL
	Renderer renderer({ 1600, 800 });
	float scale = 490.0f;
	int num_object_multi = 110;
	glm::vec2 map_scaling = { scale, scale };
	glm::ivec2 num_objects = { num_object_multi, num_object_multi };
	GPU::GPUMesh g_mesh;
	g_mesh.initiate_buffers(map_scaling);
	//g_mesh.build_CDT({ { -0.25f, -0.25f }, { -0.25f, 0.25f }, { 0.25f, 0.25f }, { 0.25f, -0.25f } }, { {0, 1}, {1, 2}, {2, 3}, {3, 0}, {0, 2} });
	//vec2 s1 = vec2(100.0f);
	//vec2 s2 = vec2(101.0f);
	//vec2 s3 = vec2(100.01f, 100.0f);
	//vec2 s4 = vec2(101.0f);
	//
	//LOG(std::to_string(line_line_test(s1, s2, s3, s4, 0.001f)));
	renderer.set_camera_base_zoom(map_scaling, 2.3f);

	GPU::GCMesh gc_mesh;
	gc_mesh.initiate_buffers(map_scaling);

	// testing function
	//test_range({ 1600, 800 }, 2, { 2, 2 }, { 1, 1 }, { 2, 2 }, { 1, 1 }, false);

	/*CPU::Mesh m;
	m.initialize_as_quad({ 0.5f, 0.5f }, { 0.f, 0.f });*/

	//lct_example(m, g_mesh);

	//test_test_map(gc_mesh, g_mesh, map_scaling, num_objects);

	//--------------------------------------------------------------------------
	//**************************************************************************
	// FIRST TEST
	//**************************************************************************
	//--------------------------------------------------------------------------
	int number_of_increases = 10;
	int iterations = 10;
	int start = 3;
	int steps = 3;
	bool test_CPU = true;
	bool test_GPU = false;
	int version = 1;
	//first_test({ 10, 10 }, {10, 10}, 1, 10, true, false, 1 );
	//--------------------------------------------------------------------------
	// CPUGPU v1
	//--------------------------------------------------------------------------
	//number_of_increases = 10;
	//iterations = 10;
	//start = 3;
	//steps = 3;
	//test_CPU = true;
	//test_GPU = false;
	//version = 1;

	//first_test({ start, start }, {steps, steps}, number_of_increases, iterations, test_CPU, test_GPU, version );
	//--------------------------------------------------------------------------
	// CPUGPU v2
	//--------------------------------------------------------------------------
	//number_of_increases = 20;
	//iterations = 10;
	//start = 3;
	//steps = 3;
	//test_CPU = true;
	//test_GPU = false;
	//version = 2;

	//first_test({ start, start }, {steps, steps}, number_of_increases, iterations, test_CPU, test_GPU, version );

	//--------------------------------------------------------------------------
	// GPU v1
	//--------------------------------------------------------------------------
	//number_of_increases = 20;
	//iterations = 10;
	//start = 3;
	//steps = 3;
	//test_CPU = false;
	//test_GPU = true;
	//version = 1;

	//first_test({ start, start }, {steps, steps}, number_of_increases, iterations, test_CPU, test_GPU, version );


	//--------------------------------------------------------------------------
	// GPU v2
	//--------------------------------------------------------------------------
	//number_of_increases = 20;
	//iterations = 10;
	//start = 3;
	//steps = 3;
	//test_CPU = false;
	//test_GPU = true;
	//version = 2;

	//first_test({ start, start }, {steps, steps}, number_of_increases, iterations, test_CPU, test_GPU, version );

	//--------------------------------------------------------------------------
	//**************************************************************************
	// SECOND TEST
	//**************************************************************************
	//--------------------------------------------------------------------------
	//second_test({ 110, 110 }, 10);

	//--------------------------------------------------------------------------
	//**************************************************************************
	// THIRD TEST
	//**************************************************************************
	//--------------------------------------------------------------------------
	
	std::vector<std::pair<glm::ivec2, float>> test1;


	//--------------------------------------------------------------------------
	// CPUGPU v1
	//--------------------------------------------------------------------------
	//number_of_increases = 10;
	//iterations = 10;
	//start = 3;
	//steps = 3;
	//test_CPU = true;
	//test_GPU = false;
	//version = 1;
	//// Third test, 25 %
	//test1 = gen_obstacle_range({ start, start }, { steps, steps }, number_of_increases, 0.25f);
	//generate_third_test_input("test", test1);
	//third_test("test", iterations, test_CPU, test_GPU, 0.25f, version);

	//// Third test, 50 %
	//test1 = gen_obstacle_range({ start, start }, { steps, steps }, number_of_increases, 0.5f);
	//generate_third_test_input("test", test1);
	//third_test("test", iterations, test_CPU, test_GPU, 0.5f, version);

	//// Third test, 75 %
	//test1 = gen_obstacle_range({ start, start }, { steps, steps }, number_of_increases, 0.75f);
	//generate_third_test_input("test", test1);
	//third_test("test", iterations, test_CPU, test_GPU, 0.75f, version);

	//--------------------------------------------------------------------------
	// CPUGPU v2
	//--------------------------------------------------------------------------
	//number_of_increases = 16;
	//iterations = 10;
	//start = 3;
	//steps = 3;
	//test_CPU = true;
	//test_GPU = false;
	//version = 2;
	//// Third test, 25 %
	//test1 = gen_obstacle_range({ start, start }, { steps, steps }, number_of_increases, 0.25f);
	//generate_third_test_input("test", test1);
	//third_test("test", iterations, test_CPU, test_GPU, 0.25f, version);

	//// Third test, 50 %
	//test1 = gen_obstacle_range({ start, start }, { steps, steps }, number_of_increases, 0.5f);
	//generate_third_test_input("test", test1);
	//third_test("test", iterations, test_CPU, test_GPU, 0.5f, version);

	//// Third test, 75 %
	//test1 = gen_obstacle_range({ start, start }, { steps, steps }, number_of_increases, 0.75f);
	//generate_third_test_input("test", test1);
	//third_test("test", iterations, test_CPU, test_GPU, 0.75f, version);

	//--------------------------------------------------------------------------
	// GPU v1
	//--------------------------------------------------------------------------
	//number_of_increases = 20;
	//iterations = 10;
	//start = 3;
	//steps = 3;
	//test_CPU = false;
	//test_GPU = true;
	//version = 1;
	//// Third test, 25 %
	//test1 = gen_obstacle_range({ start, start }, { steps, steps }, number_of_increases, 0.25f);
	//generate_third_test_input("test", test1);
	//third_test("test", iterations, test_CPU, test_GPU, 0.25f, version);

	//// Third test, 50 %
	//test1 = gen_obstacle_range({ start, start }, { steps, steps }, number_of_increases, 0.5f);
	//generate_third_test_input("test", test1);
	//third_test("test", iterations, test_CPU, test_GPU, 0.5f, version);

	//// Third test, 75 %
	//test1 = gen_obstacle_range({ start, start }, { steps, steps }, number_of_increases, 0.75f);
	//generate_third_test_input("test", test1);
	//third_test("test", iterations, test_CPU, test_GPU, 0.75f, version);

	//--------------------------------------------------------------------------
	// GPU v2
	//--------------------------------------------------------------------------
	//number_of_increases = 40;
	//iterations = 10;
	//start = 3;
	//steps = 3;
	//test_CPU = false;
	//test_GPU = true;
	//version = 2;
	// Third test, 25 %
	//test1 = gen_obstacle_range({ start, start }, { steps, steps }, number_of_increases, 0.25f);
	//generate_third_test_input("test", test1);
	//third_test("test", iterations, test_CPU, test_GPU, 0.25f, version);

	// Third test, 50 %
	//test1 = gen_obstacle_range({ start, start }, { steps, steps }, number_of_increases, 0.5f);
	//generate_third_test_input("test", test1);
	//third_test("test", iterations, test_CPU, test_GPU, 0.5f, version);

	// Third test, 75 %
/*	test1 = gen_obstacle_range({ start, start }, { steps, steps }, number_of_increases, 0.75f);
	generate_third_test_input("test", test1);
	third_test("test", iterations, test_CPU, test_GPU, 0.75f, version)*/;




	//gc_mesh.load_from_file("Output files/throwGC_150_155");
	//g_mesh.load_from_file("Output files/throwGC_150_155");

	//m.transform_into_LCT();

	//m.Locate_point({ 0.5f, 0.5f });
	//m.Locate_point({ 0.f, 0.f });
	float edge_thiccness = 2.0f;
	float point_thiccness = 4.0f;

	// Left side objects
	DebugObject debug_faces(DRAW_FACES);
	debug_faces.set_color({ 1.0f, 0.5f, 0.2f });
	debug_faces.build(gc_mesh);

	DebugObject debug_edges(DRAW_EDGES);
	debug_edges.set_edge_thiccness(edge_thiccness);
	debug_edges.set_color({ 1.f, 0.246201f, 0.201556f });
	debug_edges.build(gc_mesh);

	DebugObject debug_edges_constraints(DRAW_EDGES);
	debug_edges_constraints.set_edge_thiccness(edge_thiccness);
	debug_edges_constraints.draw_constraints(true);
	debug_edges_constraints.set_color({ 0.1f, 0.6f, 0.1f });
	debug_edges_constraints.build(gc_mesh);

	DebugObject symedge_visualizer({ glm::vec2(-0.25f, 0.25f), glm::vec2(0.25f, -0.25f) }, DRAW_EDGES);
	symedge_visualizer.set_edge_thiccness(point_thiccness);
	symedge_visualizer.set_color({ 0.f, 0.f, 0.8f });

	DebugObject debug_points(DRAW_POINTS);
	debug_points.set_point_thiccness(point_thiccness);
	debug_points.set_color({ 1.f, 0.672443f, 0.201556f });
	debug_points.build(gc_mesh);

	// Right side objects

	DebugObject r_debug_faces(DRAW_FACES);
	r_debug_faces.set_color({ 1.0f, 0.5f, 0.2f });
	r_debug_faces.build(g_mesh);
	r_debug_faces.set_draw_left_side(false);

	DebugObject r_debug_edges(DRAW_EDGES);
	r_debug_edges.set_edge_thiccness(edge_thiccness);
	r_debug_edges.set_color({ 1.f, 0.246201f, 0.201556f });
	r_debug_edges.build(g_mesh);
	r_debug_edges.set_draw_left_side(false);

	DebugObject r_debug_edges_constraints(DRAW_EDGES);
	r_debug_edges_constraints.set_edge_thiccness(edge_thiccness);
	r_debug_edges_constraints.draw_constraints(true);
	r_debug_edges_constraints.set_color({ 0.1f, 0.6f, 0.1f });
	r_debug_edges_constraints.build(g_mesh);
	r_debug_edges_constraints.set_draw_left_side(false);

	DebugObject r_symedge_visualizer({ glm::vec2(-0.25f, 0.25f), glm::vec2(0.25f, -0.25f) }, DRAW_EDGES);
	r_symedge_visualizer.set_edge_thiccness(edge_thiccness);
	r_symedge_visualizer.set_color({ 0.f, 0.f, 0.8f });
	r_symedge_visualizer.set_draw_left_side(false);

	DebugObject r_debug_points(DRAW_POINTS);
	r_debug_points.set_point_thiccness(point_thiccness);
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

