#pragma once
#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera
{
public:
	Camera();
	~Camera();

	void build_camera();
	glm::mat4x4 get_matrix();
	void translate(glm::vec3 vec);
	// no need for rotation
	void zoom(float factor);

	static float m_zoom;
	static float m_zoom_speed;
	static bool m_dirty;
	float translate_speed_factor = 1.f;
private:

	glm::vec3 m_position = glm::vec3(0.f, 0.f, 0.f);
	glm::vec3 m_forward = glm::vec3(0.f, 0.f, 1.f);
	float m_half_width = 1;
	float m_half_height = 1;

	glm::mat4x4 m_view_matrix;
	glm::mat4x4 m_ortho_matrix;
	glm::mat4x4 m_final_matrix;
};

#endif
