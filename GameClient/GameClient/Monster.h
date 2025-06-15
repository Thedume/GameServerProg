#pragma once
#include "Constants.h"
#include "Node.h"
#include <vector>
#include <utility>
#include <queue>
#include <set>

inline int GoalDistEstimate(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);
}

class Monster {
public:
    Monster(int startX, int startY, int initialHp = MonsterConstants::MAX_HP);
    ~Monster() = default;

    // 위치 관련
    void Move(int newX, int newY);
    void SetPosition(int x, int y);
    int GetX() const { return x; }
    int GetY() const { return y; }

    // 상태 관련
    void TakeDamage(int damage);
    bool IsAlive() const { return hp > 0; }
    void SetAlive(bool alive) { hp = alive ? MonsterConstants::MAX_HP : 0; }
    int GetHp() const { return hp; }

    // 방향 관련
    DirectionConstants::Direction GetDirection() const { return direction; }
    void SetDirection(DirectionConstants::Direction dir) { direction = dir; }

    // 상태 관련
    int GetState() const { return state; }
    void SetState(int newState) { state = newState; }

    // 애니메이션 관련
    int GetAnimFrame() const { return animFrame; }
    void SetAnimFrame(int frame) { animFrame = frame; }
    void NextAnimFrame() { animFrame = (animFrame + 1) % 4; }

    // 공격 관련
    void UpdateAttackCooldown(int elapsed);
    bool CanAttack() const { return attackCooldown <= 0; }
    void ResetAttackCooldown() { attackCooldown = MonsterConstants::MONSTER_ATTACK_INTERVAL; }

    // AI 관련
    std::vector<std::pair<int, int>> FindPathToTarget(int targetX, int targetY, const int map[][MapConstants::MAP_WIDTH]);
    bool IsInRange(int targetX, int targetY, int range) const;

private:
    int x, y;
    int hp = MonsterConstants::MAX_HP;
    int state = MonsterStateConstants::MONSTER_IDLE;
    DirectionConstants::Direction direction = DirectionConstants::DIR_DOWN;
    int animFrame = 0;
    int attackCooldown = 0;
}; 