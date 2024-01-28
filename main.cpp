#if defined(UNICODE) && !defined(_UNICODE)
#define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
#define UNICODE
#endif

#include <tchar.h>
#include <windows.h>
#include <iostream>
#include <thread>
#include <random>

#define WIDTH 1000
#define HEIGHT 700
#define isPressed(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)

using namespace std;

void isGameOver(HWND);
void draw(HWND);
void calculateSheepPosition(HWND);
void calculateBirdPosition(HWND);
void calculateWolfPosition(HWND);
void jumping();
void loadBitmaps();
void deleteBitmaps();
void setDefaults();

struct Object
{
    Object() : move{false}, isJumping{false}, isRight{true} {}

    int width;
    int height;
    int x;
    int y;
    bool move;
    bool isJumping;
    DWORD ticks;
    int i;
    int j;
    bool isRight;
    int jumpTicks;
};

Object sheep;
Object wolf;
Object bird;

bool gameOver = false;
bool isWolf = true;
bool allowJump = true;

HBITMAP bk, bkGameOver, platformWhite, platformBlack, sheepWR, sheepWL, sheepBR, sheepBL, birdWR, birdWL, birdBR, birdBL, arrowWR, arrowWL, arrowBR, arrowBL, wolfWR, wolfWL, wolfBR, wolfBL;

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
        if (sheep.isJumping && GetTickCount() - sheep.jumpTicks > 50)
        {

            jumping();
        }
        if (bird.move)
            calculateBirdPosition(hwnd);
        else if (GetTickCount() - bird.ticks > 7000)
            bird.move = true;
        if (wolf.move)
            calculateWolfPosition(hwnd);
        else if (GetTickCount() - wolf.ticks > 5000)
        {
            isWolf = sheep.y < 149;
            wolf.move = true;
        }
        draw(hwnd);
        isGameOver(hwnd);
        Sleep(40);
    }

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}

void isGameOver(HWND hwnd)
{
    if ((!wolf.isRight && wolf.x > sheep.x && wolf.x < sheep.x + sheep.width - 60 && ((sheep.y == 0 && isWolf) || (sheep.y == 149 && !isWolf))) ||
        (wolf.isRight && wolf.x + wolf.width - 60 > sheep.x && wolf.x + wolf.width < sheep.x + sheep.width && ((sheep.y == 0 && isWolf) || (sheep.y == 149 && !isWolf))))
    {
        wolf.move = false;
        bird.move = false;
        gameOver = true;
        wolf.x = -200;
        wolf.isRight = true;
        bird.isRight = true;
        bird.x = -100;
        bird.y = -35;
        draw(hwnd);
    }
}

