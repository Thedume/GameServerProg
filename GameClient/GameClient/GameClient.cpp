#include <windows.h>
#include <gdiplus.h>
#include <queue>
#include <vector>
#include <set>
#include <deque>
#include <string>
#include <chrono>
#include "Constants.h"
#include "Player.h"
#include "Monster.h"
#include "PathFinder.h"
#include "Node.h"
#include <iostream>
#include <conio.h>
#include <algorithm>
#include <random>

#pragma comment(lib, "Gdiplus.lib")
using namespace Gdiplus;


// ====== 설정 상수 ======
const int MAP_WIDTH = MapConstants::MAP_WIDTH;
const int MAP_HEIGHT = MapConstants::MAP_HEIGHT;
const int TILE_SIZE = MapConstants::TILE_SIZE;

const int VIEW_TILE_WIDTH = ViewConstants::VIEW_TILE_WIDTH;
const int VIEW_TILE_HEIGHT = ViewConstants::VIEW_TILE_HEIGHT;

const int ANIMATION_INTERVAL = AnimationConstants::ANIMATION_INTERVAL; // ms

const size_t MAX_LOG_LINES = UIConstants::MAX_LOG_LINES;
const int MSG_LINE_HEIGHT = UIConstants::MSG_LINE_HEIGHT;
const int MSG_PADDING = UIConstants::MSG_PADDING;
const int MSG_MAX_WIDTH = UIConstants::MSG_MAX_WIDTH;

const int MONSTER_SIGHT_RANGE = MonsterConstants::MONSTER_SIGHT_RANGE;
const int MONSTER_MOVE_INTERVAL = MonsterConstants::MONSTER_MOVE_INTERVAL;  // ms
const int MONSTER_ATTACK_INTERVAL = MonsterConstants::MONSTER_ATTACK_INTERVAL;
const int MONSTER_MAX_COUNT = MonsterConstants::MONSTER_MAX_COUNT;
const int MONSTER_SPAWN_INTERVAL = MonsterConstants::MONSTER_SPAWN_INTERVAL; // 3초 간격

// ====== 전역 변수 ======
HINSTANCE g_hInst;
HWND g_hWnd;

Player g_player;

DirectionConstants::Direction g_playerDir = DirectionConstants::DIR_DOWN;

enum MonsterState {
    MONSTER_IDLE,
    MONSTER_CHASE,
    MONSTER_ATTACK
};

int g_map[MAP_HEIGHT][MAP_WIDTH] = { 0 };
bool g_keyStates[256] = { false };

ULONG_PTR g_gdiplusToken;
Image* g_idleImage = nullptr;
Image* g_walkImage = nullptr;
Image* g_attackImage = nullptr;
Image* g_swordImage = nullptr;
Image* g_monsterIdleImage = nullptr;
Image* g_monsterWalkImage = nullptr;

bool g_isMoving = false;
int g_walkFrame = 0;
UINT_PTR g_walkTimer = 1;

bool g_isAttacking = false;
UINT_PTR g_attackTimer = 2;
const int ATTACK_DURATION = AnimationConstants::ATTACK_DURATION; // 0.2초만 보여주고 끝

UINT_PTR g_monsterAnimTimer = 3;
UINT_PTR g_monsterAiTimer = 5;
UINT_PTR g_monsterSpawnTimer = 6;
bool g_spawnedMonsters = false;

UINT_PTR g_hpRegenTimer = 4;

bool g_isChatting = false;
std::wstring g_chatBuffer;

int g_swordX = -1, g_swordY = -1;

std::vector<Monster> g_monsters;

std::deque<std::wstring> g_messageLog;

std::chrono::steady_clock::time_point g_lastMessageTime;

// ====== 함수 선언 ======
void InitWindow(HINSTANCE hInstance, int nCmdShow);
void GameInit();
void AddMessage(const std::wstring& msg);
void HandlePlayerDeath();
void SpawnMonster();
void CleanupDeadMonsters();
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// ====== WinMain ======
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    g_hInst = hInstance;
    InitWindow(hInstance, nCmdShow);
    GameInit();

    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    GdiplusShutdown(g_gdiplusToken);
    return (int)msg.wParam;
}

