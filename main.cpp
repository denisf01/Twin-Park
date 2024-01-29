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
#include "resources.h"

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
INT_PTR CALLBACK DlgProcTeamName(HWND hdlg, UINT message, WPARAM wParam, LPARAM lParam);

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

Object player1;
Object player2;
Object wolf;
Object bird;

bool gameOver = false;
bool isWolf = true;
bool allowJump = true;

int initHeight = 500;

HBITMAP bk, bkGameOver, sheepWR, sheepWL, sheepBR, sheepBL, birdWR, birdWL, birdBR, birdBL, arrowWR, arrowWL, arrowBR, arrowBL, wolfWR, wolfWL, wolfBR, wolfBL, titleWhite, titleBlack, startWhite, startBlack;

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
        if (player1.isJumping && GetTickCount() - player1.jumpTicks > 50)
        {

            jumping();
        }
        if (bird.move)
            calculateBirdPosition(hwnd);
        else if (GetTickCount() - bird.ticks > 7000)
            //  bird.move = true;
            if (wolf.move)
                calculateWolfPosition(hwnd);
            else if (GetTickCount() - wolf.ticks > 5000)
            {
                isWolf = player1.y < 149;
                //   wolf.move = true;
            }
        draw(hwnd);
        // isGameOver(hwnd);
        Sleep(40);
    }

    /* The program return-value is 0 - The value that PostQuitMessage() gave */
    return messages.wParam;
}

