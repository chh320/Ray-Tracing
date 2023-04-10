#ifndef BVH_H
#define BVH_H

#include <vector>
#include <algorithm>
#include <glm/glm.hpp>
#include "triangle.h"

using namespace std;

// 按照三角形中心排序 -- 比较函数
bool cmpx(const Triangle& t1, const Triangle& t2) {
	return t1.center.x < t2.center.x;
}
bool cmpy(const Triangle& t1, const Triangle& t2) {
	return t1.center.y < t2.center.y;
}
bool cmpz(const Triangle& t1, const Triangle& t2) {
	return t1.center.z < t2.center.z;
}

class Bvh {
public:
	struct BVHNode
	{
		int left;
		int right;
		int padding1;

		int n;
		int index;
		int padding2;

		glm::vec3 Pmin;
		glm::vec3 Pmax;
	};

	Bvh(){ }
	Bvh(std::vector<Triangle>& triangles, int l, int r, int n) {
		root = buildBVHwithSAH(triangles, nodes, l, r, n);
	}

	int buildBVH(std::vector<Triangle>& triangles, std::vector<BVHNode>& nodes, int l, int r, int n) {
		if (l > r) return 0;

		// 注：
		// 此处不可通过指针，引用等方式操作，必须用 nodes[id] 来操作
		// 因为 std::vector<> 扩容时会拷贝到更大的内存，那么地址就改变了
		// 而指针，引用均指向原来的内存，所以会发生错误
		nodes.push_back(BVHNode());
		int id = nodes.size() - 1;   // 注意： 先保存索引
		nodes[id].left = nodes[id].right = nodes[id].n = nodes[id].index = 0;
		nodes[id].Pmin = glm::vec3(1145141919, 1145141919, 1145141919);
		nodes[id].Pmax = glm::vec3(-1145141919, -1145141919, -1145141919);

		// 计算 AABB
		for (int i = l; i <= r; i++) {
			// 最小点 AA
			float minx = min(triangles[i].p1.x, min(triangles[i].p2.x, triangles[i].p3.x));
			float miny = min(triangles[i].p1.y, min(triangles[i].p2.y, triangles[i].p3.y));
			float minz = min(triangles[i].p1.z, min(triangles[i].p2.z, triangles[i].p3.z));
			nodes[id].Pmin.x = min(nodes[id].Pmin.x, minx);
			nodes[id].Pmin.y = min(nodes[id].Pmin.y, miny);
			nodes[id].Pmin.z = min(nodes[id].Pmin.z, minz);
			// 最大点 BB
			float maxx = max(triangles[i].p1.x, max(triangles[i].p2.x, triangles[i].p3.x));
			float maxy = max(triangles[i].p1.y, max(triangles[i].p2.y, triangles[i].p3.y));
			float maxz = max(triangles[i].p1.z, max(triangles[i].p2.z, triangles[i].p3.z));
			nodes[id].Pmax.x = max(nodes[id].Pmax.x, maxx);
			nodes[id].Pmax.y = max(nodes[id].Pmax.y, maxy);
			nodes[id].Pmax.z = max(nodes[id].Pmax.z, maxz);
		}

		// 不多于 n 个三角形 返回叶子节点
		if ((r - l + 1) <= n) {
			nodes[id].n = r - l + 1;
			nodes[id].index = l;
			return id;
		}

		// 否则递归建树
		float lenx = nodes[id].Pmax.x - nodes[id].Pmin.x;
		float leny = nodes[id].Pmax.y - nodes[id].Pmin.y;
		float lenz = nodes[id].Pmax.z - nodes[id].Pmin.z;
		// 按 x 划分
		if (lenx >= leny && lenx >= lenz)
			std::sort(triangles.begin() + l, triangles.begin() + r + 1, cmpx);
		// 按 y 划分
		if (leny >= lenx && leny >= lenz)
			std::sort(triangles.begin() + l, triangles.begin() + r + 1, cmpy);
		// 按 z 划分
		if (lenz >= lenx && lenz >= leny)
			std::sort(triangles.begin() + l, triangles.begin() + r + 1, cmpz);
		// 递归
		int mid = (l + r) / 2;
		int left = buildBVH(triangles, nodes, l, mid, n);
		int right = buildBVH(triangles, nodes, mid + 1, r, n);

		nodes[id].left = left;
		nodes[id].right = right;

		return id;
	}
	
