#include "Mesh.hpp"
#include "DebugObject.hpp"
#include "DebugPipeline.hpp"
#include "Renderer.h"

int main()
{
	Renderer renderer;

	Mesh m;
	m.Initialize_as_quad({ 0.5f, 0.5f }, { 0.f, 0.f });

	DebugObject thingerino(m, BOTH, { 1.0f, 0.5f, 0.2f, }, { 1.f, 0.246201f, 0.201556f, });
	thingerino.set_line_thiccness(5.f);

	ShaderPath debug_draw_path;
	debug_draw_path[VS] = "debug_vertex_shader.glsl";
	debug_draw_path[FS] = "debug_fragment_shader.glsl";

	DebugPipeline debug_pass;
	debug_pass.add_pass(DebugPipeline::DEBUG_PASS, std::move(debug_draw_path));
	debug_pass.add_drawable(std::move(thingerino));

	renderer.add_pipeline(std::move(debug_pass));
	renderer.run();

	return 0;
}

