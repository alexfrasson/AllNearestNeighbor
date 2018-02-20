#include "../include/Point.h"

Point::Point(int32_t x, int32_t y)
{
	m_x = x;
	m_y = y;
}

bool Point::operator==(const Point& other) const
{
	return m_name == other.m_name && m_x == other.m_x && m_y == other.m_y;
}


std::istream& operator >> (std::istream& stream, Point& point)
{
	stream >> point.m_name;

	if (!point.m_name.empty())
	{
		std::string val;

		//bracket
		stream >> val;
		//x
		stream >> val;
		point.m_x = std::stoi(val);

		//comma
		stream >> val;
		//y
		stream >> val;
		point.m_y = std::stoi(val);

		//bracket
		stream >> val;
	}

	return stream;
}

std::ostream& operator<<(std::ostream& stream, const Point& point)
{
	stream << point.m_name << " ( " << point.m_x << " , " << point.m_y << " ) ";

	return stream;
}