#pragma once
#include "Constants.h"
#include <string>

class Player {
public:
    Player(int startX = 1000, int startY = 1000);
    ~Player() = default;

    // 위치 관련
    void Move(int dx, int dy);
    void SetPosition(int x, int y);
    int GetX() const { return x; }
    int GetY() const { return y; }

    // 상태 관련
    void TakeDamage(int damage);
    void Heal(int amount);
    bool IsDead() const { return hp <= 0; }
    void Respawn();

    // 레벨 관련
    void AddExp(int amount);
    bool CanLevelUp() const;
    void LevelUp();

    // 게터
    int GetHp() const { return hp; }
    int GetMaxHp() const { return PlayerConstants::MAX_HP; }
    int GetLevel() const { return level; }
    int GetExp() const { return exp; }
    DirectionConstants::Direction GetDirection() const { return direction; }
    void SetDirection(DirectionConstants::Direction dir) { direction = dir; }
    static int GetExpToLevelUp(int level) { return 100 * (1 << (level - 1)); /* 2 ^ (level - 1) * 100 */ }

private:
    int x, y;
    int hp;
    int level;
    int exp;
    DirectionConstants::Direction direction;
}; 