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

void KdTree::build(int leafCapacity, const std::vector<Point>& points)
{
	m_LeafCapacity = leafCapacity;
	m_Points = points;

	// Find bounding box containing all points.
	m_AABB.min = Point(std::numeric_limits<int32_t>::max(), std::numeric_limits<int32_t>::max());
	m_AABB.max = Point(std::numeric_limits<int32_t>::min(), std::numeric_limits<int32_t>::min());
	for (size_t i = 0; i < points.size(); i++)
	{
		if (points[i].m_x < m_AABB.min.m_x)
			m_AABB.min.m_x = points[i].m_x;
		if (points[i].m_x > m_AABB.max.m_x)
			m_AABB.max.m_x = points[i].m_x;
		if (points[i].m_y < m_AABB.min.m_y)
			m_AABB.min.m_y = points[i].m_y;
		if (points[i].m_y > m_AABB.max.m_y)
			m_AABB.max.m_y = points[i].m_y;
	}

	// Build tree recursevily.
	m_Root = buildRecursive(0, m_Points.size(), m_AABB);
}


Point KdTree::nearestNeighbor(Point p)
{
	if (m_Root == nullptr || m_Points.size() == 0)
		throw std::logic_error("KdTree has not been built or is empty.");

	double dist = std::numeric_limits<double>::max();
	Point closest;

	nearestNeighborRecursive(m_Root, p, closest, dist);

	return closest;
}

void KdTree::nearestNeighborRecursive(const KdTreeNode* node, const Point p, Point& closest, double& dist)
{
	if (node->isLeaf())
	{
		for (uint16_t i = node->begin; i < node->begin + node->count; i++)
		{
			if (m_Points[i] == p)
				continue;

			double d = (p - m_Points[i]).magnitude();
			if (d < dist)
			{
				dist = d;
				closest = m_Points[i];
			}
		}

		return;
	}

	int32_t pvalue = (node->axis == 0 ? p.m_x : p.m_y);

	if (pvalue < node->value)
	{
		if (pvalue - dist < node->value)
			nearestNeighborRecursive(node->left, p, closest, dist);
		if (pvalue + dist >= node->value)
			nearestNeighborRecursive(node->right, p, closest, dist);
	}
	else
	{
		if (pvalue + dist >= node->value)
			nearestNeighborRecursive(node->right, p, closest, dist);
		if (pvalue - dist < node->value)
			nearestNeighborRecursive(node->left, p, closest, dist);
	}
}


KdTreeNode* KdTree::buildRecursive(uint32_t begin, uint32_t end, AABB aabb)
{
	if (end < begin)
		std::cout << "Should not happen." << std::endl;

	KdTreeNode* node = new KdTreeNode();

	uint32_t count = end - begin;
	// Reached the leaf capacity. Create leaf node.
	if (count <= m_LeafCapacity)
	{	
		node->begin = begin;
		node->count = count;
		return node;
	}

	// We are going to split the node in the axis with largest bound size.
	// The points are then sorted based in the chosen axis.
	node->axis = aabb.size().m_x > aabb.size().m_y ? 0 : 1;
	if (node->axis == 0)
		std::sort(m_Points.begin() + begin, m_Points.begin() + end, [](Point a, Point b) {return a.m_x < b.m_x;});
	else
		std::sort(m_Points.begin() + begin, m_Points.begin() + end, [](Point a, Point b) {return a.m_y < b.m_y;});

	// Points are split in half. The mid point will be contained by the right node.
	uint32_t mid = begin + std::floor(count * 0.5f);
	
	// Update bounding box
	AABB aabbLeft = aabb;
	AABB aabbRight = aabb;

	int32_t midvalue = (node->axis == 0 ? m_Points[mid].m_x : m_Points[mid].m_y);

	if (node->axis == 0)
		aabbLeft.max.m_x = aabbRight.min.m_x = midvalue;
	else
		aabbLeft.max.m_y = aabbRight.min.m_y = midvalue;

	// Create children
	node->left = buildRecursive(begin, mid, aabbLeft);
	node->right = buildRecursive(mid, end, aabbRight);

	node->value = midvalue;

	return node;
}

void KdTree::freeNodes(KdTreeNode* node)
{
	if (node->isLeaf())
		return;

	freeNodes(node->left);
	freeNodes(node->right);

	delete node->left;
	delete node->right;
}