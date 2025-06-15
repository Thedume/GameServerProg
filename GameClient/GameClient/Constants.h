#pragma once

// 맵 관련 상수
namespace MapConstants {
    constexpr int MAP_WIDTH = 2000;
    constexpr int MAP_HEIGHT = 2000;
    constexpr int TILE_SIZE = 20;
}

// 뷰 관련 상수
namespace ViewConstants {
    constexpr int VIEW_TILE_WIDTH = 20;
    constexpr int VIEW_TILE_HEIGHT = 20;
}

// 애니메이션 관련 상수
namespace AnimationConstants {
    constexpr int ANIMATION_INTERVAL = 100; // ms
    constexpr int ATTACK_DURATION = 200; // 0.2초만 보여주고 끝
}

// UI 관련 상수
namespace UIConstants {
    constexpr size_t MAX_LOG_LINES = 5;
    constexpr int MSG_LINE_HEIGHT = 18;
    constexpr int MSG_PADDING = 6;
    constexpr int MSG_MAX_WIDTH = 300;
}

// 몬스터 관련 상수
namespace MonsterConstants {
    constexpr int MONSTER_SIGHT_RANGE = 5;
    constexpr int MONSTER_MOVE_INTERVAL = 500;  // ms
    constexpr int MONSTER_ATTACK_INTERVAL = 1000;  // ms
    constexpr int MONSTER_MAX_COUNT = 10;
    constexpr int MONSTER_SPAWN_INTERVAL = 3000;  // 3초 간격
    constexpr int MAX_HP = 50;  // 몬스터의 최대 체력
}

// 플레이어 관련 상수
namespace PlayerConstants {
    constexpr int MAX_HP = 100;
    constexpr int HP_REGEN_INTERVAL = 5000; // 5초마다 HP 회복
    constexpr int HP_REGEN_RATIO = 10; // 현재 HP의 10% 회복
}

// 방향 관련 상수
namespace DirectionConstants {
    enum Direction {
        DIR_DOWN = 0,
        DIR_UP,
        DIR_LEFT,
        DIR_RIGHT
    };
}

// 몬스터 상태 관련 상수
namespace MonsterStateConstants {
    enum MonsterState {
        MONSTER_IDLE,
        MONSTER_CHASE,
        MONSTER_ATTACK
    };
} 