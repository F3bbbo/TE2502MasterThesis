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

float line_length(glm::vec2 line)
{
	return glm::sqrt(line.x * line.x + line.y * line.y);
}

glm::vec2 line_line_intersection_point(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d, float epsi)
{
	// Line AB represented as a1x + b1y = c1 
	float a1 = b.y - a.y;
	float b1 = a.x - b.x;
	float c1 = a1 * a.x + b1 * a.y;

	// Line CD represented as a2x + b2y = c2 
	float a2 = d.y - c.y;
	float b2 = c.x - d.x;
	float c2 = a2 * (c.x) + b2 * (c.y);

	float determinant = a1 * b2 - a2 * b1;

	if ( std::fabs(determinant) < epsi )
	{
		// The lines are parallel. This is simplified 
		// by returning a pair of FLT_MAX 
		return { FLT_MAX, FLT_MAX };
	}
	else
	{
		float x = (b2 * c1 - b1 * c2) / determinant;
		float y = (a1 * c2 - a2 * c1) / determinant;
		return { x, y };
	}

	//// http://demonstrations.wolfram.com/IntersectionOfTwoLinesUsingVectors/
	//if (glm::dot(w - u, z - v) < 0.f)
	//{
	//	auto tmp = u;
	//	u = w;
	//	w = tmp;
	//}


	//float beta = glm::acos(glm::dot(w - u, z - v));
	//glm::vec2 uv = v - u;
	//float alpha = glm::acos(glm::dot(uv, z - v));

	//return v + line_length(uv) * ((sin(alpha) * (z - v)) / (sin(beta) * line_length(z - v)));
}

bool point_in_circle(std::array<glm::vec2, 4> points)
{
	glm::mat4x4 mat;
	for (int i = 0; i < points.size(); i++)
	{
		mat[0][i] = points[i].x;
		mat[1][i] = points[i].y;
		mat[2][i] = mat[0][i] * mat[0][i] + mat[1][i] * mat[1][i];
		mat[3][i] = 1.f;
	}

	if (glm::determinant(mat) > 0)
		return true;
	return false;
}

glm::vec2 circle_center_from_points(glm::vec2 a, glm::vec2 b, glm::vec2 c)
{
	glm::vec2 ab = b - a;
	glm::vec2 bc = c - b;

	std::array<glm::vec2, 2> midpoints = { a + ab / 2.f, b + bc / 2.f };
	std::array<glm::vec2, 2> normals;
	// rotate vectors 90 degrees
	glm::vec3 vec = cross(glm::vec3(ab.x, ab.y, 0.f), glm::vec3(0.f, 0.f, 1.f));
	normals[0] = glm::vec2(vec.x, vec.y);

	vec = cross(glm::vec3(bc.x, bc.y, 0.f), glm::vec3(0.f, 0.f, 1.f));
	normals[1] = glm::vec2(vec.x, vec.y);

	return line_line_intersection_point(midpoints[0], midpoints[0] + normals[0], midpoints[1], midpoints[1] + normals[1]);
}

glm::vec2 project_point_on_line(glm::vec2 point, glm::vec2 line)
{
	line = glm::normalize(line);
	return glm::dot(point, line) * line;
}

glm::vec2 get_symmetrical_corner(glm::vec2 a, glm::vec2 b, glm::vec2 c)
{
	glm::vec2 ac = c - a;
	glm::vec2 half = a + (ac / 2.f);
	float len = line_length(half - project_point_on_line(b, ac));
	return b + 2.f * len * glm::normalize(ac);
}
