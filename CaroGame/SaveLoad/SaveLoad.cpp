/*
 * SaveLoad - Luu va tai game
 */

#include <iostream>
#include <cstdlib>
#include <fstream>
#include "Defs/Defs.h"
#include "View/View.h"
#include "Model/Model.h"
#include "SaveLoad/SaveLoad.h"

void SaveGame(void) {
    char filename[256];

    ClearMessageArea();
    GotoXY(LEFT, MESSAGE_Y);
    std::cout << "Nhap ten file muon luu: ";
    std::cin >> filename;

    std::ofstream f(filename);
    if (!f) {
        ShowMessage("Khong the tao file!");
        return;
    }

    f << _TURN << " " << _X << " " << _Y << " "
      << _WIN_P1 << " " << _WIN_P2 << " " << _MOVE_P1 << " " << _MOVE_P2 << "\n";
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            f << _A[i][j].c << " ";
        }
        f << "\n";
    }
    f.close();

    ShowMessage("Luu thanh cong!");
    GotoXY(_X, _Y);
}

void LoadGame(void) {
    char filename[256];

    ClearMessageArea();
    GotoXY(LEFT, MESSAGE_Y);
    std::cout << "Nhap ten file muon tai: ";
    std::cin >> filename;

    std::ifstream f(filename);
    if (!f) {
        ShowMessage("Khong tim thay file!");
        GotoXY(_X, _Y);
        return;
    }

    f >> _TURN >> _X >> _Y >> _WIN_P1 >> _WIN_P2 >> _MOVE_P1 >> _MOVE_P2;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            int c;
            f >> c;
            _A[i][j].c = c;
            _A[i][j].x = 4 * j + LEFT + 2;
            _A[i][j].y = 2 * i + TOP + 1;
        }
    }
    f.close();

    system("cls");
    DrawBoard(BOARD_SIZE);
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (_A[i][j].c != 0) {
                GotoXY(_A[i][j].x, _A[i][j].y);
                std::cout << (_A[i][j].c == -1 ? "X" : "O");
            }
        }
    }
    DrawPlayerInfo();
    ClearMessageArea();
    GotoXY(_X, _Y);
}
