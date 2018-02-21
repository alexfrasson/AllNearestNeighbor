#include "../include/KdTree.h"

#include <stdexcept>
#include <iostream>
#include <numeric>
#include <algorithm>
#include <cmath>


KdTree::~KdTree()
{
	freeNodes(m_Root);
}

void KdTree::build(uint8_t leafCapacity, const std::vector<Point>& points)
{
	if (points.empty())
	{
		std::cout << "Empty point set. Will not build." << std::endl;
		return;
	}

	m_LeafCapacity = leafCapacity == 0 ? 1 : leafCapacity;;
	m_Points = points;

	// Find bounding box containing all points.
	m_AABB.min = Point(std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::max());
	m_AABB.max = Point(std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::min());
	for (size_t i = 0; i < m_Points.size(); i++)
	{
		for (size_t j = 0; j < 2; j++)
		{
			if (m_Points[i][j] < m_AABB.min[j])
				m_AABB.min[j] = m_Points[i][j];
			if (m_Points[i][j] > m_AABB.max[j])
				m_AABB.max[j] = m_Points[i][j];
		}
	}

	freeNodes(m_Root);

	// Build tree recursevily.
	m_Root = buildRecursive(0, (uint32_t)m_Points.size(), m_AABB, 0);

#ifdef KDTREE_PARALLEL_BUILD
	// Wait for async builds.
	for (size_t i = 0; i < asyncBuilds.size(); i++)
		asyncBuilds[i].wait();
	asyncBuilds.clear();
#endif
}

KdTreeNode* KdTree::buildRecursive(uint32_t begin, uint32_t end, AABB aabb, int depth)
{
	KdTreeNode* node = new KdTreeNode();

	uint32_t count = end - begin;
	// Reached the leaf capacity. Create leaf node.
	if (count <= (uint32_t)m_LeafCapacity)
	{	
		node->begin = begin;
		node->count = (uint8_t)count;
		return node;
	}

	// We are going to split the node in the axis with largest bound size.
	node->axis = aabb.size().m_x > aabb.size().m_y ? 0 : 1;

	// The points are then sorted based on the chosen axis.
	if (node->axis == 0)
		std::sort(m_Points.begin() + begin, m_Points.begin() + end, [](Point a, Point b) {return a.m_x < b.m_x;});
	else
		std::sort(m_Points.begin() + begin, m_Points.begin() + end, [](Point a, Point b) {return a.m_y < b.m_y;});

	// Points are split in half. The mid point will be contained by the right node.
	uint32_t mid = begin + static_cast<uint32_t>(std::floor(count * 0.5));

	// Colinear points will remain on the right node.
	while (mid > 0)
	{
		if (m_Points[mid][node->axis] != m_Points[mid - 1][node->axis])
			break;
		mid--;
	}

	node->value = m_Points[mid][node->axis];

	// Update bounding box.
	AABB aabbLeft = aabb;
	AABB aabbRight = aabb;

	aabbLeft.max[node->axis] = aabbRight.min[node->axis] = node->value;

	// Create children.
	//
	// This is a quick solution to provide parallel tree builds.
	// A more elaborated solution would be preferred though as this does not accounts for the number of
	// logical processors and the size of the input.
	// This will spawn four async jobs, where each one builds a different subtree on a separated thread.
#ifdef KDTREE_PARALLEL_BUILD
	if (depth == 2)
	{
		auto buildAndAssign = [this](KdTreeNode** node, uint32_t begin, uint32_t end, AABB aabb, int depth) { *node = buildRecursive(begin, end, aabb, depth); };
		asyncBuilds.emplace_back(std::async(std::launch::async, buildAndAssign, &node->left,	begin,	mid, aabbLeft,	depth + 1));
		asyncBuilds.emplace_back(std::async(std::launch::async, buildAndAssign, &node->right,	mid,	end, aabbRight, depth + 1));
	}
	else
#endif
	{
		node->left = buildRecursive(begin, mid, aabbLeft, depth + 1);
		node->right = buildRecursive(mid, end, aabbRight, depth + 1);
	}

	return node;
}

Point KdTree::nearestNeighbor(Point p) const
{
	if (m_Root == nullptr || m_Points.empty())
		throw std::logic_error("KdTree has not been built or is empty.");

	double dist = std::numeric_limits<double>::max();
	Point nearest;

	nearestNeighborRecursive(m_Root, p, nearest, dist);

	return nearest;
}

void KdTree::nearestNeighborRecursive(const KdTreeNode* node, Point& p, Point& nearest, double& dist) const
{
	if (node->isLeaf())
	{
		// Naive search within leaf nodes
		for (uint32_t i = node->begin; i < node->begin + node->count; i++)
		{
			if (m_Points[i] == p)
				continue;

			double d = (p - m_Points[i]).magnitude();
			if (d < dist)
			{
				dist = d;
				nearest = m_Points[i];
			}
		}
		return;
	}

	int32_t pvalue = p[node->axis];

	// Search left first.
	if (pvalue < node->value)
	{
		// Only keep searching if the circle with radius dist touches the splitting plane.
		if (pvalue - dist < node->value)
			nearestNeighborRecursive(node->left, p, nearest, dist);
		if (pvalue + dist >= node->value)
			nearestNeighborRecursive(node->right, p, nearest, dist);
	}
	// Serach right first.
	else
	{
		// Only keep searching if the circle with radius dist touches the splitting plane.
		if (pvalue + dist >= node->value)
			nearestNeighborRecursive(node->right, p, nearest, dist);
		if (pvalue - dist < node->value)
			nearestNeighborRecursive(node->left, p, nearest, dist);
	}
}

void KdTree::freeNodes(KdTreeNode* node)
{
	if (node == nullptr || node->isLeaf())
		return;

	freeNodes(node->left);
	freeNodes(node->right);

	delete node->left;
	delete node->right;
}