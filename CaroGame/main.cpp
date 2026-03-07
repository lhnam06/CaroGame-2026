/*
 * CaroGame - Do an Co So Lap Trinh
 * Truong Dai hoc Khoa hoc Tu nhien TP.HCM
 * C++ - Lap trinh thu tuc (khong su dung OOP)
 */

#include <iostream>
#include <cctype>
#include "Defs/Defs.h"
#include "View/View.h"
#include "Model/Model.h"
#include "Control/Control.h"
#include "SaveLoad/SaveLoad.h"
#include "Menu/Menu.h"

#ifdef _WIN32
#include <conio.h>
#endif

int main(void) {
    int validEnter;
    int menuChoice;

#ifdef _WIN32
    FixConsoleWindow();
#endif

    _WIN_P1 = 0;
    _WIN_P2 = 0;

    menuChoice = ShowMainMenu();
    if (menuChoice == 2) {
        ResetData();
        LoadGame();
    } else if (menuChoice == 3) {
        ExitGame();
        return 0;
    } else {
        StartGame();
    }

    validEnter = 1;
    while (1) {
#ifdef _WIN32
        _COMMAND = toupper(getch());
#else
        _COMMAND = toupper(std::cin.get());
#endif

        if (_COMMAND == 27) {
            ExitGame();
            return 0;
        } else if (_COMMAND == 'L') {
            SaveGame();
        } else if (_COMMAND == 'T') {
            LoadGame();
        } else if (_COMMAND == 'A') {
            MoveLeft();
        } else if (_COMMAND == 'W') {
            MoveUp();
        } else if (_COMMAND == 'S') {
            MoveDown();
        } else if (_COMMAND == 'D') {
            MoveRight();
        } else if (_COMMAND == 13) {
            switch (CheckBoard(_X, _Y)) {
                case -1: std::cout << "X"; break;
                case 1:  std::cout << "O"; break;
                case 0:  validEnter = 0; break;
            }

            if (validEnter == 1) {
                switch (ProcessFinish(TestBoard())) {
                    case -1: case 1: case 0:
                        if (AskContinue() != 'Y') {
                            ExitGame();
                            return 0;
                        }
                        StartGame();
                        break;
                }
            }
            validEnter = 1;
        }
    }

    return 0;
}
