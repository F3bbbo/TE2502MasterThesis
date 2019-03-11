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
void lct_example(CPU::Mesh &m)
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

}

int main()
{
	// Important that the renderer is created first because it initializes OpenGL
	Renderer renderer(800);

	GPU::GPUMesh g_mesh;
	g_mesh.initiate_buffers({ {0.5, 0.6} }, {}, {});

	CPU::Mesh m;
	m.initialize_as_quad({ 0.5f, 0.5f }, { 0.f, 0.f });

	lct_example(m);

	//TestMap test_map;
	//test_map.set_map_size({ 0.5f, 0.5f }, { -0.5f, -0.5f });
	//test_map.set_num_obsticles({ 8, 8 });
	//test_map.set_obsticle_scale(0.05f);

	//auto obsticles = test_map.get_obsticles();
	//for (unsigned int i = 0; i < obsticles.size(); i++)
	//{
	//	m.insert_constraint(std::move(obsticles[i]), i);
	//}
	//m.transform_into_LCT();

	//m.Locate_point({ 0.5f, 0.5f });
	//m.Locate_point({ 0.f, 0.f });

	DebugObject debug_faces(DRAW_FACES);
	debug_faces.set_color({ 1.0f, 0.5f, 0.2f });
	debug_faces.build(m);

	DebugObject debug_edges(DRAW_EDGES);
	debug_edges.set_edge_thiccness(5.f);
	debug_edges.set_color({ 1.f, 0.246201f, 0.201556f });
	debug_edges.build(m);

	DebugObject debug_edges_constraints(DRAW_EDGES);
	debug_edges_constraints.set_edge_thiccness(5.f);
	debug_edges_constraints.draw_constraints(true);
	debug_edges_constraints.set_color({ 0.1f, 0.6f, 0.1f });
	debug_edges_constraints.build(m);

	DebugObject symedge_visualizer({ glm::vec2(-0.25f, 0.25f), glm::vec2(0.25f, -0.25f) }, DRAW_EDGES);
	symedge_visualizer.set_edge_thiccness(5.f);
	symedge_visualizer.set_color({ 0.f, 0.f, 0.8f });

	DebugObject debug_points(DRAW_POINTS);
	debug_points.set_point_thiccness(10.f);
	debug_points.set_color({ 1.f, 0.672443f, 0.201556f });
	debug_points.build(m);

	ShaderPath debug_draw_path;
	debug_draw_path[VS] = "debug_vertex_shader.glsl";
	debug_draw_path[FS] = "debug_fragment_shader.glsl";

	DebugPipeline debug_pass;
	debug_pass.add_pass(DebugPipeline::DEBUG_PASS, std::move(debug_draw_path));
	debug_pass.add_drawable(std::move(debug_faces));
	debug_pass.add_drawable(std::move(debug_edges));
	debug_pass.add_drawable(std::move(debug_edges_constraints));
	debug_pass.add_drawable(std::move(symedge_visualizer));
	debug_pass.add_drawable(std::move(debug_points));

	DelaunayDebugObject ddo(m);
	ddo.set_circle_color({ 1.f, 1.f, 0.f });
	ddo.set_circle_thiccness(0.005f);
	ddo.enable(true);

	ShaderPath delaunay_draw_path;
	delaunay_draw_path[VS] = "debug_delaunay_vertex_shader.glsl";
	delaunay_draw_path[FS] = "debug_delaunay_fragment_shader.glsl";

	DelaunayDebugPipeline ddp((float)renderer.get_screen_res());
	ddp.add_pass(DelaunayDebugPipeline::DELAUNAY_DEBUG_PASS, std::move(delaunay_draw_path));
	ddp.m_circles = ddo;

	renderer.add_pipeline(std::move(debug_pass));
	renderer.add_pipeline(std::move(ddp));
	//set debug edge of renderer
	renderer.set_debug_edge(m.first);

	renderer.check_error();
	while (!renderer.shut_down)
	{
		symedge_visualizer.update_edge({ m.get_vertex(renderer.m_current_edge->vertex), m.get_other_edge_vertex(renderer.m_current_edge->edge, renderer.m_current_edge->vertex) });
		renderer.run();
		// handle mouse click
		if (renderer.mouse_clicked())
		{
			std::cout << "Clicked Triangle(index): " << m.locate_face(renderer.get_mouse_pos()) << std::endl;
			//std::cout << "Mouse Location: {" << renderer.get_mouse_pos().x << ", " << renderer.get_mouse_pos().y << "}\n";
		}
	}
	return 0;
}