// ====== 윈도우 초기화 ======
void InitWindow(HINSTANCE hInstance, int nCmdShow)
{
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = TEXT("MMORPGClientClass");
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);

    RegisterClass(&wc);

    g_hWnd = CreateWindow(
        TEXT("MMORPGClientClass"), TEXT("MMORPG Client"),
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT,
        VIEW_TILE_WIDTH * TILE_SIZE + 16, VIEW_TILE_HEIGHT * TILE_SIZE + 39,
        nullptr, nullptr, hInstance, nullptr);

    ShowWindow(g_hWnd, nCmdShow);
    UpdateWindow(g_hWnd);
}

// ====== 게임 초기화 ======
void GameInit()
{
    OutputDebugString(L"[Init] GameInit 진입\n");

    // GDI+
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup(&g_gdiplusToken, &gdiplusStartupInput, nullptr);

    // 이미지 로딩
    g_idleImage = new Image(L"..\\Resource\\Actor\\Characters\\Knight\\SeparateAnim\\Idle.png");
    g_walkImage = new Image(L"..\\Resource\\Actor\\Characters\\Knight\\SeparateAnim\\Walk.png");
    g_attackImage = new Image(L"..\\Resource\\Actor\\Characters\\Knight\\SeparateAnim\\Attack.png");
    g_swordImage = new Image(L"..\\Resource\\Items\\Weapons\\Sword2\\Sword.png");

    g_monsterIdleImage = new Image(L"..\\Resource\\Actor\\Monsters\\Bear\\Bear_idle.png");
    g_monsterWalkImage = new Image(L"..\\Resource\\Actor\\Monsters\\Bear\\Bear_anim.png");

    if (g_idleImage->GetLastStatus() != Ok)
        MessageBox(NULL, TEXT("Idle.png 로드 실패!"), TEXT("오류"), MB_ICONERROR);
    if (g_walkImage->GetLastStatus() != Ok)
        MessageBox(NULL, TEXT("Walk.png 로드 실패!"), TEXT("오류"), MB_ICONERROR);
    if (g_attackImage->GetLastStatus() != Ok)
        MessageBox(NULL, TEXT("Attack.png 로드 실패!"), TEXT("오류"), MB_ICONERROR);
    if (g_swordImage->GetLastStatus() != Ok)
        MessageBox(NULL, TEXT("Sword.png 로드 실패!"), TEXT("오류"), MB_ICONERROR);

    if (g_monsterIdleImage->GetLastStatus() != Ok)
        MessageBox(NULL, TEXT("Monster_Idle.png 로드 실패!"), TEXT("오류"), MB_ICONERROR);
    if (g_monsterWalkImage->GetLastStatus() != Ok)
        MessageBox(NULL, TEXT("Monster_Walk.png 로드 실패!"), TEXT("오류"), MB_ICONERROR);

    // 장애물 예시
    for (int i = 0; i < 100; ++i) {
        g_map[1000][990 + i] = 1;
        g_map[990 + i][1000] = 1;
    }

    SetTimer(g_hWnd, g_monsterAnimTimer, AnimationConstants::ANIMATION_INTERVAL, NULL);
    SetTimer(g_hWnd, g_monsterAiTimer, MonsterConstants::MONSTER_MOVE_INTERVAL, NULL);
    SetTimer(g_hWnd, g_hpRegenTimer, PlayerConstants::HP_REGEN_INTERVAL, NULL);
    SetTimer(g_hWnd, g_monsterSpawnTimer, MonsterConstants::MONSTER_SPAWN_INTERVAL, NULL);
}

void AddMessage(const std::wstring& msg)
{
    g_messageLog.push_back(msg);
    if (g_messageLog.size() > MAX_LOG_LINES)
        g_messageLog.pop_front();

    g_lastMessageTime = std::chrono::steady_clock::now();
}

void HandlePlayerDeath()
{
    AddMessage(L"플레이어가 사망했습니다.");
    g_player.Respawn();
    AddMessage(L"HP가 회복되어 부활했습니다.");
    InvalidateRect(g_hWnd, NULL, TRUE);
}

