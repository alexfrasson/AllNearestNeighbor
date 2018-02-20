#include "../include/Parser.h"
#include <fstream>

std::vector<Point> Parser::readPoints(const std::string& fileName)
{
	std::vector<Point> points;

	std::fstream file;
	
	file.open(fileName, std::fstream::in);

	while (!file.eof())
	{
		Point point;
		file >> point;

		if (!point.m_name.empty())
		{
			points.push_back(point);
		}
	}
	
	file.close();

	return points;
}

std::vector<std::pair<Point, Point>> Parser::readSolution(const std::string& fileName)
{
	std::vector<std::pair<Point, Point>> solution;

	std::fstream file;
	file.open(fileName, std::fstream::in);

	while (!file.eof())
	{
		Point point0, point1;
		file >> point0;
		file >> point1;

		if (!point0.m_name.empty())
		{
			solution.push_back(std::pair<Point, Point>(point0, point1));
		}
	}

	file.close();

	return solution;
}