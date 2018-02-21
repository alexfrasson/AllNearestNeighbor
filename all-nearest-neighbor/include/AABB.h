#pragma once

#include "Point.h"

struct AABB
{
	Point min, max;

	AABB() = default;
	AABB(Point minn, Point maxx)
	{
		min = minn;
		max = maxx;
	}

	Point size()
	{
		return max - min;
	}
};
