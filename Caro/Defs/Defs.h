/*
 * CaroGame - Dinh nghia hang so, kieu du lieu, bien toan cuc
 */

#ifndef DEFS_H
#define DEFS_H

#define BOARD_SIZE 12
#define CELL_SIZE  40  // Mỗi ô vuông rộng 40 pixel
#define OFFSET_X   50  // Cách lề trái màn hình 50 pixel
#define OFFSET_Y   50  // Cách lề trên màn hình 50 pixel

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

#endif
