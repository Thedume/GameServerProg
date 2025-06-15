#include "Player.h"

Player::Player(int x, int y)
    : x(x)
    , y(y)
    , hp(PlayerConstants::MAX_HP)
    , level(1)
    , exp(0)
    , direction(DirectionConstants::DIR_DOWN)
{
}

void Player::Move(int dx, int dy)
{
    x += dx;
    y += dy;
}

void Player::SetPosition(int x, int y)
{
    this->x = x;
    this->y = y;
}

void Player::TakeDamage(int damage)
{
    hp -= damage;
    if (hp < 0) hp = 0;
}

void Player::Heal(int amount)
{
    hp += amount;
    if (hp > PlayerConstants::MAX_HP)
        hp = PlayerConstants::MAX_HP;
}

void Player::Respawn()
{
    x = 1000;
    y = 1000;
    hp = PlayerConstants::MAX_HP;
    exp /= 2; // 경험치 절반 차감
}

void Player::AddExp(int exp)
{
    this->exp += exp;
    while (this->exp >= GetExpToLevelUp(level))
    {
        this->exp -= GetExpToLevelUp(level);
        level++;
        hp = PlayerConstants::MAX_HP;  // 레벨업 시 HP 회복
    }
}

bool Player::CanLevelUp() const
{
    return exp >= GetExpToLevelUp(level);
}

void Player::LevelUp()
{
    if (!CanLevelUp()) return;
    
    level++;
    hp = PlayerConstants::MAX_HP; // 레벨업시 HP 회복
}

int GetExpToLevelUp(int level)
{
    return 100 * (1 << (level - 1)); // 2 ^ (level - 1) * 100
} 