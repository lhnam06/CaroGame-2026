/*
 * Control - Dieu khien di chuyen va game flow
 */

#include <cstdlib>
#include "Defs/Defs.h"
#include "View/View.h"
#include "Model/Model.h"
#include "Control/Control.h"

void MoveRight(void) {
    if (_Y < BOARD_SIZE - 1) _Y++;
}

void MoveLeft(void) {
    if (_Y > 0) _Y--;
}

void MoveDown(void) {
    if (_X < BOARD_SIZE - 1) _X++;
}

void MoveUp(void) {
    if (_X > 0) _X--;
}

void StartGame(void) {
    ResetData();
    DrawBoard(); 
}

void ExitGame(void) {
    GabageCollect();
}