void SpawnMonster()
{
    const int spawnCount = 10;
    int spawned = 0;

    while (spawned < spawnCount) {
        int mx = 900 + rand() % 201;
        int my = 900 + rand() % 201;

        if (g_map[my][mx] != 0)
            continue;

        g_monsters.emplace_back(mx, my);
        ++spawned;
    }

    AddMessage(L"몬스터들이 등장했습니다!");
    g_spawnedMonsters = true;
}

void CleanupDeadMonsters()
{
    g_monsters.erase(
        std::remove_if(g_monsters.begin(), g_monsters.end(),
            [](const Monster& m) { return !m.IsAlive(); }),
        g_monsters.end());
}

// ====== 메시지 처리 ======
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_KEYDOWN:
        if (g_isChatting && wParam != VK_RETURN && wParam != VK_BACK)
            return 0;

        if (wParam == VK_ESCAPE) {
            PostQuitMessage(0);
            return 0;
        }

        if (!g_keyStates[wParam]) {
            g_keyStates[wParam] = true;

            switch (wParam)
            {
            case VK_UP:    g_player.SetDirection(DirectionConstants::DIR_UP); break;
            case VK_DOWN:  g_player.SetDirection(DirectionConstants::DIR_DOWN); break;
            case VK_LEFT:  g_player.SetDirection(DirectionConstants::DIR_LEFT); break;
            case VK_RIGHT: g_player.SetDirection(DirectionConstants::DIR_RIGHT); break;
            }

            SetTimer(hWnd, g_walkTimer, AnimationConstants::ANIMATION_INTERVAL, NULL);
        }

        if (wParam == 'A' && !g_isAttacking) {
            g_isAttacking = true;
            SetTimer(hWnd, g_attackTimer, AnimationConstants::ATTACK_DURATION, NULL);

            switch (g_player.GetDirection())
            {
            case DirectionConstants::DIR_DOWN:  g_swordX = g_player.GetX();      g_swordY = g_player.GetY() + 1; break;
            case DirectionConstants::DIR_LEFT:  g_swordX = g_player.GetX() - 1;  g_swordY = g_player.GetY();     break;
            case DirectionConstants::DIR_UP:    g_swordX = g_player.GetX();      g_swordY = g_player.GetY() - 1;     break;
            case DirectionConstants::DIR_RIGHT: g_swordX = g_player.GetX() + 1;  g_swordY = g_player.GetY();     break;
            }

            // 기존 몬스터 공격 처리 (4방향)
            int dx[] = { 0, 0, -1, 1 }; // DOWN, UP, LEFT, RIGHT
            int dy[] = { 1, -1, 0, 0 };

            int tx = g_player.GetX() + dx[g_player.GetDirection()];
            int ty = g_player.GetY() + dy[g_player.GetDirection()];

            g_swordX = tx;
            g_swordY = ty;

            // 몬스터 한 칸만 체크
            for (auto& m : g_monsters) {
                if (m.IsAlive() && m.GetX() == tx && m.GetY() == ty) {
                    m.TakeDamage(10);

                    AddMessage(L"몬스터에게 10의 데미지를 입혔습니다.");
                    if (m.GetHp() <= 0) {
                        m.SetAlive(false);
                        AddMessage(L"몬스터를 처치했습니다!");
                        g_player.AddExp(50);
                    }
                }
            }

            InvalidateRect(hWnd, NULL, TRUE);
        }

        // 테스트: K 키 누르면 데미지
        if (wParam == 'K') { 
            g_player.TakeDamage(30);
            AddMessage(L"플레이어가 30의 데미지를 입었습니다.");
            if (g_player.IsDead()) {
                HandlePlayerDeath();
            }
            InvalidateRect(hWnd, NULL, TRUE);
        }

        // 테스트 : M 키 누르면 몹 소환
        if (wParam == 'M') {
            int mx = g_player.GetX() + 1;
            int my = g_player.GetY();

            if (mx >= MAP_WIDTH || my >= MAP_HEIGHT || g_map[my][mx] == 1) {
                AddMessage(L"유효하지 않은 위치에 몬스터를 생성할 수 없습니다.");
                break;
            }

            g_monsters.emplace_back(mx, my);
            AddMessage(L"수동으로 몬스터를 생성했습니다.");
            InvalidateRect(hWnd, NULL, TRUE);
        }

        if (wParam == 'T' && !g_isChatting) {
            g_isChatting = true;
            g_chatBuffer.clear();
            InvalidateRect(hWnd, NULL, TRUE);
        }

        break;

    case WM_KEYUP:
        g_keyStates[wParam] = false;
        break;

    case WM_TIMER:
        if (wParam == g_attackTimer) {
            g_isAttacking = false;
            g_swordX = g_swordY = -1; // 검 제거
            KillTimer(hWnd, g_attackTimer);
            InvalidateRect(hWnd, NULL, TRUE);
        }

        if (wParam == g_walkTimer)
        {
            bool moved = false;

            if (g_keyStates[VK_UP] && g_player.GetY() > 0) {
                g_player.Move(0, -1); g_player.SetDirection(DirectionConstants::DIR_UP); moved = true;
            }
            if (g_keyStates[VK_DOWN] && g_player.GetY() < MapConstants::MAP_HEIGHT - 1) {
                g_player.Move(0, 1); g_player.SetDirection(DirectionConstants::DIR_DOWN); moved = true;
            }
            if (g_keyStates[VK_LEFT] && g_player.GetX() > 0) {
                g_player.Move(-1, 0); g_player.SetDirection(DirectionConstants::DIR_LEFT); moved = true;
            }
            if (g_keyStates[VK_RIGHT] && g_player.GetX() < MapConstants::MAP_WIDTH - 1) {
                g_player.Move(1, 0); g_player.SetDirection(DirectionConstants::DIR_RIGHT); moved = true;
            }

            if (moved) {
                g_isMoving = true;
                g_walkFrame = (g_walkFrame + 1) % 4;
            }
            else {
                g_isMoving = false;
                KillTimer(hWnd, g_walkTimer);
            }

            InvalidateRect(hWnd, NULL, TRUE);
        }

        if (wParam == g_monsterAnimTimer) {
            for (auto& m : g_monsters) {
                if (!m.IsAlive()) continue;
                m.NextAnimFrame();
            }
            InvalidateRect(hWnd, NULL, TRUE);
        }

        if (wParam == g_hpRegenTimer) {
            if (g_player.GetHp() < g_player.GetMaxHp()) {
                int recover = max(1, g_player.GetHp() / PlayerConstants::HP_REGEN_RATIO);
                g_player.Heal(recover);

                std::wstring msg = L"HP가 " + std::to_wstring(recover) + L" 회복되었습니다.";
                AddMessage(msg);
                InvalidateRect(hWnd, NULL, TRUE);
            }
        }
        
        if (wParam == g_monsterAiTimer) {
            for (auto& m : g_monsters) {
                if (!m.IsAlive()) continue;

                if (m.IsInRange(g_player.GetX(), g_player.GetY(), 1)) {
                    m.SetState(MonsterStateConstants::MONSTER_ATTACK);

                    m.UpdateAttackCooldown(MonsterConstants::MONSTER_MOVE_INTERVAL);
                    if (m.CanAttack()) {
                        m.ResetAttackCooldown();
                        g_player.TakeDamage(15);
                        AddMessage(L"몬스터의 공격으로 15의 데미지를 입었습니다.");

                        if (g_player.IsDead())
                            HandlePlayerDeath();
                    }
                }
                else if (m.IsInRange(g_player.GetX(), g_player.GetY(), MonsterConstants::MONSTER_SIGHT_RANGE)) {
                    m.SetState(MonsterStateConstants::MONSTER_CHASE);
                    m.ResetAttackCooldown();

                    auto path = m.FindPathToTarget(g_player.GetX(), g_player.GetY(), g_map);
                    if (!path.empty()) {
                        int nx = path[0].first;
                        int ny = path[0].second;

                        // 방향 갱신
                        if (nx > m.GetX()) m.SetDirection(DirectionConstants::DIR_RIGHT);
                        else if (nx < m.GetX()) m.SetDirection(DirectionConstants::DIR_LEFT);
                        else if (ny > m.GetY()) m.SetDirection(DirectionConstants::DIR_DOWN);
                        else if (ny < m.GetY()) m.SetDirection(DirectionConstants::DIR_UP);

                        m.Move(nx, ny);
                    }
                }
                else {
                    m.SetState(MonsterStateConstants::MONSTER_IDLE);
                    m.ResetAttackCooldown();
                }
            }

            InvalidateRect(hWnd, NULL, TRUE);
        }

        if (wParam == g_monsterSpawnTimer && !g_spawnedMonsters) {
            //SpawnMonster();
            //KillTimer(hWnd, g_monsterSpawnTimer); // 다시 호출되지 않도록 제거
            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;

    case WM_CHAR:
        if (g_isChatting) {
            if (wParam == VK_RETURN) {
                if (!g_chatBuffer.empty()) {
                    AddMessage(L"플레이어: " + g_chatBuffer);
                }
                g_chatBuffer.clear();
                g_isChatting = false;
            }
            else if (wParam == VK_BACK) {
                if (!g_chatBuffer.empty())
                    g_chatBuffer.pop_back();
            }
            else if (wParam >= 32) {
                g_chatBuffer.push_back((wchar_t)wParam);
            }
            InvalidateRect(hWnd, NULL, TRUE);
        }
        break;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);

        // 더블 버퍼링
        HDC memDC = CreateCompatibleDC(hdc);
        HBITMAP memBitmap = CreateCompatibleBitmap(hdc,
            VIEW_TILE_WIDTH * TILE_SIZE,
            VIEW_TILE_HEIGHT * TILE_SIZE);
        HBITMAP oldBitmap = (HBITMAP)SelectObject(memDC, memBitmap);
        Graphics graphics(memDC);

        int startX = g_player.GetX() - VIEW_TILE_WIDTH / 2;
        int startY = g_player.GetY() - VIEW_TILE_HEIGHT / 2;

        // 플레이어
        for (int y = 0; y < VIEW_TILE_HEIGHT; ++y) {
            for (int x = 0; x < VIEW_TILE_WIDTH; ++x) {
                int mapX = startX + x;
                int mapY = startY + y;
                int drawX = x * TILE_SIZE;
                int drawY = y * TILE_SIZE;

                SolidBrush bgBrush(Color(230, 230, 230));
                graphics.FillRectangle(&bgBrush, drawX, drawY, TILE_SIZE, TILE_SIZE);

                if (mapX >= 0 && mapX < MAP_WIDTH && mapY >= 0 && mapY < MAP_HEIGHT) {
                    if (g_map[mapY][mapX] == 1) {
                        SolidBrush wallBrush(Color(100, 100, 100));
                        graphics.FillRectangle(&wallBrush, drawX, drawY, TILE_SIZE, TILE_SIZE);
                    }
                }

                if (mapX == g_player.GetX() && mapY == g_player.GetY()) {
                    int fw = 16, fh = 16;
                    Rect destRect(drawX, drawY, TILE_SIZE, TILE_SIZE);

                    if (g_isAttacking && g_attackImage) {
                        int srcX = fw * g_player.GetDirection();
                        int srcY = 0;

                        graphics.DrawImage(g_attackImage,
                            destRect,
                            srcX, srcY, fw, fh,
                            UnitPixel);
                    }
                    else if (g_isMoving && g_walkImage) {
                        int dirCol = 0;
                        switch (g_player.GetDirection()) {
                        case DirectionConstants::DIR_DOWN:  dirCol = 0; break;
                        case DirectionConstants::DIR_UP:    dirCol = 1; break;
                        case DirectionConstants::DIR_LEFT:  dirCol = 2; break;
                        case DirectionConstants::DIR_RIGHT: dirCol = 3; break;
                        }
                        int srcX = fw * dirCol;
                        int srcY = fh * g_walkFrame;

                        graphics.DrawImage(g_walkImage, destRect,
                            srcX, srcY, fw, fh, UnitPixel);
                    }
                    else if (g_idleImage) {
                        int srcX = fw * g_player.GetDirection();
                        graphics.DrawImage(g_idleImage, destRect,
                            srcX, 0, fw, fh, UnitPixel);
                    }
                }
            }
        }

        // 몬스터
        for (auto& m : g_monsters) {
            if (!m.IsAlive()) continue;

            int relX = m.GetX() - startX;
            int relY = m.GetY() - startY;

            if (relX >= 0 && relX < VIEW_TILE_WIDTH &&
                relY >= 0 && relY < VIEW_TILE_HEIGHT) {
                int drawX = relX * TILE_SIZE;
                int drawY = relY * TILE_SIZE;

                Image* sprite = (m.GetState() == MonsterStateConstants::MONSTER_CHASE || 
                               m.GetState() == MonsterStateConstants::MONSTER_ATTACK)
                    ? g_monsterWalkImage : g_monsterIdleImage;
                if (!sprite) continue;

                // 16x16 기준
                int frameW = 16, frameH = 16;
                int srcX = frameW * m.GetAnimFrame();
                int srcY = frameH * m.GetDirection();

                graphics.DrawImage(sprite, Rect(drawX, drawY, TILE_SIZE, TILE_SIZE),
                    srcX, srcY, frameW, frameH, UnitPixel);
            }
        }

        // 검
        if (g_swordImage && g_swordX >= 0 && g_swordY >= 0) {
            int relX = g_swordX - startX;
            int relY = g_swordY - startY;

            if (relX >= 0 && relX < VIEW_TILE_WIDTH && relY >= 0 && relY < VIEW_TILE_HEIGHT) {
                float centerX = relX * TILE_SIZE + TILE_SIZE / 2.0f;
                float centerY = relY * TILE_SIZE + TILE_SIZE / 2.0f;

                float drawWidth = 6.0f;
                float drawHeight = 15.0f;

                float offsetX = drawWidth / 2.0f;
                float offsetY = drawHeight / 2.0f;

                graphics.TranslateTransform(centerX, centerY);

                switch (g_player.GetDirection()) {
                case DirectionConstants::DIR_UP:    graphics.RotateTransform(0); break;
                case DirectionConstants::DIR_DOWN:  graphics.RotateTransform(180); break;
                case DirectionConstants::DIR_LEFT:  graphics.RotateTransform(-90); break;
                case DirectionConstants::DIR_RIGHT: graphics.RotateTransform(90); break;
                }

                graphics.DrawImage(g_swordImage,
                    RectF(-offsetX, -offsetY, drawWidth, drawHeight),
                    0, 0,
                    (REAL)g_swordImage->GetWidth(),
                    (REAL)g_swordImage->GetHeight(),
                    UnitPixel);

                graphics.ResetTransform(); // 반드시 원상 복귀
            }
        }

        // 메시지 박스
        auto now = std::chrono::steady_clock::now();
        auto secondsSinceLastMessage = std::chrono::duration_cast<std::chrono::seconds>(now - g_lastMessageTime).count();

        if (!g_messageLog.empty() && secondsSinceLastMessage < 5) {
            int lineCount = (int)g_messageLog.size();
            int boxHeight = lineCount * MSG_LINE_HEIGHT + MSG_PADDING * 2;
            int boxWidth = MSG_MAX_WIDTH;

            int boxX = 5;
            int boxY = VIEW_TILE_HEIGHT * TILE_SIZE - boxHeight - 5;

            // 배경 박스 (반투명 검정)
            SolidBrush bg(Color(180, 0, 0, 0));
            graphics.FillRectangle(&bg, boxX, boxY, boxWidth, boxHeight);

            // 글자
            FontFamily fontFamily(L"맑은 고딕");
            Font font(&fontFamily, 12, FontStyleRegular, UnitPixel);
            SolidBrush text(Color(255, 255, 255, 255));

            for (int i = 0; i < lineCount; ++i) {
                PointF pt((REAL)(boxX + MSG_PADDING), (REAL)(boxY + MSG_PADDING + i * MSG_LINE_HEIGHT));
                graphics.DrawString(g_messageLog[i].c_str(), -1, &font, pt, &text);
            }
        }

        // === 플레이어 HP & 레벨 UI ===
        {
            int barX = 10;
            int barY = 10;
            int barWidth = 200;
            int barHeight = 16;

            float hpRatio = g_player.GetHp() / 100.0f;
            int hpBarFill = static_cast<int>(barWidth * hpRatio);

            // 배경 박스
            SolidBrush bgBrush(Color(100, 100, 100));
            graphics.FillRectangle(&bgBrush, barX, barY, barWidth, barHeight);

            // 체력 바 (녹색)
            SolidBrush hpBrush(Color(0, 200, 0));
            graphics.FillRectangle(&hpBrush, barX, barY, hpBarFill, barHeight);

            // 테두리
            Pen borderPen(Color(255, 255, 255));
            graphics.DrawRectangle(&borderPen, barX, barY, barWidth, barHeight);

            // 레벨 텍스트
            FontFamily fontFamily(L"맑은 고딕");
            Font font(&fontFamily, 12, FontStyleBold, UnitPixel);
            SolidBrush textBrush(Color(0, 0, 0));
            std::wstring levelText = L"Lv. " + std::to_wstring(g_player.GetLevel());

            // 경험치 바 (레벨업까지 얼마나 남았는지)
            int expY = barY + barHeight + 6;
            int expMax = Player::GetExpToLevelUp(g_player.GetLevel());
            float expRatio = (float)g_player.GetExp() / expMax;
            int expFill = static_cast<int>(barWidth * expRatio);

            // 배경
            SolidBrush expBg(Color(60, 60, 60));
            graphics.FillRectangle(&expBg, barX, expY, barWidth, 10);

            // 채움
            SolidBrush expFillBrush(Color(0, 120, 255));
            graphics.FillRectangle(&expFillBrush, barX, expY, expFill, 10);

            // 레벨 텍스트 위치 조정
            int levelTextX = barX;
            int levelTextY = expY + 10 + 4;

            graphics.DrawString(
                levelText.c_str(),
                -1,
                &font,
                PointF((REAL)levelTextX, (REAL)levelTextY),
                &textBrush);
        }

        // 채팅창
        {
            if (g_isChatting) {
                std::wstring prompt = L"> " + g_chatBuffer;

                int chatBoxY = VIEW_TILE_HEIGHT * TILE_SIZE - 28;

                SolidBrush inputBg(Color(200, 30, 30, 30));
                graphics.FillRectangle(&inputBg, 5, chatBoxY, 300, 24);

                FontFamily fontFamily(L"맑은 고딕");
                Font font(&fontFamily, 12, FontStyleRegular, UnitPixel);
                SolidBrush inputText(Color(255, 255, 255));

                graphics.DrawString(prompt.c_str(), -1, &font, PointF(10, (REAL)(chatBoxY + 4)), &inputText);
            }
        }

        // 플레이어 좌표 UI
        {
            // ==== 플레이어 좌표 표시 ====
            std::wstring posText = L"Pos: (" + std::to_wstring(g_player.GetX()) + L", " + std::to_wstring(g_player.GetY()) + L")";

            // 폰트 설정
            FontFamily fontFamily(L"맑은 고딕");
            Font font(&fontFamily, 16, FontStyleRegular, UnitPixel);
            SolidBrush textBrush(Color(0, 0, 0));

            // 위치 계산 (오른쪽 상단)
            int posTextWidth = 160;
            int posTextHeight = 20;
            int posX = VIEW_TILE_WIDTH * TILE_SIZE - posTextWidth - 10;
            int posY = 10;

            graphics.DrawString(posText.c_str(), -1, &font, PointF((REAL)posX, (REAL)posY), &textBrush);

        }

        BitBlt(hdc, 0, 0,
            VIEW_TILE_WIDTH * TILE_SIZE,
            VIEW_TILE_HEIGHT * TILE_SIZE,
            memDC, 0, 0, SRCCOPY);

        SelectObject(memDC, oldBitmap);
        DeleteObject(memBitmap);
        DeleteDC(memDC);
        EndPaint(hWnd, &ps);
    }
    break;

    case WM_DESTROY:
        delete g_idleImage;
        delete g_walkImage;
        delete g_attackImage;
        delete g_swordImage;
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}
