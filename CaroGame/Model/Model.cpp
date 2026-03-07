/*
 * Model - Du lieu va logic ban co
 */

#include "Defs/Defs.h"
#include "Model/Model.h"

void ResetData(void) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            _A[i][j].x = 4 * j + LEFT + 2;
            _A[i][j].y = 2 * i + TOP + 1;
            _A[i][j].c = 0;
        }
    }
    _TURN = 1;
    _COMMAND = -1;
    _X = _A[0][0].x;
    _Y = _A[0][0].y;
    _MOVE_P1 = 0;
    _MOVE_P2 = 0;
}

void GabageCollect(void) {
}

int TestBoard(void) {
    int lastPlayer = _TURN ? -1 : 1;

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (_A[i][j].c == 0) goto not_full;
        }
    }
    return 0;  /* Hoa - ban day */
not_full:

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (_A[i][j].c != lastPlayer) continue;
            int count = 1;
            while (j + count < BOARD_SIZE && _A[i][j + count].c == lastPlayer) count++;
            if (count >= 5) return lastPlayer;
            count = 1;
            while (i + count < BOARD_SIZE && _A[i + count][j].c == lastPlayer) count++;
            if (count >= 5) return lastPlayer;
            count = 1;
            while (i + count < BOARD_SIZE && j + count < BOARD_SIZE && _A[i + count][j + count].c == lastPlayer) count++;
            if (count >= 5) return lastPlayer;
            count = 1;
            while (i - count >= 0 && j + count < BOARD_SIZE && _A[i - count][j + count].c == lastPlayer) count++;
            if (count >= 5) return lastPlayer;
        }
    }
    return 2;
}

int CheckBoard(int pX, int pY) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (_A[i][j].x == pX && _A[i][j].y == pY && _A[i][j].c == 0) {
                if (_TURN == 1) {
                    _A[i][j].c = -1;
                    _MOVE_P1++;
                } else {
                    _A[i][j].c = 1;
                    _MOVE_P2++;
                }
                return _A[i][j].c;
            }
        }
    }
    return 0;
}
