#include "Mesh.hpp"
#include "DebugObject.hpp"
#include "DebugPipeline.hpp"
#include "Renderer.h"

int main()
{
	// Important that the renderer is created first because it initializes OpenGL
	Renderer renderer;

	Mesh m;
	m.Initialize_as_quad({ 0.5f, 0.5f }, { 0.f, 0.f });
	glm::vec2 point = { 0.4f, -0.4f };
	LocateRes lr = m.Locate_point(point);
	m.Insert_point_in_face(point, lr.sym_edge);
	point = { -0.2f, 0.1f };
	lr = m.Locate_point(point);
	m.Insert_point_in_face(point, lr.sym_edge);
	point = { -0.2f, 0.2f };
	lr = m.Locate_point(point);
	m.Insert_point_in_face(point, lr.sym_edge);

	//point = { 0.f, 0.0f };
	//lr = m.Locate_point(point);
	//m.Insert_point_in_edge(point, lr.sym_edge);

	point = { 0.f, -0.5f };
	lr = m.Locate_point(point);
	m.Insert_point_in_edge(point, lr.sym_edge);

	std::vector<glm::vec2> points = { {-0.5f, -0.5f}, {0.5f, 0.5f} };

	m.insert_constraint(std::move(points), 0);

	points = { {0.f, -0.2f}, {0.f, 0.2f} };

	m.insert_constraint(std::move(points), 1);

	points = { {0.f, -0.4f}, {0.f, 0.4f} };

	m.insert_constraint(std::move(points), 2);

	//m.Locate_point({ 0.5f, 0.5f });
	//m.Locate_point({ 0.f, 0.f });

	DebugObject thingerino(m, DRAW_ALL);
	thingerino.set_point_thiccness(10.f);
	thingerino.set_edge_thiccness(5.f);
	thingerino.set_point_color({ 1.f, 0.672443f, 0.201556f });
	thingerino.set_edge_color({ 1.f, 0.246201f, 0.201556f });
	thingerino.set_face_color({ 1.0f, 0.5f, 0.2f });

	ShaderPath debug_draw_path;
	debug_draw_path[VS] = "debug_vertex_shader.glsl";
	debug_draw_path[FS] = "debug_fragment_shader.glsl";

	DebugPipeline debug_pass;
	debug_pass.add_pass(DebugPipeline::DEBUG_PASS, std::move(debug_draw_path));
	debug_pass.add_drawable(std::move(thingerino));

	renderer.add_pipeline(std::move(debug_pass));

	while (!renderer.shut_down)
	{
		renderer.run();
	}
	return 0;
}

