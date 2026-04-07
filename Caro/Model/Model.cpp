/*
 * Model - Du lieu va logic ban co
 */

#include "Defs/Defs.h"
#include "Model/Model.h"

void ResetData(void) {
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            _A[i][j].x = i; 
            _A[i][j].y = j; 
            _A[i][j].c = 0; 
        }
    }
    _TURN = 1;
    _COMMAND = -1;
    _X = 0; 
    _Y = 0; 
    _MOVE_P1 = 0;
    _MOVE_P2 = 0;
}

void GabageCollect(void) {
}

int TestBoard(void) {
    int lastPlayer = _TURN ? -1 : 1; 

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (_A[i][j].c != lastPlayer) continue;

            // Kiểm tra Ngang
            int count = 1;
            while (j + count < BOARD_SIZE && _A[i][j + count].c == lastPlayer) count++;
            if (count >= 5) {
                _WIN_R1 = i; _WIN_C1 = j; _WIN_R2 = i; _WIN_C2 = j + count - 1;
                return lastPlayer;
            }

            // Kiểm tra Dọc
            count = 1;
            while (i + count < BOARD_SIZE && _A[i + count][j].c == lastPlayer) count++;
            if (count >= 5) {
                _WIN_R1 = i; _WIN_C1 = j; _WIN_R2 = i + count - 1; _WIN_C2 = j;
                return lastPlayer;
            }

            // Kiểm tra Chéo chính (\)
            count = 1;
            while (i + count < BOARD_SIZE && j + count < BOARD_SIZE && _A[i + count][j + count].c == lastPlayer) count++;
            if (count >= 5) {
                _WIN_R1 = i; _WIN_C1 = j; _WIN_R2 = i + count - 1; _WIN_C2 = j + count - 1;
                return lastPlayer;
            }

            // Kiểm tra Chéo phụ (/)
            count = 1;
            while (i - count >= 0 && j + count < BOARD_SIZE && _A[i - count][j + count].c == lastPlayer) count++;
            if (count >= 5) {
                _WIN_R1 = i; _WIN_C1 = j; _WIN_R2 = i - count + 1; _WIN_C2 = j + count - 1;
                return lastPlayer;
            }
        }
    }

    // Kiểm tra hòa (đầy bàn)
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (_A[i][j].c == 0) return 2; // Còn ô trống -> Chưa xong
        }
    }
    return 0; // Hòa
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

