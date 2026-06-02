/*
 * CaroGame - Dinh nghia hang so, kieu du lieu, bien toan cuc
 */

#ifndef DEFS_H
#define DEFS_H

#include <tchar.h>

#define BOARD_SIZE 12
#define DESIGN_W   1280
#define DESIGN_H   960

extern int CELL_SIZE;
extern int OFFSET_X;
extern int OFFSET_Y;
extern int SCREEN_W;
extern int SCREEN_H;
extern float UI_SCALE;

#define CHAR_COUNT 23

struct _POINT {
    int x, y, c;  /* x: dong, y: cot, c: 0=trong, -1=X, 1=O */
};

extern struct _POINT _A[BOARD_SIZE][BOARD_SIZE];
extern int _TURN;
extern int _COMMAND;
extern int _X, _Y;
extern int _WIN_P1, _WIN_P2, _MOVE_P1, _MOVE_P2;
extern int _WIN_R1, _WIN_C1; // Tọa độ hàng, cột của viên đầu tiên
extern int _WIN_R2, _WIN_C2; // Tọa độ hàng, cột của viên cuối cùng
extern int _VS_BOT;          // 1 = human (X) vs computer (O), 0 = two humans
extern int _CHAR_P1;         // Selected character id (1..CHAR_COUNT)
extern int _CHAR_P2;
extern TCHAR _NAME_P1[32];   // Display name chosen in character select
extern TCHAR _NAME_P2[32];

// Board themes: 0 = warm wood, 1 = dark slate, 2 = golden
extern int _BOARD_THEME;

// Move-order tracking
extern int _MOVE_ORDER[BOARD_SIZE][BOARD_SIZE]; // 0 = empty, >0 = move number
extern int _MOVE_COUNT;                         // total moves placed so far

// Undo stack (each entry = {row, col, piece})
#define UNDO_MAX (BOARD_SIZE * BOARD_SIZE)
struct _UNDO_ENTRY { int r; int c; int piece; };
extern _UNDO_ENTRY _UNDO_STACK[UNDO_MAX];
extern int _UNDO_TOP;

#endif
