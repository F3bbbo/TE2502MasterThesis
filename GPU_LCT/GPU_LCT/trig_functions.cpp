#include "trig_functions.hpp"
//#include <glm/geometric.hpp>
//#include <glm/exponential.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/norm.hpp>

bool line_seg_intersection_ccw(glm::vec2 p1, glm::vec2 q1, glm::vec2 p2, glm::vec2 q2)
{
	Ori o1 = orientation(p1, q1, p2);
	Ori o2 = orientation(p1, q1, q2);
	Ori o3 = orientation(p2, q2, p1);
	Ori o4 = orientation(p2, q2, q1);

	if (o1 != o2 && o3 != o4)
		return true;

	return false;
}

Ori orientation(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3)
{
	float val = (p2.y - p1.y) * (p3.x - p2.x) - (p2.x - p1.x) * (p3.y - p2.y);

	if (val == 0.0f) return COLINEAR;
	return (val > 0.0f) ? CLOCKWISE : COUNTER_CLOCKWISE;
}

glm::vec2 tri_centroid(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3)
{
	return (p1 + p2 + p3) / 3.0f;
}

bool point_equal(glm::vec2 p1, glm::vec2 p2, float epsi)
{
	glm::vec2 tmp = p1 - p2;
	if (fabs(tmp.x) < epsi && fabs(tmp.y) < epsi)
		return true;
	return false;
}

bool point_segment_test(glm::vec2 p1, glm::vec2 s1, glm::vec2 s2, float epsi)
{
	glm::vec3 v1 = glm::vec3(s1 - p1, 0.0f);
	glm::vec3 v2 = glm::vec3(s1 - s2, 0.0f);
	if (fabs(glm::length(glm::cross(v1, v2))) > epsi)
		return false;
	float dot_p = glm::dot(v1, v2);
	if (dot_p < epsi)
		return false;
	if (dot_p > (glm::length2(v2) - epsi))
		return false;

	return true;
}

bool point_triangle_test(glm::vec2 p1, glm::vec2 t1, glm::vec2 t2, glm::vec2 t3, float epsi)
{

	return false;
}

glm::vec2 point_segment_projection(glm::vec2 p1, glm::vec2 s1, glm::vec2 s2)
{
	glm::vec2 seg_dir = glm::normalize(s2 - s1);
	glm::vec2 tmp = p1 - s1;
	float dist = glm::dot(seg_dir, tmp);
	return s1 + seg_dir * dist;
}
