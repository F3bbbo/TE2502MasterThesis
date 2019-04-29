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

float vec2_cross(glm::vec2 &v, glm::vec2& w)
{
	return v.x*w.y - v.y*w.x;
}

bool line_line_test(glm::vec2 p1, glm::vec2 p2, glm::vec2 q1, glm::vec2 q2, float epsi)
{
	// solution found:
	//https://stackoverflow.com/questions/563198/how-do-you-detect-where-two-line-segments-intersect
	glm::vec2 s = p2 - p1;
	glm::vec2 r = q2 - q1;
	float rs = vec2_cross(s, r);
	glm::vec2 qp = (q1 - p1);
	float qpr = vec2_cross(qp, r);
	if (abs(rs) < epsi && abs(qpr) < epsi) // case 1
	{
		float r2 = dot(r, r);
		float t0 = dot((q1 - p1), r) / r2;
		float sr = dot(s, r);
		float t1 = t0 + (sr / r2);
		if (sr < 0.0f)
		{
			float tmp = t0;
			t0 = t1;
			t1 = tmp;
		}
		if ((t0 < 0.0f && t1 < 0.0f) || t0 > 1.0f && t1 > 1.0f)
			return false;
		else
			return true;
	}
	else if (abs(rs) < epsi && !(abs(qpr) < epsi)) // case 2
	{
		return false;
	}
	else // case 3
	{
		float u = qpr / rs;
		float t = vec2_cross(qp, s) / rs;
		if ((0.0f - epsi) < u && u < (1.0f + epsi) && (0.0f - epsi) < t && t < (1.0f + epsi))
			return true;
	}
	return false;
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

bool point_line_test(glm::vec2 p1, glm::vec2 s1, glm::vec2 s2, float epsi)
{

	//glm::vec3 v1 = glm::vec3(s1 - p1, 0.0f);
	//glm::vec3 v2 = glm::vec3(s1 - s2, 0.0f);
	//if (fabs(glm::length(glm::cross(v1, v2))) > epsi)
	//	return false;
	glm::vec2 dist_vec = project_point_on_line(p1, s1, s2);
	if (!point_ray_test(p1, s1, s2, epsi))
		return false;
	glm::vec2 v1 = s1 - p1;
	glm::vec2 v2 = s1 - s2;
	float dot_p = glm::dot(v1, v2);
	if (dot_p < epsi * epsi)
		return false;
	if (dot_p > (glm::length2(v2) - epsi * epsi))
		return false;

	return true;
}

bool point_ray_test(glm::vec2 p1, glm::vec2 r1, glm::vec2 r2, float epsi)
{
	glm::vec2 dist_vec = project_point_on_line(p1, r1, r2);
	return abs(glm::distance(dist_vec, p1)) < epsi ? true : false;
}

float sign(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3)
{
	return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
}

bool point_triangle_test(glm::vec2 p1, glm::vec2 t1, glm::vec2 t2, glm::vec2 t3, float epsi)
{

	float d1, d2, d3;
	bool has_neg, has_pos;

	d1 = sign(p1, t1, t2);
	d2 = sign(p1, t2, t3);
	d3 = sign(p1, t3, t1);

	has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
	has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

	return !(has_neg && has_pos);
}

bool segment_triangle_test(glm::vec2 p1, glm::vec2 p2, glm::vec2 t1, glm::vec2 t2, glm::vec2 t3)
{
	if (point_triangle_test(p1, t1, t2, t3) && point_triangle_test(p2, t1, t2, t3)) {
		// If triangle contains both points of segment return true
		return true;
	}
	if (line_seg_intersection_ccw(p1, p2, t1, t2) ||
		line_seg_intersection_ccw(p1, p2, t2, t3) ||
		line_seg_intersection_ccw(p1, p2, t3, t1)) {
		// If segment intersects any of the edges of the triangle return true
		return true;
	}
	// Otherwise segment is missing the triangle
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

float line_length2(glm::vec2 line)
{
	return line.x * line.x + line.y * line.y;
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

	if (std::fabs(determinant) < epsi)
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
	for (size_t i = 0; i < points.size(); i++)
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

glm::vec2 project_point_on_line(glm::vec2 point, glm::vec2 a, glm::vec2 b)
{
	glm::vec2 ab = glm::normalize(b - a);
	glm::vec2 ap = point - a;
	return a + glm::dot(ap, ab) * ab;
}

glm::vec2 get_symmetrical_corner(glm::vec2 a, glm::vec2 b, glm::vec2 c)
{
	glm::vec2 ac = c - a;
	glm::vec2 half = a + (ac / 2.f);
	float len = line_length(half - project_point_on_line(b, a, c));
	return b + 2.f * len * glm::normalize(ac);
}

float area_of_triangle(glm::vec2 a, glm::vec2 b, glm::vec2 c)
{
	// https://www.mathsisfun.com/geometry/herons-formula.html
	float abc[3] = { line_length(b - a) + line_length(c - b) + line_length(a - c) };
	float s = (abc[0] + abc[1] + abc[2]) / 2.f;
	return glm::sqrt(s * (s - abc[0]) * (s - abc[1]) * (s - abc[2]));
}

bool point_inside_triangle(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 p)
{
	// Source
	// https://www.youtube.com/watch?time_continue=47&v=H9qu9Xptf-w
	float combined_area = area_of_triangle(p, a, b) + area_of_triangle(p, b, c) + area_of_triangle(p, a, c);
	float abc_area = area_of_triangle(a, b, c);
	return (combined_area > abc_area - EPSILON) && (combined_area < abc_area + EPSILON);
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

std::vector<glm::vec2> ray_circle_intersection(std::array<glm::vec2, 2> ray, glm::vec2 center, float r)
{
	// Solution
	// https://math.stackexchange.com/questions/311921/get-location-of-vector-circle-intersection

	float a = (ray[1].x - ray[0].x) * (ray[1].x - ray[0].x) + (ray[1].y - ray[0].y) * (ray[1].y - ray[0].y);
	float b = 2.f * (ray[1].x - ray[0].x) * (ray[0].x - center.x) + 2.f * (ray[1].y - ray[0].y) * (ray[0].y - center.y);
	float c = (ray[0].x - center.x) * (ray[0].x - center.x) + (ray[0].y - center.y) * (ray[0].y - center.y) - r * r;

	float disc = b * b - 4.f * a * c;
	if (disc < 0.f)
		return {};

	// Alternative quadratic formula for more numerical precision
	// http://mathworld.wolfram.com/QuadraticFormula.html
	float t[2] = { (2.f * c) / (-b + glm::sqrt(disc)), (2.f * c) / (-b - glm::sqrt(disc)) };

	std::vector<glm::vec2> result;
	for (int i = 0; i < 2; i++)
	{
		if (t[i] >= 0.f && t[i] <= 1.f)
			result.push_back(ray[0] + ((ray[1] - ray[0])* t[i]));
	}
	return result;
}