/*
 * CaroGame - Dinh nghia hang so, kieu du lieu, bien toan cuc
 */

#ifndef DEFS_H
#define DEFS_H

#define BOARD_SIZE 12
#define LEFT 3
#define TOP 1

/* Vung hien thi thong bao - duoi ban co va thong tin nguoi choi */
#define MESSAGE_Y (TOP + 2 * BOARD_SIZE + 5)
#define MESSAGE_WIDTH 60

struct _POINT {
    int x, y, c;  /* x: dong, y: cot, c: 0=trong, -1=X, 1=O */
};

extern struct _POINT _A[BOARD_SIZE][BOARD_SIZE];
extern int _TURN;
extern int _COMMAND;
extern int _X, _Y;
extern int _WIN_P1, _WIN_P2, _MOVE_P1, _MOVE_P2;

#endif
