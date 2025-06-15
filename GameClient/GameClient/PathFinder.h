#pragma once
#include <vector>
#include <queue>
#include <set>
#include <algorithm>
#include "Constants.h"
#include "Node.h"

class PathFinder {
public:
    static std::vector<std::pair<int, int>> FindPath(int startX, int startY, int targetX, int targetY, const int map[][MapConstants::MAP_WIDTH]);

private:
    static int GoalDistEstimate(int x1, int y1, int x2, int y2) {
        return std::abs(x1 - x2) + std::abs(y1 - y2);
    }
}; 