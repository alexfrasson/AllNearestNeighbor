#pragma once
#include <vector>
#include "Point.h"

class Parser
{
public:
	static std::vector<Point> readPoints(const std::string& fileName);
	static std::vector<std::pair<Point, Point>> readSolution(const std::string& fileName);
};