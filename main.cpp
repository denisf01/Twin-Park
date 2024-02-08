#if defined(UNICODE) && !defined(_UNICODE)
#define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
#define UNICODE
#endif

#include <tchar.h>
#include <windows.h>
#include <iostream>
#include <thread>
#include <algorithm>
#include <random>
#include <vector>
#include "resources.h"

#define WIDTH 1000
#define HEIGHT 700
#define isPressed(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)

using namespace std;

string teamName = "";

void isGameOver(HWND);
void draw(HWND);
void calculateSheepPosition(HWND);
void calculateBirdPosition(HWND);
void calculateWolfPosition(HWND);
void loadBitmaps();
void deleteBitmaps();
void setDefaults();
void setLevel(int);
INT_PTR CALLBACK DlgProcTeamName(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DlgProcLeaderboard(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam);

class Object
{
public:
    Object() = default;
    Object(int w, int h, int x, int y) : width{w}, height{h}, x{x}, y{y} {}
    int width;
    int height;
    int x;
    int y;
    bool isPlayer = false;
};

vector<Object *> filterObjects(const vector<Object *> &other, Object *obj)
{

    // Filter condition
    auto condition = [&](Object *o)
    { return obj->y == o->y + o->height && obj->x + obj->width > o->x && obj->x < o->x + o->width && o != obj; };

    // New vector to store filtered elements
    std::vector<Object *> filteredVector;

    // Filter the original vector
    std::copy_if(other.begin(), other.end(), std::back_inserter(filteredVector), condition);
    return filteredVector;
}

bool shouldFall(Object *obj);
bool isBlocked(Object *obj, bool);
void checkStart(HWND);
void checkLeaderboard(HWND);

class Player : public Object
{
public:
    Player() : Object(), isJumping{false}, isRight{true}, isFalling{false}
    {
        isPlayer = true;
    }
    bool isJumping;
    int i;
    bool isRight;
    bool isFalling;
    int initY;
    bool completed;
    void jumping();
};

Player player1;
Player player2;
Object platform(WIDTH, 0, 0, 0);
Object leftPlt(WIDTH / 3 - 30, 100, 0, -100);
Object rightPlt(WIDTH, 100, WIDTH / 2 + 165, -100);
Object rightWall(70, HEIGHT, WIDTH - 70, HEIGHT);
Object leftWall(70, HEIGHT, 0, HEIGHT);
Object upperPlatform;
Object leftPlt2(WIDTH / 3 - 30, 100, 0, -100);
Object rightPlt2(WIDTH, 100, WIDTH / 2 + 250, -100);
Object upperPlatform2;
Object boxObj;
Object boxObj1;
Object boxObj2;
Object boxObj3;
Object boxObj4;

static int level = 2;

vector<Object *> objects = vector<Object *>();

bool gameOver = false;
void checkPortal(Player *p);
void checkButtons(Player *p1, Player *p2);
void checkSuccess(Player *p1, Player *p2);
void checkGameover(Player *p1, Player *p2);
void introScreen();
void level1();

int introInitHeight = 530;

HBITMAP bk, bk2, bkGameOver, player1WR, player1WL, player1BR, player1BL, titleWhite, titleBlack, startWhite, startBlack, box;
HBITMAP player2WR, player2WL, player2BR, player2BL, powerB, powerW;
HBITMAP wall, platform2, plt, plt2, portalW, portalB, doorW, doorB, buttonUpW, buttonUpB, buttonDownW, buttonDownB;
HBITMAP buttonW1, buttonB1, buttonW2, buttonB2, leaderboardW, leaderboardB;

/*  Declare Windows procedure  */
LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);

/*  Make the class name into a global variable  */
TCHAR szClassName[] = _T("CodeBlocksWindowsApp");