	int buildBVHwithSAH(std::vector<Triangle>& triangles, std::vector<BVHNode>& nodes, int left, int right, int n) {
		if (left > right) return 0;

		nodes.push_back(BVHNode());
		int id = nodes.size() - 1;
		nodes[id].left = nodes[id].right = nodes[id].n = nodes[id].index = 0;
		nodes[id].Pmin = glm::vec3(FLT_MAX, FLT_MAX, FLT_MAX);
		nodes[id].Pmax = glm::vec3(-FLT_MAX, -FLT_MAX, -FLT_MAX);

		for (int i = left; i <= right; i++) {
			float minx = std::min(triangles[i].p1.x, std::min(triangles[i].p2.x, triangles[i].p3.x));
			float miny = std::min(triangles[i].p1.y, std::min(triangles[i].p2.y, triangles[i].p3.y));
			float minz = std::min(triangles[i].p1.z, std::min(triangles[i].p2.z, triangles[i].p3.z));
			nodes[id].Pmin.x = std::min(minx, nodes[id].Pmin.x);
			nodes[id].Pmin.y = std::min(miny, nodes[id].Pmin.y);
			nodes[id].Pmin.z = std::min(minz, nodes[id].Pmin.z);

			float maxx = std::max(triangles[i].p1.x, std::max(triangles[i].p2.x, triangles[i].p3.x));
			float maxy = std::max(triangles[i].p1.y, std::max(triangles[i].p2.y, triangles[i].p3.y));
			float maxz = std::max(triangles[i].p1.z, std::max(triangles[i].p2.z, triangles[i].p3.z));
			nodes[id].Pmax.x = std::max(maxx, nodes[id].Pmax.x);
			nodes[id].Pmax.y = std::max(maxy, nodes[id].Pmax.y);
			nodes[id].Pmax.z = std::max(maxz, nodes[id].Pmax.z);
		}

		if ((right - left + 1) <= n) {
			nodes[id].n = right - left + 1;
			nodes[id].index = left;
			return id;
		}

		float Cost = FLT_MAX;
		int Axis = 0;
		int Split = (left + right) / 2;
		for (int axis = 0; axis < 3; axis++) {
			if (axis == 0) std::sort(triangles.begin() + left, triangles.begin() + right + 1, cmpx);
			if (axis == 1) std::sort(triangles.begin() + left, triangles.begin() + right + 1, cmpy);
			if (axis == 2) std::sort(triangles.begin() + left, triangles.begin() + right + 1, cmpz);

			std::vector<glm::vec3> leftMax(right - left + 1, glm::vec3(-FLT_MAX));
			std::vector<glm::vec3> leftMin(right - left + 1, glm::vec3( FLT_MAX));

			for (int i = left; i <= right; i++) {
				Triangle& t = triangles[i];
				int bias = (i == left) ? 0 : 1;
				leftMax[i - left].x = std::max(leftMax[i - left - bias].x, std::max(t.p1.x, std::max(t.p2.x, t.p3.x)));
				leftMax[i - left].y = std::max(leftMax[i - left - bias].y, std::max(t.p1.y, std::max(t.p2.y, t.p3.y)));
				leftMax[i - left].z = std::max(leftMax[i - left - bias].z, std::max(t.p1.z, std::max(t.p2.z, t.p3.z)));
			
				leftMin[i - left].x = std::min(leftMin[i - left - bias].x, std::min(t.p1.x, std::min(t.p2.x, t.p3.x)));
				leftMin[i - left].y = std::min(leftMin[i - left - bias].y, std::min(t.p1.y, std::min(t.p2.y, t.p3.y)));
				leftMin[i - left].z = std::min(leftMin[i - left - bias].z, std::min(t.p1.z, std::min(t.p2.z, t.p3.z)));
			}

			std::vector<glm::vec3> rightMax(right - left + 1, glm::vec3(-FLT_MAX));
			std::vector<glm::vec3> rightMin(right - left + 1, glm::vec3(FLT_MAX));

			for (int i = right; i >= left; i--) {
				Triangle& t = triangles[i];
				int bias = (i == right) ? 0 : 1;
				rightMax[i - left].x = std::max(rightMax[i - left + bias].x, std::max(t.p1.x, std::max(t.p2.x, t.p3.x)));
				rightMax[i - left].y = std::max(rightMax[i - left + bias].y, std::max(t.p1.y, std::max(t.p2.y, t.p3.y)));
				rightMax[i - left].z = std::max(rightMax[i - left + bias].z, std::max(t.p1.z, std::max(t.p2.z, t.p3.z)));

				rightMin[i - left].x = std::min(rightMin[i - left + bias].x, std::min(t.p1.x, std::min(t.p2.x, t.p3.x)));
				rightMin[i - left].y = std::min(rightMin[i - left + bias].y, std::min(t.p1.y, std::min(t.p2.y, t.p3.y)));
				rightMin[i - left].z = std::min(rightMin[i - left + bias].z, std::min(t.p1.z, std::min(t.p2.z, t.p3.z)));
			}

			float cost = FLT_MAX;
			int split = left;
			for (int i = left; i <= right - 1; i++) {
				float lenx, leny, lenz;
				
				// leftCost [left, i]
				glm::vec3 leftPmin = leftMin[i - left];
				glm::vec3 leftPmax = leftMax[i - left];
				lenx = leftPmax.x - leftPmin.x;
				leny = leftPmax.y - leftPmin.y;
				lenz = leftPmax.z - leftPmin.z;
				float leftSurface = 2.0 * (lenx * leny + leny * lenz + lenz * lenx);
				float leftCost = leftSurface * (i - left + 1);
				
				// rightCost [i+1, right]
				glm::vec3 rightPmin = rightMin[i + 1 - left];
				glm::vec3 rightPmax = rightMax[i + 1 - left];
				lenx = rightPmax.x - rightPmin.x;
				leny = rightPmax.y - rightPmin.y;
				lenz = rightPmax.z - rightPmin.z;
				float rightSurface = 2.0 * (lenx * leny + leny * lenz + lenz * lenx);
				float rightCost = rightSurface * (right - i);

				float totalCost = leftCost + rightCost;
				if (totalCost < cost) {
					cost = totalCost;
					split = i;
				}
			}
			if (cost < Cost) {
				Cost = cost;
				Axis = axis;
				Split = split;
			}
		}
		if (Axis == 0)
			std::sort(triangles.begin() + left, triangles.begin() + right + 1, cmpx);
		if (Axis == 1)
			std::sort(triangles.begin() + left, triangles.begin() + right + 1, cmpy);
		if (Axis == 2)
			std::sort(triangles.begin() + left, triangles.begin() + right + 1, cmpz);

		nodes[id].left = buildBVHwithSAH(triangles, nodes, left, Split, n);
		nodes[id].right = buildBVHwithSAH(triangles, nodes, Split + 1, right, n);

		return id;
	}

	int root;
	std::vector<BVHNode> nodes;
};
#endif // !BVH_H
