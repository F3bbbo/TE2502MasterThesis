#include "Camera.hpp"

using namespace glm;

float Camera::m_zoom_speed = 0.1f;
float Camera::m_zoom = 1.f;
bool Camera::m_dirty = true;

Camera::Camera()
{
}

Camera::~Camera()
{
}

void Camera::build_camera()
{
	m_view_matrix = lookAtLH(m_position, m_position + m_forward, glm::vec3(0.f, 1.f, 0.f));
	m_ortho_matrix = ortho(-m_half_width * Camera::m_zoom, m_half_width * Camera::m_zoom, -m_half_height * Camera::m_zoom, m_half_height * Camera::m_zoom);
	m_final_matrix = m_ortho_matrix * m_view_matrix;
}

glm::mat4x4 Camera::get_matrix()
{
	if (m_dirty)
	{
		build_camera();
		m_dirty = false;
	}
	return m_final_matrix;
}

void Camera::translate(glm::vec3 vec)
{
	m_position += vec;
	m_dirty = true;
}

void Camera::zoom(float factor)
{
	m_half_width = m_half_width * Camera::m_zoom_speed;
	m_half_height = m_half_height * Camera::m_zoom_speed;
	m_dirty = true;
}

void Camera::set_starting_dimensions(float width, float height)
{
	m_half_width = width / 2.f;
	m_half_height = height / 2.f;
	m_dirty = true;
}
