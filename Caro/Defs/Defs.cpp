/*
 * CaroGame - Dinh nghia bien toan cuc
 */

#include "Defs/Defs.h"

struct _POINT _A[BOARD_SIZE][BOARD_SIZE];
int _TURN;
int _COMMAND;
int _X, _Y;
int _WIN_P1, _WIN_P2, _MOVE_P1, _MOVE_P2;
int _WIN_R1, _WIN_C1;
int _WIN_R2, _WIN_C2;
int _VS_BOT = 0;

int _BOARD_THEME = 0;

int _MOVE_ORDER[BOARD_SIZE][BOARD_SIZE];
int _MOVE_COUNT = 0;

_UNDO_ENTRY _UNDO_STACK[UNDO_MAX];
int _UNDO_TOP = 0;