void isGameOver(HWND hwnd)
{
    if ((!wolf.isRight && wolf.x > player1.x && wolf.x < player1.x + player1.width - 60 && ((player1.y == 0 && isWolf) || (player1.y == 149 && !isWolf))) ||
        (wolf.isRight && wolf.x + wolf.width - 60 > player1.x && wolf.x + wolf.width < player1.x + player1.width && ((player1.y == 0 && isWolf) || (player1.y == 149 && !isWolf))))
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
    sheepWhite = player1.isRight ? sheepWR : sheepWL;
    sheepBlack = player1.isRight ? sheepBR : sheepBL;
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

    // title
    SelectObject(hdcTmp, titleWhite);
    GetObject(titleWhite, sizeof(BITMAP), &bm);
    BitBlt(hdcMem, WIDTH / 2 - bm.bmWidth / 2, HEIGHT / 10, bm.bmWidth, bm.bmHeight, hdcTmp, 0, 0, SRCAND);
    SelectObject(hdcTmp, titleBlack);
    BitBlt(hdcMem, WIDTH / 2 - bm.bmWidth / 2, HEIGHT / 10, bm.bmWidth, bm.bmHeight, hdcTmp, 0, 0, SRCPAINT);

    // start button

    SelectObject(hdcTmp, startWhite);
    GetObject(startWhite, sizeof(BITMAP), &bm);
    BitBlt(hdcMem, WIDTH / 2 - bm.bmWidth / 2, HEIGHT / 3, bm.bmWidth, bm.bmHeight, hdcTmp, 0, 0, SRCAND);
    SelectObject(hdcTmp, startBlack);
    BitBlt(hdcMem, WIDTH / 2 - bm.bmWidth / 2, HEIGHT / 3, bm.bmWidth, bm.bmHeight, hdcTmp, 0, 0, SRCPAINT);

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

    // player1
    SelectObject(hdcTmp, sheepWhite);
    GetObject(sheepWhite, sizeof(BITMAP), &bm);
    player1.width = bm.bmWidth / 8;
    player1.height = bm.bmHeight / 2;
    BitBlt(hdcMem, player1.x, initHeight - player1.height + 30 - player1.y, player1.width, player1.height, hdcTmp, player1.i * player1.width, player1.j * player1.height, SRCAND);
    SelectObject(hdcTmp, sheepBlack);
    BitBlt(hdcMem, player1.x, initHeight - player1.height + 30 - player1.y, player1.width, player1.height, hdcTmp, player1.i * player1.width, player1.j * player1.height, SRCPAINT);

    // player2
    SelectObject(hdcTmp, sheepWhite);
    GetObject(sheepWhite, sizeof(BITMAP), &bm);
    player2.width = bm.bmWidth / 8;
    player2.height = bm.bmHeight / 2;
    BitBlt(hdcMem, player2.x, initHeight - player2.height + 30 - player2.y, player2.width, player2.height, hdcTmp, player2.i * player2.width, player2.j * player2.height, SRCAND);
    SelectObject(hdcTmp, sheepBlack);
    BitBlt(hdcMem, player2.x, initHeight - player2.height + 30 - player2.y, player2.width, player2.height, hdcTmp, player2.i * player2.width, player2.j * player2.height, SRCPAINT);

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
        player1.i = 2;
        player1.j = 0;
        break;
    case WM_LBUTTONDOWN:
        cout << LOWORD(lParam) << " " << HIWORD(lParam) << endl;
        gameOver = false;
        bird.ticks = GetTickCount();
        wolf.ticks = GetTickCount();
        DialogBox(NULL, MAKEINTRESOURCE(IDD_TEAM_NAME), hwnd, DlgProcTeamName);

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
        if (allowJump && (player1.y == 0 && (player1.y = 49) || (player1.y == 149 && player1.x > 500 && player1.x < 800 - player1.width) && (player1.y = 199)))
        {
            player1.isJumping = true;
            allowJump = false;
            player1.jumpTicks = GetTickCount();
        }
    }
    if (isPressed(VK_RIGHT))
    {
        player1.isRight = true;
        if (player1.x + player1.width <= WIDTH)
            player1.x += 10;
        if (player1.y == 149 && (player1.x < 500 || player1.x > 800 - player1.width) && !player1.isJumping)
        {
            player1.y = 150;
            player1.isJumping = true;
        }
        if (++player1.i > 7)
        {
            player1.i = 0;
            ++player1.j %= 2;
        }
    }

    if (isPressed(0x44))
    {
        player2.isRight = true;
        if (player2.x + player2.width <= WIDTH)
            player2.x += 10;
        if (player2.y == 149 && (player2.x < 500 || player2.x > 800 - player2.width) && !player2.isJumping)
        {
            player2.y = 150;
            player2.isJumping = true;
        }
        if (++player2.i > 7)
        {
            player2.i = 0;
            ++player2.j %= 2;
        }
    }

    if (isPressed(VK_LEFT))
    {
        player1.isRight = false;
        if (player1.x >= 0)
            player1.x -= 10;
        if (player1.y == 149 && (player1.x < 500 || player1.x > 800 - player1.width) && !player1.isJumping)
        {
            player1.y = 150;
            player1.isJumping = true;
        }
        if (++player1.i > 3)
        {
            player1.i = 0;
            ++player1.j %= 2;
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
    if (player1.y == 200 && player1.x > 500 && player1.x < 800 - player1.width)
    {
        player1.isJumping = false;
        player1.y = 149;
    }
    else if (player1.y == 50)
    {
        player1.isJumping = false;
        player1.y = 0;
    }
    else if (player1.y == 149)
    {
        player1.y = 200;
        cout << "test";
    }
    else if (player1.y == 349)
        player1.y = 300;
    else if (player1.y % 10)
        player1.y += 50;
    else
        player1.y -= 50;

    player1.jumpTicks = GetTickCount();
}

void loadBitmaps()
{
    bk = (HBITMAP)LoadImage(NULL, "introBackground.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    bkGameOver = (HBITMAP)LoadImage(NULL, "gameOver.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    titleWhite = (HBITMAP)LoadImage(NULL, "titleWhite.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    titleBlack = (HBITMAP)LoadImage(NULL, "titleBlack.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    startWhite = (HBITMAP)LoadImage(NULL, "startWhite.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    startBlack = (HBITMAP)LoadImage(NULL, "startBlack.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    sheepWR = (HBITMAP)LoadImage(NULL, "playerWR.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    sheepWL = (HBITMAP)LoadImage(NULL, "playerWL.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    sheepBR = (HBITMAP)LoadImage(NULL, "playerBR.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    sheepBL = (HBITMAP)LoadImage(NULL, "playerBL.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

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
    player1.x = WIDTH / 2;
    player2.x = WIDTH / 6;
    wolf.x = -200;
    bird.x = -100;
    bird.y = -35;
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