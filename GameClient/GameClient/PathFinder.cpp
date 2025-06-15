#include "PathFinder.h"

std::vector<std::pair<int, int>> PathFinder::FindPath(int startX, int startY, int targetX, int targetY, const int map[][MapConstants::MAP_WIDTH])
{
    auto cmp = [](const Node* a, const Node* b) { return a->f > b->f; };
    std::priority_queue<Node*, std::vector<Node*>, decltype(cmp)> open(cmp);
    std::set<std::pair<int, int>> closed;

    Node* start = new Node(startX, startY);
    start->g = 0;
    start->h = GoalDistEstimate(startX, startY, targetX, targetY);
    start->f = start->g + start->h;
    start->parent = nullptr;
    open.push(start);

    const int dx[] = { 0, -1, 1, 0 };
    const int dy[] = { -1, 0, 0, 1 };

    while (!open.empty()) {
        Node* current = open.top(); open.pop();

        if (current->x == targetX && current->y == targetY) {
            std::vector<std::pair<int, int>> path;
            while (current->parent) {
                path.emplace_back(current->x, current->y);
                current = current->parent;
            }
            std::reverse(path.begin(), path.end());
            return path;
        }

        closed.insert({ current->x, current->y });

        for (int i = 0; i < 4; ++i) {
            int nx = current->x + dx[i];
            int ny = current->y + dy[i];

            if (nx < 0 || nx >= MapConstants::MAP_WIDTH || ny < 0 || ny >= MapConstants::MAP_HEIGHT)
                continue;
            if (map[ny][nx] == 1)
                continue;
            if (closed.count({ nx, ny }))
                continue;

            int newg = current->g + 1;

            Node* neighbor = new Node(nx, ny);
            neighbor->g = newg;
            neighbor->h = GoalDistEstimate(nx, ny, targetX, targetY);
            neighbor->f = neighbor->g + neighbor->h;
            neighbor->parent = current;

            open.push(neighbor);
        }
    }

    return {}; // 실패 시 빈 경로
} 