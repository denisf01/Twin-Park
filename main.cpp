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

class Object
{
public:
    int width;
    int height;
    int x;
    int y;
};

class Player : public Object
{
public:
    Player() : Object(), move{false}, isJumping{false}, isRight{true} {}
    bool move;
    bool isJumping;
    DWORD ticks;
    int i;
    int j;
    bool isRight;
    int jumpTicks;
};

Player player1;
Player player2;

bool gameOver = false;
bool allowJump = true;

int introInitHeight = 500;

HBITMAP bk, bkGameOver, player1WR, player1WL, player1BR, player1BL, titleWhite, titleBlack, startWhite, startBlack;
HBITMAP player2WR, player2WL, player2BR, player2BL;

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
        if (player1.isJumping && GetTickCount() - player1.jumpTicks > 50 || player2.isJumping && GetTickCount() - player2.jumpTicks > 50)
        {
            jumping();
        }
        draw(hwnd);
        // isGameOver(hwnd);
        Sleep(40);
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

    // player1
    SelectObject(hdcTmp, player1White);
    GetObject(player1White, sizeof(BITMAP), &bm);
    player1.width = bm.bmWidth / 8;
    player1.height = bm.bmHeight / 2;
    BitBlt(hdcMem, player1.x, introInitHeight - player1.height + 30 - player1.y, player1.width, player1.height, hdcTmp, player1.i * player1.width, player1.j * player1.height, SRCAND);
    SelectObject(hdcTmp, player1Black);
    BitBlt(hdcMem, player1.x, introInitHeight - player1.height + 30 - player1.y, player1.width, player1.height, hdcTmp, player1.i * player1.width, player1.j * player1.height, SRCPAINT);

    // player2
    SelectObject(hdcTmp, player2White);
    GetObject(player2White, sizeof(BITMAP), &bm);
    player2.width = bm.bmWidth / 8;
    player2.height = bm.bmHeight / 2;
    BitBlt(hdcMem, player2.x, introInitHeight - player2.height + 30 - player2.y, player2.width, player2.height, hdcTmp, player2.i * player2.width, player2.j * player2.height, SRCAND);
    SelectObject(hdcTmp, player2Black);
    BitBlt(hdcMem, player2.x, introInitHeight - player2.height + 30 - player2.y, player2.width, player2.height, hdcTmp, player2.i * player2.width, player2.j * player2.height, SRCPAINT);

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
        if (VK_SPACE)
            allowJump = true;
        player1.i = 2;
        player1.j = 0;
        break;
    case WM_LBUTTONDOWN:
        cout << LOWORD(lParam) << " " << HIWORD(lParam) << endl;
        gameOver = false;
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
    if (isPressed(VK_UP))
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
        if (++player1.i > 7)
        {
            player1.i = 0;
            ++player1.j %= 2;
        }
    }
    if (isPressed(VK_LEFT))
    {
        player1.isRight = false;
        if (player1.x >= 0)
            player1.x -= 10;
        if (++player1.i > 3)
        {
            player1.i = 0;
            ++player1.j %= 2;
        }
    }

    if (isPressed(0x57)) // VK_W
    {
        if (allowJump && (player2.y == 0 && (player2.y = 49) || (player2.y == 149 && player2.x > 500 && player2.x < 800 - player2.width) && (player2.y = 199)))
        {
            player2.isJumping = true;
            allowJump = false;
            player2.jumpTicks = GetTickCount();
        }
    }
    if (isPressed(0x44)) // VK_D
    {
        player2.isRight = true;
        if (player2.x + player2.width <= WIDTH)
            player2.x += 10;
        if (++player2.i > 7)
        {
            player2.i = 0;
            ++player2.j %= 2;
        }
    }
    if (isPressed(0x41)) // VK_A
    {
        player2.isRight = false;
        if (player2.x >= 0)
            player2.x -= 10;
        if (++player2.i > 3)
        {
            player2.i = 0;
            ++player2.j %= 2;
        }
    }
}

void jumping()
{
    if (player1.y == 50)
    {
        player1.isJumping = false;
        player1.y = 0;
    }
    else if (player1.y == 149)
    {
        player1.y = 200;
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

    player1WR = (HBITMAP)LoadImage(NULL, "playerWR.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    player1WL = (HBITMAP)LoadImage(NULL, "playerWL.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    player1BR = (HBITMAP)LoadImage(NULL, "playerBR.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    player1BL = (HBITMAP)LoadImage(NULL, "playerBL.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);

    player2WR = (HBITMAP)LoadImage(NULL, "playerWR.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    player2WL = (HBITMAP)LoadImage(NULL, "playerWL.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    player2BR = (HBITMAP)LoadImage(NULL, "playerBR.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    player2BL = (HBITMAP)LoadImage(NULL, "playerBL.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
}

void deleteBitmaps()
{

    DeleteObject(bk);
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
    player1.x = WIDTH / 2;
    player2.x = WIDTH / 6;
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