int WINAPI WinMain(HINSTANCE hThisInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpszArgument,
                   int nCmdShow)
{
    HWND hwnd;        /* This is the handle for our window */
    MSG messages;     /* Here messages to the application are saved */
    WNDCLASSEX wincl; /* Data structure for the windowclass */

    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure; /* This function is called by windows */
    wincl.style = CS_DBLCLKS;            /* Catch double-clicks */
    wincl.cbSize = sizeof(WNDCLASSEX);

    /* Use default icon and mouse-pointer */
    wincl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL; /* No menu */
    wincl.cbClsExtra = 0;      /* No extra bytes after the window class */
    wincl.cbWndExtra = 0;      /* structure or the window instance */
    /* Use Windows's default colour as the background of the window */
    wincl.hbrBackground = (HBRUSH)COLOR_BACKGROUND;

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClassEx(&wincl))
        return 0;

    /* The class is registered, let's create the program*/
    hwnd = CreateWindowEx(
        0,                                       /* Extended possibilites for variation */
        szClassName,                             /* Classname */
        _T("Code::Blocks Template Windows App"), /* Title Text */
        WS_OVERLAPPEDWINDOW,                     /* default window */
        CW_USEDEFAULT,                           /* Windows decides the position */
        CW_USEDEFAULT,                           /* where the window ends up on the screen */
        WIDTH,                                   /* The programs width */
        HEIGHT,                                  /* and height in pixels */
        HWND_DESKTOP,                            /* The window is a child-window to desktop */
        NULL,                                    /* No menu */
        hThisInstance,                           /* Program Instance handler */
        NULL                                     /* No Window Creation data */
    );

    /* Make the window visible on the screen */
    ShowWindow(hwnd, nCmdShow);

    /* Run the message loop. It will run until GetMessage() returns 0 */
    while (1)
    {
        if (PeekMessage(&messages, NULL, 0, 0, PM_REMOVE))
        {
            if (messages.message == WM_QUIT)
                break;
            TranslateMessage(&messages);
            DispatchMessage(&messages);
        }
        if (gameOver)
        {
            Sleep(100);
            continue;
        }
        calculateSheepPosition(hwnd);
        if (player1.isJumping)

            player1.jumping();

        if (player2.isJumping)
            player2.jumping();
        draw(hwnd);
        // isGameOver(hwnd);
        // Sleep(40);
    }

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}

