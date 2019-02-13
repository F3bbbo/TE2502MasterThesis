#pragma once
#ifndef TRIG_FUNCTIONS_HPP
#define TRIG_FUNCTIONS_HPP
#include<glm/vec2.hpp>
#define EPSILON 0.00005

bool line_seg_intersection_ccw(glm::vec2 p1, glm::vec2 q1, glm::vec2 p2, glm::vec2 q2);

enum Ori {
	COLINEAR,
	CLOCKWISE,
	COUNTER_CLOCKWISE
};

Ori orientation(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3);

glm::vec2 tri_centroid(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3);

bool point_equal(glm::vec2 p1, glm::vec2 p2, float epsi = EPSILON);

bool point_segment_test(glm::vec2 p1, glm::vec2 s1, glm::vec2 s2, float epsi = EPSILON);

bool point_triangle_test(glm::vec2 p1, glm::vec2 t1, glm::vec2 t2, glm::vec2 t3, float epsi = EPSILON);

glm::vec2 point_segment_projection(glm::vec2 p1, glm::vec2 s1, glm::vec2 s2);

#endif
