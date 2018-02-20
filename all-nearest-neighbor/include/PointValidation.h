#pragma once
#include <vector>
#include "Point.h"

class PointValidation
{
public:
	static bool validatePoints(const std::string& solutionFilename, const std::vector<std::pair<Point, Point>>& result);
};