#version 430 core

out vec4 FragColor;
in vec4 gl_FragCoord;

struct input_parameters
{
	vec3 color;
	float circle_thiccness;
	float screen_resolution;
	vec2 viewport_offset;
	float pad;
};

uniform input_parameters input_data;

struct circle_struct
{
    vec2 center;
    float radius;
	float pad;
};

layout(std430, binding = 1) buffer circle_data_buffer
{
	circle_struct circle_data[];
};

void main()
{
	vec4 output_color = vec4(0.f, 0.f, 0.f, 0.f);
	for (int circle_index = 0; circle_index < circle_data.length(); circle_index++)
	{
		// Rescale coordinate
		vec2 coordinate = ((gl_FragCoord.xy - input_data.viewport_offset) / vec2(input_data.screen_resolution)) * vec2(2.f) - vec2(1.f);
		// Calculate the distance from the pixel position to the circle boundary
		float dist_from_boundary = abs(circle_data[circle_index].radius - length(coordinate - circle_data[circle_index].center));
		// Calculate the shading factor
		float falloff = 1.f - dist_from_boundary / input_data.circle_thiccness;
		// Use blending and the alpha channel to enable the output depending on if the pixel coord is within distance
		output_color = max(output_color, vec4(vec3(falloff), (falloff) * (1.f - step(input_data.circle_thiccness, dist_from_boundary))));
	}
	FragColor = output_color * vec4(input_data.color, 1.f);
};
