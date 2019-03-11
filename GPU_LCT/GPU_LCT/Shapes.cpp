#include "Shapes.hpp"

//-------------------------------------------------------------------
// Base Shape
//-------------------------------------------------------------------

Shape::Shape()
{
	m_scale = 1.0f;
	m_location = { 0.0f, 0.0f };
}

Shape::Shape(std::initializer_list<glm::vec2> l)
	:m_points(l)
{
}

Shape::~Shape()
{
}

std::vector<glm::vec2> Shape::get_segments()
{
	// Transform the points of the segments into world pos before returning them.
	std::vector<glm::vec2> trans_points;
	trans_points.reserve(m_points.size());
	for (auto point : m_points) {
		glm::vec2 tmp = point;
		tmp *= m_scale;
		tmp += m_location;
		trans_points.push_back(tmp);
	}
	return trans_points;
}

void Shape::set_scale(float s)
{
	m_scale = s;
}

void Shape::set_location(glm::vec2 p)
{
	m_location = p;
}

//-------------------------------------------------------------------
// Triangle
//-------------------------------------------------------------------

Triangle::Triangle()
	:Shape({ {0.0f, 0.5f}, {-0.5f, -0.5f}, {0.5f, -0.5f} })
{

}
Triangle::Triangle(float scale, glm::vec2 location)
	: Triangle()
{
	m_scale = scale;
	m_location = location;
}

Triangle::~Triangle()
{
}

std::vector<glm::vec2> Triangle::get_segments()
{
	std::vector<glm::vec2> ret_segs = Shape::get_segments();
	ret_segs.push_back(ret_segs[0]);
	return ret_segs;
}



//-------------------------------------------------------------------
// Square
//-------------------------------------------------------------------
Square::Square()
	:Shape({ {0.5f, 0.5f}, {0.5f,-0.5f}, {-0.5f, -0.5f},{-0.5f, 0.5f} })
{


}

Square::Square(float scale, glm::vec2 location)
	:Square()
{
	m_scale = scale;
	m_location = location;
}

Square::~Square()
{

}

std::vector<glm::vec2> Square::get_segments()
{
	std::vector<glm::vec2> ret_segs = Shape::get_segments();
	ret_segs.push_back(ret_segs[0]);
	return ret_segs;
}
