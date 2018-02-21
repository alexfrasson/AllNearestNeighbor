#pragma once

#define KDTREE_PARALLEL_BUILD


#include "Point.h"
#include "AABB.h"
#include <vector>

#ifdef KDTREE_PARALLEL_BUILD
#include <future>
#endif


struct KdTreeNode
{
	KdTreeNode* left = nullptr;
	KdTreeNode* right = nullptr;

	union
	{
		// When a leaf. The starting index of the points contained by the node.
		uint32_t begin;
		// The value of the spliting plane.
		int32_t value;
	};

	union
	{
		// When a leaf. The count of points contained by the node.
		uint8_t count;
		// The axis in which the node has been split.
		// 0 - x
		// 1 - y
		uint8_t axis;
	};

	inline bool isLeaf() const
	{
		return left == nullptr && right == nullptr;
	}
};

class KdTree
{
public:
	// The point set's AABB.
	AABB m_AABB;
	KdTreeNode* m_Root = nullptr;
	
private:
	std::vector<Point> m_Points;
	uint8_t m_LeafCapacity;

#ifdef KDTREE_PARALLEL_BUILD
	std::vector<std::future<void>> asyncBuilds;
#endif // KDTREE_PARALLEL_BUILD

public:
	KdTree() = default;
	~KdTree();

	// Creates an internal copy of the point set and builds the tree with it.
	void build(uint8_t leafCapacity, const std::vector<Point>& points);
	Point nearestNeighbor(Point p) const;

private:
	// Builds the tree recursively. The range [begin, end) represent the points contained within the node. 
	// AABB is the bound containing all points within the range [begin, end).
	KdTreeNode* buildRecursive(uint32_t begin, uint32_t end, AABB aabb, int depth);
	void nearestNeighborRecursive(const KdTreeNode* node, Point& p, Point& nearest, double& dist) const;
	void freeNodes(KdTreeNode* node);
};