/*
 * View - Giao dien man hinh
 */

#include <iostream>
#include <cctype>
#include "Defs/Defs.h"
#include "View/View.h"

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#endif

void FixConsoleWindow(void) {
#ifdef _WIN32
    HWND consoleWindow = GetConsoleWindow();
    LONG style = GetWindowLong(consoleWindow, GWL_STYLE);
    style = style & ~(WS_MAXIMIZEBOX) & ~(WS_THICKFRAME);
    SetWindowLong(consoleWindow, GWL_STYLE, style);
#endif
}

void GotoXY(int x, int y) {
#ifdef _WIN32
    COORD coord;
    coord.X = x;
    coord.Y = y;
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), coord);
#else
    std::cout << "\033[" << (y + 1) << ";" << (x + 1) << "H";
#endif
}

void DrawBoard(int pSize) {
    for (int i = 0; i <= pSize; i++) {
        for (int j = 0; j <= pSize; j++) {
            GotoXY(LEFT + 4 * i, TOP + 2 * j);
            std::cout << ".";
        }
    }
}

void DrawPlayerInfo(void) {
    int infoY = TOP + 2 * BOARD_SIZE + 2;
    GotoXY(LEFT, infoY);
    std::cout << "Nguoi 1 (X): " << _MOVE_P1 << " nuoc | " << _WIN_P1 << " van thang";
    GotoXY(LEFT, infoY + 1);
    std::cout << "Nguoi 2 (O): " << _MOVE_P2 << " nuoc | " << _WIN_P2 << " van thang";
    GotoXY(LEFT, infoY + 2);
    std::cout << "Phim: W,A,S,D | Enter danh | L luu | T tai | ESC thoat";
}

void DrawWinEffect(int pWhoWin) {
    int y = _A[BOARD_SIZE - 1][BOARD_SIZE - 1].y + 2;
    GotoXY(LEFT, y);
    for (int i = 0; i < 50; i++) std::cout << "=";
    GotoXY(LEFT, y + 1);
    if (pWhoWin == -1)
        std::cout << "  >>> CHUC MUNG! Nguoi choi 1 (X) da thang! <<<  ";
    else if (pWhoWin == 1)
        std::cout << "  >>> CHUC MUNG! Nguoi choi 2 (O) da thang! <<<  ";
    else if (pWhoWin == 0)
        std::cout << "  >>> HAI BEN HOA NHAU! <<<  ";
    GotoXY(LEFT, y + 2);
    for (int i = 0; i < 50; i++) std::cout << "=";
}

int ProcessFinish(int pWhoWin) {
    GotoXY(0, _A[BOARD_SIZE - 1][BOARD_SIZE - 1].y + 2);
    switch (pWhoWin) {
        case -1:
            _WIN_P1++;
            DrawWinEffect(-1);
            break;
        case 1:
            _WIN_P2++;
            DrawWinEffect(1);
            break;
        case 0:
            DrawWinEffect(0);
            break;
        case 2:
            _TURN = !_TURN;
            break;
    }
    GotoXY(_X, _Y);
    return pWhoWin;
}

int AskContinue(void) {
    GotoXY(0, _A[BOARD_SIZE - 1][BOARD_SIZE - 1].y + 5);
    std::cout << "Nhan 'Y' tiep tuc, phim bat ky de thoat: ";
#ifdef _WIN32
    return toupper(getch());
#else
    return toupper(std::cin.get());
#endif
}
