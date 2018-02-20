#pragma once

#include "Point.h"
#include <vector>

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

struct KdTreeNode
{
	KdTreeNode* left = nullptr;
	KdTreeNode* right = nullptr;

	union
	{
		// When leaf. The starting indice of the points inside this node.
		uint32_t begin;
		// The value of the spliting plane.
		uint32_t value;
	};

	union
	{
		// When leaf. The count of points inside the node.
		uint16_t count;
		// The axis in which the node has been split.
		// 0 - x
		// 1 - y
		uint16_t axis;
	};

	inline bool isToTheLeft(const Point& p) const
	{
		return (axis == 0 ? p.m_x : p.m_y) < value;
	}

	inline bool isLeaf() const
	{
		return left == nullptr && right == nullptr;
	}
};

class KdTree
{
public:
	AABB m_AABB;

	uint16_t m_LeafCapacity;
	std::vector<Point> m_Points;
	KdTreeNode* m_Root = nullptr;

	KdTree()
	{
	}
	~KdTree();
	void build(int leafCapacity, const std::vector<Point>& points);
	Point nearestNeighbor(Point p);

private:
	// Builds the tree recursively. The range [begin, end) represent the points contained within the node. 
	// AABB is the bound containing all points within the range [begin, end).
	KdTreeNode* buildRecursive(uint32_t begin, uint32_t end, AABB aabb);
	void nearestNeighborRecursive(const KdTreeNode* node, const Point p, Point& closest, double& dist);
	void freeNodes(KdTreeNode* node);
};