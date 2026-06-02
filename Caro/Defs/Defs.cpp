/*
 * CaroGame - Dinh nghia bien toan cuc
 */

#include "Defs/Defs.h"

int CELL_SIZE  = 40;
int OFFSET_X   = 50;
int OFFSET_Y   = 50;
int SCREEN_W   = 800;
int SCREEN_H   = 600;
float UI_SCALE = 1.0f;

struct _POINT _A[BOARD_SIZE][BOARD_SIZE];
int _TURN;
int _COMMAND;
int _X, _Y;
int _WIN_P1, _WIN_P2, _MOVE_P1, _MOVE_P2;
int _WIN_R1, _WIN_C1;
int _WIN_R2, _WIN_C2;
int _VS_BOT = 0;
int _CHAR_P1 = 1;
int _CHAR_P2 = 2;
TCHAR _NAME_P1[32] = _T("Red");
TCHAR _NAME_P2[32] = _T("Blue");

int _BOARD_THEME = 0;

int _MOVE_ORDER[BOARD_SIZE][BOARD_SIZE];
int _MOVE_COUNT = 0;

_UNDO_ENTRY _UNDO_STACK[UNDO_MAX];
int _UNDO_TOP = 0;
