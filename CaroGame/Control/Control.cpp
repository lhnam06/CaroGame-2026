/*
 * Control - Dieu khien di chuyen va game flow
 */

#include <cstdlib>
#include "Defs/Defs.h"
#include "View/View.h"
#include "Model/Model.h"
#include "Control/Control.h"

void MoveRight(void) {
    if (_X < _A[BOARD_SIZE - 1][BOARD_SIZE - 1].x) {
        _X += 4;
        GotoXY(_X, _Y);
    }
}

void MoveLeft(void) {
    if (_X > _A[0][0].x) {
        _X -= 4;
        GotoXY(_X, _Y);
    }
}

void MoveDown(void) {
    if (_Y < _A[BOARD_SIZE - 1][BOARD_SIZE - 1].y) {
        _Y += 2;
        GotoXY(_X, _Y);
    }
}

void MoveUp(void) {
    if (_Y > _A[0][0].y) {
        _Y -= 2;
        GotoXY(_X, _Y);
    }
}

void StartGame(void) {
    system("cls");
    ResetData();
    DrawBoard(BOARD_SIZE);
    DrawPlayerInfo();
    GotoXY(_X, _Y);
}

void ExitGame(void) {
    system("cls");
    GabageCollect();
}
