#pragma once

struct Node {
    int x, y;
    int g = 0;  // 시작점에서 현재까지의 비용
    int h = 0;  // 현재에서 목표까지의 예상 비용
    int f = 0;  // g + h
    Node* parent = nullptr;

    Node(int x = 0, int y = 0) : x(x), y(y) {}

    bool operator>(const Node& other) const {
        return f > other.f;
    }

    bool operator==(const Node& other) const {
        return x == other.x && y == other.y;
    }
}; 