#include "Mesh.hpp"
#include "DebugObject.hpp"
#include "DebugPipeline.hpp"
#include "DelaunayDebugObject.hpp"
#include "DelaunayDebugPipeline.hpp"
#include "Renderer.hpp"
#include <array>

int main()
{
	// Important that the renderer is created first because it initializes OpenGL
	Renderer renderer(800);

	Mesh m;
	m.initialize_as_quad({ 0.5f, 0.5f }, { 0.f, 0.f });
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

	m.insert_constraint(std::move(points), 0);

	points = { {0.f, -0.4f}, {0.f, 0.4f} };

	m.insert_constraint(std::move(points), 1);
	
	/*point = {0.f, -0.2f};
	lr = m.Locate_point(point);
	m.Insert_point_in_edge(point, lr.sym_edge);

	point = {0.f, 0.2f};
	lr = m.Locate_point(point);
	m.Insert_point_in_edge(point, lr.sym_edge);*/

	points = { { 0.f, -0.2f }, {0.f, 0.2f}, {-0.2, 0.1f} };

	m.insert_constraint(std::move(points), 2);

	m.transform_into_LCT();

	//m.Locate_point({ 0.5f, 0.5f });
	//m.Locate_point({ 0.f, 0.f });

	DebugObject thingerino(m, DRAW_ALL, true);
	thingerino.set_point_thiccness(10.f);
	thingerino.set_edge_thiccness(5.f);
	thingerino.set_point_color({ 1.f, 0.672443f, 0.201556f });
	thingerino.set_edge_color({ 1.f, 0.246201f, 0.201556f });
	thingerino.set_face_color({ 1.0f, 0.5f, 0.2f });

	DebugObject symedge_visualizer({ glm::vec2(-0.25f, 0.25f), glm::vec2(0.25f, -0.25f) }, DRAW_EDGES);
	symedge_visualizer.set_edge_thiccness(5.f);
	symedge_visualizer.set_edge_color({ 0.f, 0.f, 0.8f });

	ShaderPath debug_draw_path;
	debug_draw_path[VS] = "debug_vertex_shader.glsl";
	debug_draw_path[FS] = "debug_fragment_shader.glsl";

	DebugPipeline debug_pass;
	debug_pass.add_pass(DebugPipeline::DEBUG_PASS, std::move(debug_draw_path));
	debug_pass.add_drawable(std::move(thingerino));
	debug_pass.add_drawable(std::move(symedge_visualizer));
	
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
		symedge_visualizer.update_edge(m.get_edge(renderer.m_current_edge->edge));
		renderer.run();
	}
	return 0;
}

