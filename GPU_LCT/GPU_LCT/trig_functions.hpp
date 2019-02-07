#pragma once
#ifndef TRIG_FUNCTIONS_HPP
#define TRIG_FUNCTIONS_HPP
#include<glm/vec2.hpp>


bool line_seg_intersection_ccw(glm::vec2 p1, glm::vec2 q1, glm::vec2 p2, glm::vec2 q2);

enum Ori {
	COLINEAR,
	CLOCKWISE,
	COUNTER_CLOCKWISE
};

Ori orientation(glm::vec2 p1, glm::vec2 p2, glm::vec2 p3);



#endif