void draw(HWND hwnd)
{
    HDC hdc = GetDC(hwnd);
    HDC hdcMem, hdcTmp;
    HBITMAP hbmMem, background, sheepWhite, sheepBlack, wolfWhite, wolfBlack, birdWhite, birdBlack, arrowWhite, arrowBlack, tmp1, tmp2;
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
    sheepWhite = sheep.isRight ? sheepWR : sheepWL;
    sheepBlack = sheep.isRight ? sheepBR : sheepBL;
    wolfWhite = wolf.isRight ? wolfWR : wolfWL;
    wolfBlack = wolf.isRight ? wolfBR : wolfBL;
    arrowWhite = wolf.isRight ? arrowWR : arrowWL;
    arrowBlack = wolf.isRight ? arrowBR : arrowBL;
    birdWhite = bird.isRight ? birdWR : birdWL;
    birdBlack = bird.isRight ? birdBR : birdBL;

    hdcMem = CreateCompatibleDC(hdc);
    hbmMem = CreateCompatibleBitmap(hdc, WIDTH, HEIGHT);
    tmp1 = (HBITMAP)SelectObject(hdcMem, hbmMem);
    hdcTmp = CreateCompatibleDC(hdc);

    // background
    tmp2 = (HBITMAP)SelectObject(hdcTmp, background);
    GetObject(background, sizeof(BITMAP), &bm);
    StretchBlt(hdcMem, 0, 0, WIDTH, HEIGHT, hdcTmp, 0, 0, bm.bmWidth, bm.bmHeight, SRCCOPY);

    // platform
    SelectObject(hdcTmp, platformWhite);
    GetObject(platformWhite, sizeof(BITMAP), &bm);
    BitBlt(hdcMem, WIDTH / 2, 400, bm.bmWidth, bm.bmHeight, hdcTmp, 0, 0, SRCAND);
    SelectObject(hdcTmp, platformBlack);
    BitBlt(hdcMem, WIDTH / 2, 400, bm.bmWidth, bm.bmHeight, hdcTmp, 0, 0, SRCPAINT);

    // wolf or arrow
    if (isWolf)
    {
        SelectObject(hdcTmp, wolfWhite);
        GetObject(wolfWhite, sizeof(BITMAP), &bm);
        wolf.width = bm.bmWidth / 3;
        wolf.height = bm.bmHeight / 2;
        BitBlt(hdcMem, wolf.x, 575 - wolf.height + 30, wolf.width, wolf.height, hdcTmp, wolf.i * wolf.width, wolf.j * wolf.height, SRCAND);
        SelectObject(hdcTmp, wolfBlack);
        BitBlt(hdcMem, wolf.x, 575 - wolf.height + 30, wolf.width, wolf.height, hdcTmp, wolf.i * wolf.width, wolf.j * wolf.height, SRCPAINT);
    }
    else
    {
        SelectObject(hdcTmp, arrowWhite);
        GetObject(arrowWhite, sizeof(BITMAP), &bm);
        wolf.width = bm.bmWidth;
        wolf.height = bm.bmHeight;
        BitBlt(hdcMem, wolf.x, 570 - wolf.height - 140, wolf.width, wolf.height, hdcTmp, 0, 0, SRCAND);
        SelectObject(hdcTmp, arrowBlack);
        BitBlt(hdcMem, wolf.x, 570 - wolf.height - 140, wolf.width, wolf.height, hdcTmp, 0, 0, SRCPAINT);
    }

    // sheep
    SelectObject(hdcTmp, sheepWhite);
    GetObject(sheepWhite, sizeof(BITMAP), &bm);
    sheep.width = bm.bmWidth / 4;
    sheep.height = bm.bmHeight / 2;
    BitBlt(hdcMem, sheep.x, 575 - sheep.height + 30 - sheep.y, sheep.width, sheep.height, hdcTmp, sheep.i * sheep.width, sheep.j * sheep.height, SRCAND);
    SelectObject(hdcTmp, sheepBlack);
    BitBlt(hdcMem, sheep.x, 575 - sheep.height + 30 - sheep.y, sheep.width, sheep.height, hdcTmp, sheep.i * sheep.width, sheep.j * sheep.height, SRCPAINT);

    // bird
    SelectObject(hdcTmp, birdWhite);
    GetObject(birdWhite, sizeof(BITMAP), &bm);
    bird.width = bm.bmWidth / 5;
    bird.height = bm.bmHeight / 2;
    BitBlt(hdcMem, bird.x, bird.y, bird.width, bird.height, hdcTmp, bird.i * bird.width, bird.j * bird.height, SRCAND);
    SelectObject(hdcTmp, birdBlack);
    BitBlt(hdcMem, bird.x, bird.y, bird.width, bird.height, hdcTmp, bird.i * bird.width, bird.j * bird.height, SRCPAINT);

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
        bird.ticks = GetTickCount();
        wolf.ticks = GetTickCount();
        loadBitmaps();
        setDefaults();
        break;
    case WM_KEYUP:
        if (VK_SPACE)
            allowJump = true;
        sheep.i = 2;
        sheep.j = 0;
        break;
    case WM_LBUTTONDOWN:
        cout << LOWORD(lParam) << " " << HIWORD(lParam) << endl;
        gameOver = false;
        bird.ticks = GetTickCount();
        wolf.ticks = GetTickCount();
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
    if (isPressed(VK_SPACE))
    {
        if (allowJump && (sheep.y == 0 && (sheep.y = 49) || (sheep.y == 149 && sheep.x > 500 && sheep.x < 800 - sheep.width) && (sheep.y = 199)))
        {
            sheep.isJumping = true;
            allowJump = false;
            sheep.jumpTicks = GetTickCount();
        }
    }
    if (isPressed(VK_RIGHT) || isPressed(0x44))
    {
        sheep.isRight = true;
        if (sheep.x + sheep.width <= WIDTH)
            sheep.x += 10;
        if (sheep.y == 149 && (sheep.x < 500 || sheep.x > 800 - sheep.width) && !sheep.isJumping)
        {
            sheep.y = 150;
            sheep.isJumping = true;
        }
        if (++sheep.i > 3)
        {
            sheep.i = 0;
            ++sheep.j %= 2;
        }
    }

    else if (isPressed(VK_LEFT) || isPressed(0x41))
    {
        sheep.isRight = false;
        if (sheep.x >= 0)
            sheep.x -= 10;
        if (sheep.y == 149 && (sheep.x < 500 || sheep.x > 800 - sheep.width) && !sheep.isJumping)
        {
            sheep.y = 150;
            sheep.isJumping = true;
        }
        if (++sheep.i > 3)
        {
            sheep.i = 0;
            ++sheep.j %= 2;
        }
    }
}