void draw(HWND hwnd)
{
    HDC hdc = GetDC(hwnd);
    HDC hdcMem, hdcTmp;
    HBITMAP hbmMem, background, player1White, player1Black, tmp1, tmp2, player2White, player2Black;
    BITMAP bm;

    if (gameOver)
    {
        background = bkGameOver;
        hdcMem = CreateCompatibleDC(hdc);
        tmp1 = (HBITMAP)SelectObject(hdcMem, background);
        GetObject(background, sizeof(BITMAP), &bm);
        StretchBlt(hdc, 0, 0, WIDTH, HEIGHT, hdcMem, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

        SelectObject(hdcMem, tmp1);
        DeleteObject(hdcMem);

        ReleaseDC(hwnd, hdc);
        return;
    }

    background = bk;
    player1White = player1.isRight ? player1WR : player1WL;
    player1Black = player1.isRight ? player1BR : player1BL;

    player2White = player2.isRight ? player2WR : player2WL;
    player2Black = player2.isRight ? player2BR : player2BL;

    hdcMem = CreateCompatibleDC(hdc);
    hbmMem = CreateCompatibleBitmap(hdc, WIDTH, HEIGHT);
    tmp1 = (HBITMAP)SelectObject(hdcMem, hbmMem);
    hdcTmp = CreateCompatibleDC(hdc);

    // background
    tmp2 = (HBITMAP)SelectObject(hdcTmp, background);
    GetObject(background, sizeof(BITMAP), &bm);
    StretchBlt(hdcMem, 0, 0, WIDTH, HEIGHT, hdcTmp, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

    if (level == 0)
    {

        // title
        SelectObject(hdcTmp, titleWhite);
        GetObject(titleWhite, sizeof(BITMAP), &bm);
        BitBlt(hdcMem, WIDTH / 2 - bm.bmWidth / 2, HEIGHT / 10, bm.bmWidth, bm.bmHeight, hdcTmp, 0, 0, SRCAND);
        SelectObject(hdcTmp, titleBlack);
        BitBlt(hdcMem, WIDTH / 2 - bm.bmWidth / 2, HEIGHT / 10, bm.bmWidth, bm.bmHeight, hdcTmp, 0, 0, SRCPAINT);

        // start button

        SelectObject(hdcTmp, startWhite);
        GetObject(startWhite, sizeof(BITMAP), &bm);
        BitBlt(hdcMem, WIDTH / 3 - bm.bmWidth / 2, HEIGHT / 3, bm.bmWidth, bm.bmHeight, hdcTmp, 0, 0, SRCAND);
        SelectObject(hdcTmp, startBlack);
        BitBlt(hdcMem, WIDTH / 3 - bm.bmWidth / 2, HEIGHT / 3, bm.bmWidth, bm.bmHeight, hdcTmp, 0, 0, SRCPAINT);

        // leaderboard button

        SelectObject(hdcTmp, leaderboardW);
        GetObject(leaderboardW, sizeof(BITMAP), &bm);
        BitBlt(hdcMem, 2 * WIDTH / 3 - bm.bmWidth / 2, HEIGHT / 3, bm.bmWidth, bm.bmHeight, hdcTmp, 0, 0, SRCAND);
        SelectObject(hdcTmp, leaderboardB);
        BitBlt(hdcMem, 2 * WIDTH / 3 - bm.bmWidth / 2, HEIGHT / 3, bm.bmWidth, bm.bmHeight, hdcTmp, 0, 0, SRCPAINT);
    }
    else if (level == 1)
    {

        // //   platform
        // SelectObject(hdcTmp, plt);
        // GetObject(plt, sizeof(BITMAP), &bm);
        // BitBlt(hdcMem, -30, HEIGHT - 170, bm.bmWidth, bm.bmHeight, hdcTmp, 0, 0, SRCCOPY);

        // portal

        SelectObject(hdcTmp, portalW);
        GetObject(portalW, sizeof(BITMAP), &bm);
        BitBlt(hdcMem, 75, introInitHeight - bm.bmHeight + 5, bm.bmWidth, bm.bmHeight, hdcTmp, 0, 0, SRCAND);
        BitBlt(hdcMem, WIDTH - 180, 200 - bm.bmHeight - 30, bm.bmWidth, bm.bmHeight, hdcTmp, 0, 0, SRCAND);

        SelectObject(hdcTmp, portalB);
        BitBlt(hdcMem, 75, introInitHeight - bm.bmHeight + 5, bm.bmWidth, bm.bmHeight, hdcTmp, 0, 0, SRCPAINT);
        BitBlt(hdcMem, WIDTH - 180, 200 - bm.bmHeight - 30, bm.bmWidth, bm.bmHeight, hdcTmp, 0, 0, SRCPAINT);

        // buttons
        SelectObject(hdcTmp, buttonW1);
        GetObject(buttonW1, sizeof(BITMAP), &bm);
        BitBlt(hdcMem, WIDTH - 300, introInitHeight - bm.bmHeight + 5, bm.bmWidth, bm.bmHeight, hdcTmp, 0, 0, SRCAND);
        SelectObject(hdcTmp, buttonW2);
        GetObject(buttonW2, sizeof(BITMAP), &bm);
        BitBlt(hdcMem, WIDTH / 2, 200 - bm.bmHeight - 30, bm.bmWidth, bm.bmHeight, hdcTmp, 0, 0, SRCAND);

        SelectObject(hdcTmp, buttonB1);
        GetObject(buttonB1, sizeof(BITMAP), &bm);
        BitBlt(hdcMem, WIDTH - 300, introInitHeight - bm.bmHeight + 5, bm.bmWidth, bm.bmHeight, hdcTmp, 0, 0, SRCPAINT);
        SelectObject(hdcTmp, buttonB2);
        GetObject(buttonB2, sizeof(BITMAP), &bm);
        BitBlt(hdcMem, WIDTH / 2, 200 - bm.bmHeight - 30, bm.bmWidth, bm.bmHeight, hdcTmp, 0, 0, SRCPAINT);

        // platform with hole

        SelectObject(hdcTmp, plt);
        GetObject(plt, sizeof(BITMAP), &bm);
        BitBlt(hdcMem, leftPlt.x, HEIGHT - 170, leftPlt.width, bm.bmHeight, hdcTmp, 0, 0, SRCCOPY);
        BitBlt(hdcMem, rightPlt.x, HEIGHT - 170, bm.bmWidth / 2, bm.bmHeight, hdcTmp, 100, 0, SRCCOPY);

        // door
        SelectObject(hdcTmp, doorW);
        GetObject(doorW, sizeof(BITMAP), &bm);
        BitBlt(hdcMem, WIDTH - 180, introInitHeight - bm.bmHeight + 5, bm.bmWidth, bm.bmHeight, hdcTmp, 0, 0, SRCAND);

        SelectObject(hdcTmp, doorB);
        BitBlt(hdcMem, WIDTH - 180, introInitHeight - bm.bmHeight + 5, bm.bmWidth, bm.bmHeight, hdcTmp, 0, 0, SRCPAINT);

        // box

        // SelectObject(hdcTmp, box);
        // GetObject(box, sizeof(BITMAP), &bm);
        // boxObj.width = bm.bmWidth;
        // boxObj.height = bm.bmHeight;
        // BitBlt(hdcMem, boxObj.x, introInitHeight - boxObj.height - boxObj.y, boxObj.width, boxObj.height, hdcTmp, 0, 0, SRCCOPY);

        // wall

        SelectObject(hdcTmp, wall);
        GetObject(wall, sizeof(BITMAP), &bm);
        BitBlt(hdcMem, -5, 0, bm.bmWidth, bm.bmHeight, hdcTmp, 0, 0, SRCCOPY);
        BitBlt(hdcMem, WIDTH - bm.bmWidth - 5, 0, bm.bmWidth, bm.bmHeight, hdcTmp, 0, 0, SRCCOPY);

        // upper platform

        SelectObject(hdcTmp, platform2);
        GetObject(platform2, sizeof(BITMAP), &bm);
        upperPlatform.width = bm.bmWidth;
        upperPlatform.height = bm.bmHeight;
        upperPlatform.x = WIDTH - upperPlatform.width - 70;
        BitBlt(hdcMem, WIDTH - upperPlatform.width - 70, introInitHeight - upperPlatform.height - upperPlatform.y, bm.bmWidth, bm.bmHeight, hdcTmp, 0, 0, SRCCOPY);
    }else{

        // platform with hole
        SelectObject(hdcTmp, plt);
        GetObject(plt, sizeof(BITMAP), &bm);
        BitBlt(hdcMem, leftPlt2.x, HEIGHT - 170, leftPlt2.width, bm.bmHeight, hdcTmp, 0, 0, SRCCOPY);
        BitBlt(hdcMem, rightPlt2.x, HEIGHT - 170, bm.bmWidth / 2, bm.bmHeight, hdcTmp, 100, 0, SRCCOPY);
        
         // door
        SelectObject(hdcTmp, doorW);
        GetObject(doorW, sizeof(BITMAP), &bm);
        BitBlt(hdcMem, WIDTH - 180, introInitHeight - bm.bmHeight + 5, bm.bmWidth, bm.bmHeight, hdcTmp, 0, 0, SRCAND);

        SelectObject(hdcTmp, doorB);
        BitBlt(hdcMem, WIDTH - 180, introInitHeight - bm.bmHeight + 5, bm.bmWidth, bm.bmHeight, hdcTmp, 0, 0, SRCPAINT);

        // wall
        SelectObject(hdcTmp, wall);
        GetObject(wall, sizeof(BITMAP), &bm);
        BitBlt(hdcMem, -5, 0, bm.bmWidth, bm.bmHeight, hdcTmp, 0, 0, SRCCOPY);
        BitBlt(hdcMem, WIDTH - bm.bmWidth - 5, 0, bm.bmWidth, bm.bmHeight, hdcTmp, 0, 0, SRCCOPY);
        
        // upper platform

        SelectObject(hdcTmp, platform2);
        GetObject(platform2, sizeof(BITMAP), &bm);
        upperPlatform2.width = bm.bmWidth - 200;
        upperPlatform2.height = bm.bmHeight;
        upperPlatform2.x = WIDTH - upperPlatform2.width - 70;
        BitBlt(hdcMem, WIDTH - upperPlatform2.width - 70, introInitHeight - upperPlatform2.height - upperPlatform2.y, bm.bmWidth - 200, bm.bmHeight, hdcTmp, 0, 0, SRCCOPY);

         // box

        SelectObject(hdcTmp, box);
        GetObject(box, sizeof(BITMAP), &bm);
        boxObj.width = bm.bmWidth;
        boxObj.height = bm.bmHeight;
        BitBlt(hdcMem, boxObj.x, introInitHeight - boxObj.height - boxObj.y, boxObj.width, boxObj.height, hdcTmp, 0, 0, SRCCOPY);
        BitBlt(hdcMem, boxObj1.x, introInitHeight - boxObj.height - boxObj1.y, boxObj.width, boxObj.height, hdcTmp, 0, 0, SRCCOPY);
        BitBlt(hdcMem, boxObj2.x, introInitHeight - boxObj.height - boxObj2.y, boxObj.width, boxObj.height, hdcTmp, 0, 0, SRCCOPY);
        BitBlt(hdcMem, boxObj3.x, introInitHeight - boxObj.height - boxObj3.y, boxObj.width, boxObj.height, hdcTmp, 0, 0, SRCCOPY);
        BitBlt(hdcMem, boxObj4.x, introInitHeight - boxObj.height - boxObj4.y, boxObj.width, boxObj.height, hdcTmp, 0, 0, SRCCOPY);
    
        // power

        SelectObject(hdcTmp, powerW);
        GetObject(powerW, sizeof(BITMAP), &bm);
        BitBlt(hdcMem, WIDTH - 180, introInitHeight - 450, bm.bmWidth, bm.bmHeight, hdcTmp, 0, 0, SRCAND);
        SelectObject(hdcTmp, powerB);
        BitBlt(hdcMem, WIDTH - 180, introInitHeight - 450, bm.bmWidth, bm.bmHeight, hdcTmp, 0, 0, SRCPAINT);
    }
    // player1
    SelectObject(hdcTmp, player1White);
    GetObject(player1White, sizeof(BITMAP), &bm);
    player1.width = bm.bmWidth / 17;
    player1.height = bm.bmHeight;
    BitBlt(hdcMem, player1.x, introInitHeight - player1.height - player1.y, player1.width, player1.height, hdcTmp, player1.i * player1.width, 0, SRCAND);
    SelectObject(hdcTmp, player1Black);
    BitBlt(hdcMem, player1.x, introInitHeight - player1.height - player1.y, player1.width, player1.height, hdcTmp, player1.i * player1.width, 0, SRCPAINT);

    // player2
    SelectObject(hdcTmp, player2White);
    GetObject(player2White, sizeof(BITMAP), &bm);
    player2.width = bm.bmWidth / 17;
    player2.height = bm.bmHeight;
    BitBlt(hdcMem, player2.x, introInitHeight - player2.height - player2.y, player2.width, player2.height, hdcTmp, player2.i * player2.width, 0, SRCAND);
    SelectObject(hdcTmp, player2Black);
    BitBlt(hdcMem, player2.x, introInitHeight - player2.height - player2.y, player2.width, player2.height, hdcTmp, player2.i * player2.width, 0, SRCPAINT);

    GetObject(hbmMem, sizeof(BITMAP), &bm);
    BitBlt(hdc, 0, 0, bm.bmWidth, bm.bmHeight, hdcMem, 0, 0, SRCCOPY);

    SelectObject(hdcMem, tmp1);
    SelectObject(hdcTmp, tmp2);

    DeleteObject(hdcMem);
    DeleteObject(hdcTmp);
    DeleteObject(hbmMem);

    ReleaseDC(hwnd, hdc);
}

/*  This function is called by the Windows function DispatchMessage()  */

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message) /* handle the messages */
    {
    case WM_CREATE:
        loadBitmaps();
        setDefaults();
        break;
    case WM_KEYUP:
        if (gameOver)
        {
            gameOver = false;
            objects.clear();
            setDefaults();
        }
        break;
    case WM_LBUTTONDOWN:
        // DialogBox(NULL, MAKEINTRESOURCE(IDD_TEAM_NAME), hwnd, DlgProcTeamName);

        break;
    case WM_DESTROY:
        deleteBitmaps();
        PostQuitMessage(0); /* send a WM_QUIT to the message queue */
        break;
    default: /* for messages that we don't deal with */
        return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}

void calculateSheepPosition(HWND hwnd)
{
    if (shouldFall(&player1) && !player1.isJumping)
    {
        player1.y--;
        player1.isFalling = true;
    }
    if (shouldFall(&player2) && !player2.isJumping)
    {
        player2.y--;
        player2.isFalling = true;
    }
    checkGameover(&player1, &player2);
    if (isPressed(VK_UP))
    {

        player1.isJumping = true;
    }
    if (isPressed(VK_RIGHT))
    {
        player1.isRight = true;
        if (player1.x + player1.width <= WIDTH && !isBlocked(&player1, player1.isRight))
            player1.x += 1;
        if (++player1.i > 16)
        {
            player1.i = 0;
        }
    }
    if (isPressed(VK_LEFT))
    {
        player1.isRight = false;
        if (player1.x >= 0 && !isBlocked(&player1, player1.isRight))
            player1.x -= 1;
        if (++player1.i > 16)
        {
            player1.i = 0;
        }
    }

    if (isPressed(0x57)) // VK_W
    {
        player2.isJumping = true;
    }
    if (isPressed(0x44)) // VK_D
    {
        player2.isRight = true;
        if (player2.x + player2.width <= WIDTH && !isBlocked(&player2, player2.isRight))
            player2.x += 1;
        if (++player2.i > 16)
        {
            player2.i = 0;
        }
    }
    if (isPressed(0x41)) // VK_A
    {
        player2.isRight = false;
        if (player2.x >= 0 && !isBlocked(&player2, player2.isRight))
            player2.x -= 1;
        if (++player2.i > 16)
        {
            player2.i = 0;
        }
    }
    if (level == 1)
    {
        checkPortal(&player1);
        checkPortal(&player2);
        checkButtons(&player1, &player2);
        checkSuccess(&player1, &player2);
    }
    if (level == 0)
    {
        checkStart(hwnd);
        checkLeaderboard(hwnd);
    }
}

void Player::jumping()
{
    auto tmp = filterObjects(objects, this);
    std::sort(tmp.begin(), tmp.end(), [](Object *a, Object *b)
              { return a->y > b->y; });
    if (!tmp.empty() && this->isFalling)
    {
        this->initY = this->y = tmp.front()->y + tmp.front()->height;
        this->isFalling = false;
        this->isJumping = false;
    }
    else
    {
        if (this->y == this->initY + 100)

            this->isFalling = true;

        this->y += this->isFalling ? -1 : 1;
    }
}

void checkSuccess(Player *p1, Player *p2)
{
    if (p1->x >= WIDTH - 175 && p2->x >= WIDTH - 175 && p1->y == 0 && p2->y == 0)
    {
        cout << "Level completed" << endl;
    }
}

void checkGameover(Player *p1, Player *p2)
{
    if (p1->y < -200 || p2->y < -200)
        gameOver = true;
}

void checkPortal(Player *p)
{
    if (p->x == 100 && p->y == 0)
    {
        p->x = WIDTH - 185;
        p->y = p->initY = upperPlatform.y + upperPlatform.height;
    }
    if (p->x == WIDTH - 180 && p->y == upperPlatform.y + upperPlatform.height)
    {
        p->x = 105;
        p->y = p->initY = 0;
    }
}

void checkButtons(Player *p1, Player *p2)
{
    if (p1->x > WIDTH - 300 - 10 && p1->x < WIDTH - 300 + 10 && p1->y == 0 || p2->x > WIDTH - 300 - 10 && p2->x < WIDTH - 300 + 10 && p2->y == 0)
    {
        buttonW1 = buttonDownW;
        buttonB1 = buttonDownB;
        leftPlt.width = WIDTH;
        return;
    }
    if (p1->x > WIDTH / 2 - 10 && p1->x < WIDTH / 2 + 10 && p1->y == upperPlatform.y + upperPlatform.height || p2->x > WIDTH / 2 - 10 && p2->x < WIDTH / 2 + 10 && p2->y == upperPlatform.y + upperPlatform.height)
    {
        buttonW2 = buttonDownW;
        buttonB2 = buttonDownB;
        leftPlt.width = WIDTH;
        return;
    }
    buttonW1 = buttonUpW;
    buttonW2 = buttonUpW;
    buttonB1 = buttonUpB;
    buttonB2 = buttonUpB;
    leftPlt.width = WIDTH / 3 - 30;
}

void loadBitmaps()
{
    bk = (HBITMAP)LoadImage(NULL, "assets/introBackground.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    plt = (HBITMAP)LoadImage(NULL, "assets/platform.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    plt2 = (HBITMAP)LoadImage(NULL, "assets/platformHole.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    bk2 = (HBITMAP)LoadImage(NULL, "assets/levelOneBackground.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    wall = (HBITMAP)LoadImage(NULL, "assets/wall.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    platform2 = (HBITMAP)LoadImage(NULL, "assets/platform2.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    box = (HBITMAP)LoadImage(NULL, "assets/box.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    bkGameOver = (HBITMAP)LoadImage(NULL, "assets/gameOver.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    titleWhite = (HBITMAP)LoadImage(NULL, "assets/titleWhite.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    titleBlack = (HBITMAP)LoadImage(NULL, "assets/titleBlack.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    portalW = (HBITMAP)LoadImage(NULL, "assets/portalW.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    portalB = (HBITMAP)LoadImage(NULL, "assets/portalB.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    doorW = (HBITMAP)LoadImage(NULL, "assets/doorW.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    doorB = (HBITMAP)LoadImage(NULL, "assets/doorB.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    buttonDownW = (HBITMAP)LoadImage(NULL, "assets/buttonDownW.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    buttonDownB = (HBITMAP)LoadImage(NULL, "assets/buttonDownB.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    buttonUpW = (HBITMAP)LoadImage(NULL, "assets/buttonUpW.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    buttonUpB = (HBITMAP)LoadImage(NULL, "assets/buttonUpB.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    startWhite = (HBITMAP)LoadImage(NULL, "assets/startWhite.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    startBlack = (HBITMAP)LoadImage(NULL, "assets/startBlack.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    leaderboardW = (HBITMAP)LoadImage(NULL, "assets/leaderboardW.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    leaderboardB = (HBITMAP)LoadImage(NULL, "assets/leaderboardB.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    player1WR = (HBITMAP)LoadImage(NULL, "assets/playerWR.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    player1WL = (HBITMAP)LoadImage(NULL, "assets/playerWL.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    player1BR = (HBITMAP)LoadImage(NULL, "assets/playerBR.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    player1BL = (HBITMAP)LoadImage(NULL, "assets/playerBL.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    player2WR = (HBITMAP)LoadImage(NULL, "assets/player2WR.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    player2WL = (HBITMAP)LoadImage(NULL, "assets/player2WL.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    player2BR = (HBITMAP)LoadImage(NULL, "assets/player2BR.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    player2BL = (HBITMAP)LoadImage(NULL, "assets/player2BL.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    powerB = (HBITMAP)LoadImage(NULL, "assets/powerB.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    powerW = (HBITMAP)LoadImage(NULL, "assets/powerW.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);


}

void deleteBitmaps()
{

    DeleteObject(bk);
    DeleteObject(box);
    DeleteObject(bkGameOver);
    DeleteObject(player1WR);
    DeleteObject(player1WL);
    DeleteObject(player1BR);
    DeleteObject(player1BL);

    DeleteObject(player2WR);
    DeleteObject(player2WL);
    DeleteObject(player2BR);
    DeleteObject(player2BL);
}

void setDefaults()
{
    player1.x = WIDTH / 6 + 5;
    player2.x = WIDTH / 6;
    player1.y = player1.initY = player2.y = player2.initY = 0;
    boxObj.x = WIDTH / 2;
    boxObj1.x = WIDTH / 2 - 70;
    boxObj2.x = WIDTH / 2 - 140;
    boxObj3.x = WIDTH / 2 - 210;
    boxObj4.x = WIDTH / 2 - 280;
    upperPlatform.y = 330;
    upperPlatform2.y = 330;
    boxObj.y = 300;
    boxObj1.y = 250;
    boxObj2.y = 200;
    boxObj3.y = 150;
    boxObj4.y = 100;
    if (level == 0)
        objects.push_back(&platform);
    if (level == 1)
    {
        objects.push_back(&leftPlt);
        objects.push_back(&rightPlt);
        objects.push_back(&rightWall);
        objects.push_back(&leftWall);
        objects.push_back(&upperPlatform);
    }
    if(level == 2){
        objects.push_back(&leftPlt2);
        objects.push_back(&rightPlt2);
        objects.push_back(&rightWall);
        objects.push_back(&leftWall);
        objects.push_back(&upperPlatform2);
        objects.push_back(&boxObj);
        objects.push_back(&boxObj1);
        objects.push_back(&boxObj2);
        objects.push_back(&boxObj3);
        objects.push_back(&boxObj4);
    }

    objects.push_back(&player1);
    objects.push_back(&player2);

    buttonW1 = buttonUpW;
    buttonB1 = buttonUpB;

    // objects.push_back(&boxObj);
}

bool shouldFall(Object *obj)
{
    return filterObjects(objects, obj).empty();
}

void checkStart(HWND hwnd)
{
    if (player1.x > WIDTH / 3 - 50 && player1.x < WIDTH / 3 && player1.y == 150 && !player1.isFalling || player2.x > WIDTH / 3 - 30 && player2.x < WIDTH / 3 && player2.y == 150 && !player2.isFalling)
    {
        DialogBox(NULL, MAKEINTRESOURCE(IDD_TEAM_NAME), hwnd, DlgProcTeamName);
    }
    // WIDTH / 3 - bm.bmWidth / 2, HEIGHT / 3,
}

void checkLeaderboard(HWND hwnd)
{
    if (player1.x > 2 * WIDTH / 3 - 50 && player1.x < 2 * WIDTH / 3 && player1.y == 150 && !player1.isFalling || player2.x > 2 * WIDTH / 3 - 30 && player2.x < 2 * WIDTH / 3 && player2.y == 150 && !player2.isFalling)
    {
        DialogBox(NULL, MAKEINTRESOURCE(IDD_LEADERBOARD), hwnd, DlgProcLeaderboard);
    }
}

bool isBlocked(Object *p, bool isRight)
{
    if (isRight)
    {
        for (auto obj : objects)
        {
            if (p->x + p->width == obj->x && !obj->isPlayer && p->y < obj->y + obj->height && p->y + p->height > obj->y)
                return true;
        }
        return false;
    }
    else
    {
        for (auto obj : objects)
        {
            if (p->x == obj->x + obj->width && !obj->isPlayer && p->y < obj->y + obj->height && p->y + p->height > obj->y)
                return true;
        }
        return false;
    }
}

void setLevel(int l)
{
    level = l;
    objects.clear();
    setDefaults();
}

INT_PTR CALLBACK DlgProcTeamName(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {

    case WM_COMMAND:
    {

        switch (LOWORD(wParam))
        {
        case IDC_SUBMIT:
        {
            char tempUnos[50];
            tempUnos[0] = 0;
            GetWindowText(GetDlgItem(hdlg, IDC_TEAM_NAME), tempUnos, sizeof(tempUnos));

            // if (tempUnos[0] == 0)
            // {
            //     MessageBox(hdlg, "Potrebno je unijeti sve podatke", "Gre�ka", MB_OK | MB_ICONWARNING);
            //     break;
            // }
            cout << string(tempUnos);
            teamName = string(tempUnos);
            setLevel(1);
            EndDialog(hdlg, 0);
        }
        }
        return TRUE;
    }
    case WM_CLOSE:
    {
        EndDialog(hdlg, 0);
        return TRUE;
    }
    default:
        return FALSE;
    }
}

INT_PTR CALLBACK DlgProcLeaderboard(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    int status;
    char *err = 0;
    switch (message)
    {

    case WM_INITDIALOG:
    {
        // listHandle = GetDlgItem(hdlg, IDC_LISTA_STUDENATA);
        // InitListViewColumns(listHandle);

        return TRUE;
    }
    case WM_COMMAND:
    {
        switch (LOWORD(wParam))
        {
            // case SEARCH_ALL:
            //     type = "all";
            //     break;
            // case SEARCH_IME:
            //     type = "ime";
            //     break;
            // case SEARCH_PREZIME:
            //     type = "prezime";
            //     break;

            // case IDC_SAVE:
            // {
            //     ListView_DeleteAllItems(listHandle);
            //     sqlite3_open("studenti.db", &db);
            //     int status;
            //     char *err = 0;
            //     char sql[200], tempUnos[50];
            //     tempUnos[0] = 0;
            //     GetWindowText(GetDlgItem(hdlg, SEARCH), tempUnos, sizeof(tempUnos));

            //     if (tempUnos[0] == 0 && type != "all")
            //     {
            //         MessageBox(hdlg, "Potrebno je unijeti kriterij za pretragu", "Gre�ka", MB_OK | MB_ICONWARNING);
            //         break;
            //     }
            //     if (type == "all")
            //         sprintf(sql, "SELECT * FROM Studenti");
            //     else if (type == "ime")
            //         sprintf(sql, "SELECT * FROM Studenti WHERE ime = '%s'", tempUnos);
            //     else
            //         sprintf(sql, "SELECT * FROM Studenti WHERE prezime = '%s'", tempUnos);

            //     status = sqlite3_exec(db, sql, callback, 0, &err);
            //     if (status == SQLITE_OK)
            //     {

            //         sqlite3_free(err);
            //         sqlite3_close(db);
            //     }
            //     else
            //     {
            //         MessageBox(hdlg, err, "Gre�ka", MB_OK);
            //         sqlite3_free(err);
            //         sqlite3_close(db);
            //         EndDialog(hdlg, 0);
            //         break;
            //     }
            //     break;
            // }
            // }
            return TRUE;
        }
    case WM_CLOSE:
    {
        EndDialog(hdlg, 0);
        return TRUE;
    }
    default:
        return FALSE;
    }
    }
}