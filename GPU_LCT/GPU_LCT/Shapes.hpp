#pragma once
#ifndef SHAPES_HPP
#define SHAPES_HPP
#include <vector>
#include <glm/glm.hpp>

class Shape {
public:
	Shape();
	Shape(std::initializer_list<glm::vec2> l);
	Shape(std::vector<glm::vec2> &vec);
	virtual ~Shape();
	virtual std::vector<glm::vec2> get_segments();
	void set_scale(float s);
	void set_scale(glm::vec2 s);
	void set_location(glm::vec2 p);
protected:
	std::vector<glm::vec2> m_points;
	glm::vec2 m_scale;
	glm::vec2 m_location;
};


class Triangle : public Shape {
public:
	Triangle();
	Triangle(float scale, glm::vec2 location);
	virtual ~Triangle();
	virtual std::vector<glm::vec2> get_segments();
};

class Square : public Shape {
public:
	Square();
	Square(float scale, glm::vec2 location);
	virtual ~Square();
	virtual std::vector<glm::vec2> get_segments();
};


#endif