void calculateBirdPosition(HWND hwnd)
{
    if (++bird.i > 4)
    {
        bird.i = 0;
        ++bird.j %= 2;
    }
    if (bird.isRight)
    {
        if (bird.x >= WIDTH + 100)
        {
            bird.isRight = false;
            bird.move = false;
            bird.ticks = GetTickCount();
        }
        else
            bird.x += 20;
        if (bird.x >= WIDTH / 2)
            bird.y -= 7;
        else
            bird.y += 7;
    }
    else
    {
        if (bird.x <= -100)
        {
            bird.move = false;
            bird.isRight = true;
            bird.ticks = GetTickCount();
        }
        else
            bird.x -= 20;
        if (bird.x >= WIDTH / 2)
            bird.y += 7;
        else
            bird.y -= 7;
    }
}

void calculateWolfPosition(HWND hwnd)
{
    if ((wolf.isRight && wolf.x >= WIDTH) || (!wolf.isRight && wolf.x <= -200))
    {
        wolf.move = false;
        wolf.isRight = rand() % 2;
        wolf.x = wolf.isRight ? -200 : WIDTH + 10;
        wolf.ticks = GetTickCount();
    }
    else
    {
        wolf.x += wolf.isRight ? 20 : -20;
        if (++wolf.i > 2)
        {
            wolf.i = 0;
            ++wolf.j %= 2;
        }
    }
}

void jumping()
{
    if (sheep.y == 200 && sheep.x > 500 && sheep.x < 800 - sheep.width)
    {
        sheep.isJumping = false;
        sheep.y = 149;
    }
    else if (sheep.y == 50)
    {
        sheep.isJumping = false;
        sheep.y = 0;
    }
    else if (sheep.y == 149)
    {
        sheep.y = 200;
        cout << "test";
    }
    else if (sheep.y == 349)
        sheep.y = 300;
    else if (sheep.y % 10)
        sheep.y += 50;
    else
        sheep.y -= 50;

    sheep.jumpTicks = GetTickCount();
}

void loadBitmaps()
{
    bk = (HBITMAP)LoadImage(NULL, "background.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    bkGameOver = (HBITMAP)LoadImage(NULL, "gameOver.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    platformWhite = (HBITMAP)LoadImage(NULL, "platformWhite.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    platformBlack = (HBITMAP)LoadImage(NULL, "platformBlack.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    sheepWR = (HBITMAP)LoadImage(NULL, "sheepWhiteRight.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    sheepWL = (HBITMAP)LoadImage(NULL, "sheepWhiteLeft.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    sheepBR = (HBITMAP)LoadImage(NULL, "sheepBlackRight.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    sheepBL = (HBITMAP)LoadImage(NULL, "sheepBlackLeft.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    birdWR = (HBITMAP)LoadImage(NULL, "birdWhiteRight.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    birdWL = (HBITMAP)LoadImage(NULL, "birdWhiteLeft.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    birdBR = (HBITMAP)LoadImage(NULL, "birdBlackRight.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    birdBL = (HBITMAP)LoadImage(NULL, "birdBlackLeft.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    arrowWR = (HBITMAP)LoadImage(NULL, "arrowWhiteRight.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    arrowWL = (HBITMAP)LoadImage(NULL, "arrowWhiteLeft.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    arrowBR = (HBITMAP)LoadImage(NULL, "arrowBlackRight.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    arrowBL = (HBITMAP)LoadImage(NULL, "arrowBlackLeft.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    wolfWR = (HBITMAP)LoadImage(NULL, "wolfWhiteRight.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    wolfWL = (HBITMAP)LoadImage(NULL, "wolfWhiteLeft.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    wolfBR = (HBITMAP)LoadImage(NULL, "wolfBlackRight.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    wolfBL = (HBITMAP)LoadImage(NULL, "wolfBlackLeft.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
}

void deleteBitmaps()
{

    DeleteObject(bk);
    DeleteObject(bkGameOver);
    DeleteObject(platformWhite);
    DeleteObject(platformBlack);
    DeleteObject(sheepWR);
    DeleteObject(sheepWL);
    DeleteObject(sheepBR);
    DeleteObject(sheepBL);
    DeleteObject(birdWR);
    DeleteObject(birdWL);
    DeleteObject(birdBR);
    DeleteObject(birdBL);
    DeleteObject(arrowWR);
    DeleteObject(arrowWL);
    DeleteObject(arrowBR);
    DeleteObject(arrowBL);
    DeleteObject(wolfWR);
    DeleteObject(wolfWL);
    DeleteObject(wolfBR);
    DeleteObject(wolfBL);
}

void setDefaults()
{
    sheep.x = WIDTH / 2;
    wolf.x = -200;
    bird.x = -100;
    bird.y = -35;
}
