#include "framework.h"//одна из важнейших частей для компиляции!!
#include <windows.h>
#include <gl/gl.h>
#include <vector>
#include <cmath>
#include <string>

#pragma comment(lib, "opengl32.lib")

const float GRAVITY_MAG = -0.0007f;
const float FRICTION = 0.99f;
const float BOUNCE = 0.2f;
const int SUB_STEPS = 8;
const float PLAYER_SPEED = 0.00005f;
const float JUMP_FORCE = 0.0007f;
const float ROT_SPEED = 0.009f;

int windowWidth = 1600, windowHeight = 900;

const int MAP_HEIGHT = 12;
const int MAP_WIDTH = 20;
std::string levelMap[MAP_HEIGHT] = {
    "####################",
    "#                  #",
    "#                  #",
    "2     P            1",
    "#########          #",
    "#       #######    #",
    "#                  #",
    "#                  #",
    "################## #",
    "#                  #",
    "#                  #",
    "####################"
};

// Вычисляем размер тайла так, чтобы вся ширина MAP_WIDTH входила в 2 * aspect
float GetTileSize() {
    float aspect = (float)windowWidth / (float)windowHeight;
    return (aspect * 2.0f) / (float)MAP_WIDTH;
}
struct RGB { float r, g, b; };
struct Vector2 { float x, y; };
struct Object {
    Vector2 pos;
    float vx, vy, radius, angle;
    RGB col;
};

class Player {
    int jumpCnt = 1, MjumpCnt = 1;
public:
    void Movement(Object& pl) {
        if (GetAsyncKeyState('Q') & 0x8000) pl.angle += ROT_SPEED;
        if (GetAsyncKeyState('E') & 0x8000) pl.angle -= ROT_SPEED;
        if (GetAsyncKeyState('T') & 0x8000) pl.angle = 0;

        float downX = sinf(pl.angle), downY = -cosf(pl.angle);
        float rightX = cosf(pl.angle), rightY = sinf(pl.angle);

        pl.vx += downX * (-GRAVITY_MAG / SUB_STEPS);
        pl.vy += downY * (-GRAVITY_MAG / SUB_STEPS);

        if (GetAsyncKeyState('A') & 0x8000) { pl.vx -= rightX * PLAYER_SPEED; pl.vy -= rightY * PLAYER_SPEED; }
        if (GetAsyncKeyState('D') & 0x8000) { pl.vx += rightX * PLAYER_SPEED; pl.vy += rightY * PLAYER_SPEED; }

        if ((GetAsyncKeyState(VK_SPACE) & 0x8000) && (jumpCnt != 0)) {
            for (int i = 0; i < 30; i++) {
                pl.vx -= downX * (JUMP_FORCE * 5 / SUB_STEPS);
                pl.vy -= downY * (JUMP_FORCE * 5 / SUB_STEPS);
            }
            jumpCnt--;
        }
    }

    void Collisions(Object& pl) {
        float aspect = (float)windowWidth / windowHeight;
        float tileSize = GetTileSize();

        for (int y = 0; y < MAP_HEIGHT; y++) {
            for (int x = 0; x < MAP_WIDTH; x++) {
                if (levelMap[y][x] == '#') {
                    // Центрирование карты: -aspect + смещение. 
                    // Смещение по Y начинается сверху (1.0) вниз.
                    float tx = -aspect + x * tileSize + tileSize / 2.0f;
                    float ty = 1.0f - y * tileSize - tileSize / 2.0f;
                    float half = tileSize / 2.0f;

                    float dx = pl.pos.x - tx;
                    float dy = pl.pos.y - ty;
                    float px = (pl.radius + half) - fabsf(dx);
                    float py = (pl.radius + half) - fabsf(dy);

                    if (px > 0 && py > 0) {
                        if (px < py) {
                            pl.pos.x += (dx > 0) ? px : -px;
                            pl.vx *= -BOUNCE;
                        }
                        else {
                            pl.pos.y += (dy > 0) ? py : -py;
                            pl.vy *= -BOUNCE;
                            if (pl.angle == 0)jumpCnt = MjumpCnt;
                        }
                    }
                }                
            }
        }
    }
};

