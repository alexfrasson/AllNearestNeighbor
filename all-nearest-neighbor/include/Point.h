#pragma once
#include <string>
#include <fstream>
#include <cmath>

struct Point
{
	int32_t m_x = 0;
	int32_t m_y = 0;
	std::string m_name;

	Point() = default;

	Point(int32_t x, int32_t y);

	inline double squaredMagnitude() const;
	inline double magnitude() const;

	bool operator==(const Point& other) const;
	inline int32_t& operator[](std::size_t idx);
	inline Point operator-(const Point& other) const;
};

std::istream& operator >> (std::istream& stream, Point& point);
std::ostream& operator << (std::ostream& stream, const Point& point);

int32_t& Point::operator[](std::size_t idx)
{
	return idx == 0 ? m_x : m_y;
}

Point Point::operator-(const Point& other) const
{
	return Point(m_x - other.m_x, m_y - other.m_y);
}

double Point::magnitude() const
{
	return std::sqrt(squaredMagnitude());
}

double Point::squaredMagnitude() const
{
	return static_cast<double>(m_x) * static_cast<double>(m_x) + static_cast<double>(m_y) * static_cast<double>(m_y);
}