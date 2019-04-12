#pragma once
#ifndef TRIG_FUNCTIONS_HPP
#define TRIG_FUNCTIONS_HPP
#include<glm/vec2.hpp>
#define EPSILON 0.00005f
#include <array>
#include <vector>

#include "Log.hpp"

bool line_seg_intersection_ccw(glm::vec2 p1, glm::vec2 q1, glm::vec2 p2, glm::vec2 q2);

bool line_line_test(glm::vec2 p1, glm::vec2 q1, glm::vec2 p2, glm::vec2 q2, float epsi = EPSILON);

enum Ori {
	COLINEAR,
	CLOCKWISE,
	COUNTER_CLOCKWISE
};

Ori orientation(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3);

glm::vec2 tri_centroid(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3);

bool point_equal(glm::vec2 p1, glm::vec2 p2, float epsi = EPSILON);

bool point_line_test(glm::vec2 p1, glm::vec2 s1, glm::vec2 s2, float epsi = EPSILON);

bool point_ray_test(glm::vec2 p1, glm::vec2 r1, glm::vec2 r2, float epsi = EPSILON);

bool point_triangle_test(glm::vec2 p1, glm::vec2 t1, glm::vec2 t2, glm::vec2 t3, float epsi = EPSILON);

bool segment_triangle_test(glm::vec2 p1, glm::vec2 p2, glm::vec2 t1, glm::vec2 t2, glm::vec2 t3); // returns true if segment intersect triangle or is inside of triangle

glm::vec2 point_segment_projection(glm::vec2 p1, glm::vec2 s1, glm::vec2 s2);

float line_length(glm::vec2 line);

float line_length2(glm::vec2 line);

glm::vec2 line_line_intersection_point(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 d, float epsi = EPSILON);

// the three first points belong to the triangle and are ccw
// the last point is the point being tested
bool point_in_circle(std::array<glm::vec2, 4 > points);

glm::vec2 circle_center_from_points(glm::vec2 a, glm::vec2 b, glm::vec2 c);

std::vector<glm::vec2> ray_circle_intersection(std::array<glm::vec2, 2> ray, glm::vec2 center, float r);

glm::vec2 project_point_on_line(glm::vec2 point, glm::vec2 a, glm::vec2 b);

glm::vec2 get_symmetrical_corner(glm::vec2 a, glm::vec2 b, glm::vec2 c);

float area_of_triangle(glm::vec2 a, glm::vec2 b, glm::vec2 c);

bool point_inside_triangle(glm::vec2 a, glm::vec2 b, glm::vec2 c, glm::vec2 p);

#endif