Object playerObj, obj;
Player playerLogic;

void DrawSquare(float x, float y, float size, float angle, RGB rgb) {
    glPushMatrix();
    glTranslatef(x, y, 0);
    glRotatef(angle * 57.2958f, 0, 0, 1);
    glColor3f(rgb.r, rgb.g, rgb.b);
    glBegin(GL_QUADS);
    glVertex2f(-size, -size); glVertex2f(size, -size);
    glVertex2f(size, size); glVertex2f(-size, size);
    glEnd();
    glPopMatrix();
}

void DrawMap() {
    float aspect = (float)windowWidth / windowHeight;
    float tileSize = GetTileSize();
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (levelMap[y][x] == '#') {
                float tx = -aspect + x * tileSize + tileSize / 2.0f;
                float ty = 1.0f - y * tileSize - tileSize / 2.0f;
                DrawSquare(tx, ty, tileSize / 2.0f, 0, { 0.2f, 0.8f, 0.6f });
            }
        }
    }
}

void Update() {
    for (int step = 0; step < SUB_STEPS; step++) {
        playerLogic.Movement(playerObj);
        playerLogic.Movement(obj);
        playerLogic.Collisions(obj);
        playerObj.vx *= FRICTION; playerObj.vy *= FRICTION;
        playerObj.pos.x += playerObj.vx; playerObj.pos.y += playerObj.vy;

        playerLogic.Collisions(playerObj);
    }
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_DESTROY) PostQuitMessage(0);
    if (msg == WM_SIZE) { windowWidth = LOWORD(lp); windowHeight = HIWORD(lp); }
    return DefWindowProc(hWnd, msg, wp, lp);
}

int APIENTRY wWinMain(HINSTANCE hI, HINSTANCE hP, LPWSTR lp, int nS) {
    WNDCLASSEXW wcex = { sizeof(WNDCLASSEX), CS_HREDRAW | CS_VREDRAW, WndProc, 0, 0, hI, 0, LoadCursor(0, IDC_ARROW), 0, 0, L"LevelEd", 0 };
    RegisterClassExW(&wcex);
    HWND hWnd = CreateWindowW(L"LevelEd", L"Map Fitting Screen", WS_OVERLAPPEDWINDOW, 100, 100, 1290, 800, 0, 0, hI, 0);

    HDC hDC = GetDC(hWnd);
    PIXELFORMATDESCRIPTOR pfd = { sizeof(pfd), 1, PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, PFD_TYPE_RGBA, 32 };
    SetPixelFormat(hDC, ChoosePixelFormat(hDC, &pfd), &pfd);
    wglMakeCurrent(hDC, wglCreateContext(hDC));

    // Инициализация игрока (позиционирование привязано к размеру тайла)
    windowWidth = 1280; windowHeight = 790; // Временные для инициализации
    float aspect = (float)windowWidth / windowHeight;
    float ts = GetTileSize();
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (levelMap[y][x] == 'P') {
                playerObj = { {-aspect + x * ts + ts / 2.0f, 1.0f - y * ts - ts / 2.0f}, 0, 0, ts * 0.4f, 0, 1.0f, 0.2f, 0.2f };
            }
        }
    }

    ShowWindow(hWnd, nS);
    MSG msg = { 0 };
    while (msg.message != WM_QUIT) {
        if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) { TranslateMessage(&msg); DispatchMessage(&msg); }
        else {
            Update();
            glViewport(0, 0, windowWidth, windowHeight);
            glClearColor(0.1f, 0.1f, 0.12f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glLoadIdentity();
            float aspect = (float)windowWidth / windowHeight;
            glOrtho(-aspect, aspect, -1, 1, -1, 1);

            DrawMap();
            DrawSquare(playerObj.pos.x, playerObj.pos.y, playerObj.radius, playerObj.angle, { playerObj.col.r, playerObj.col.g, playerObj.col.b });

            SwapBuffers(hDC);
            Sleep(1);
        }
    }
    return 0;
}