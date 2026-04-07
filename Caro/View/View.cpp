/*
 * View - Giao dien man hinh
 */

#include <iostream>
#include <cctype>
#include "Defs/Defs.h"
#include "View/View.h"
#pragma comment(lib, "Msimg32.lib")
IMAGE imgBackground; 
IMAGE imgX;         
IMAGE imgO;

void InitSystem(void) {
    initgraph(800, 600);
    loadimage(&imgBackground, _T("UI_PNG\\bgm.jpg"), 800, 600); 
    loadimage(&imgX, _T("UI_PNG\\x_image.png"), CELL_SIZE - 4, CELL_SIZE - 4);
    loadimage(&imgO, _T("UI_PNG\\o_image.png"), CELL_SIZE - 4, CELL_SIZE - 4);
}

void CloseSystem(void) {
    closegraph();
}

void DrawBoard(void) {
    putimage(0, 0, &imgBackground); 
    setlinecolor(BLACK); 
    setlinestyle(PS_SOLID, 2);

    for (int i = 0; i <= BOARD_SIZE; i++) {
        line(OFFSET_X, OFFSET_Y + i * CELL_SIZE, 
             OFFSET_X + BOARD_SIZE * CELL_SIZE, OFFSET_Y + i * CELL_SIZE);
        line(OFFSET_X + i * CELL_SIZE, OFFSET_Y, 
             OFFSET_X + i * CELL_SIZE, OFFSET_Y + BOARD_SIZE * CELL_SIZE);
    }
}

void DrawX(int row, int col) {
    int pixelX = OFFSET_X + col * CELL_SIZE + 2;
    int pixelY = OFFSET_Y + row * CELL_SIZE + 2;
    int w = imgX.getwidth();
    int h = imgX.getheight();
    TransparentBlt(
        GetImageHDC(NULL), pixelX, pixelY, w, h, 
        GetImageHDC(&imgX), 0, 0, w, h,          
        RGB(0, 0, 0)                             
    );
}

void DrawO(int row, int col) {
    int pixelX = OFFSET_X + col * CELL_SIZE + 2;
    int pixelY = OFFSET_Y + row * CELL_SIZE + 2;
    int w = imgO.getwidth();
    int h = imgO.getheight();
    TransparentBlt(
        GetImageHDC(NULL), pixelX, pixelY, w, h, 
        GetImageHDC(&imgO), 0, 0, w, h,          
        RGB(0, 0, 0)                            
    );
}

void DrawCursor(int row, int col) {
    setlinecolor(RGB(0, 200, 0));  
    setlinestyle(PS_SOLID, 3);     
    
    int x = OFFSET_X + col * CELL_SIZE;
    int y = OFFSET_Y + row * CELL_SIZE;
    rectangle(x, y, x + CELL_SIZE, y + CELL_SIZE);
}

void DrawPlayerInfo(void) {
    settextcolor(BLACK);
    settextstyle(20, 0, _T("Consolas")); 

    TCHAR info1[100], info2[100], turnInfo[100];
    _stprintf_s(info1, _T("Player 1 (X): %d win games"), _WIN_P1);
    _stprintf_s(info2, _T("Player 2 (O): %d win games"), _WIN_P2);

    if (_TURN == 1) _stprintf_s(turnInfo, _T("Current Turn: Player 1 (X)"));
    else _stprintf_s(turnInfo, _T("Current Turn: Player 2 (O)"));

    int textY = OFFSET_Y + BOARD_SIZE * CELL_SIZE + 20;
    
    outtextxy(OFFSET_X, textY, info1);
    outtextxy(OFFSET_X, textY + 30, info2);

    if (_TURN == 1) settextcolor(RED);
    else settextcolor(BLUE);
    outtextxy(OFFSET_X + 250, textY + 15, turnInfo);
}

void RenderGame(void) {
    cleardevice(); 
    DrawBoard();   
    
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (_A[i][j].c == -1) DrawX(i, j);
            else if (_A[i][j].c == 1) DrawO(i, j);
        }
    }
    
    DrawCursor(_X, _Y);
    DrawPlayerInfo();
    // Vẽ biểu tượng Hamburger Menu (3 dấu gạch ngang) ở góc phải
    int menuX = 720, menuY = 20;
    setlinecolor(BLACK);
    setlinestyle(PS_SOLID, 2);
    setfillcolor(RGB(50, 50, 50)); // Nền hộp vuông xám đen
    fillroundrect(menuX, menuY, menuX + 50, menuY + 50, 10, 10);
    
    setfillcolor(WHITE); // Vẽ 3 vạch trắng
    fillrectangle(menuX + 10, menuY + 12, menuX + 40, menuY + 16);
    fillrectangle(menuX + 10, menuY + 23, menuX + 40, menuY + 27);
    fillrectangle(menuX + 10, menuY + 34, menuX + 40, menuY + 38);
}

void DrawWinningLine(int winner) {
    setlinecolor(RGB(255, 215, 0)); // Màu vàng gold
    setlinestyle(PS_SOLID, 8);      // Độ dày nét vẽ = 8
    
    int startX = OFFSET_X + _WIN_C1 * CELL_SIZE + CELL_SIZE / 2;
    int startY = OFFSET_Y + _WIN_R1 * CELL_SIZE + CELL_SIZE / 2;
    int endX   = OFFSET_X + _WIN_C2 * CELL_SIZE + CELL_SIZE / 2;
    int endY   = OFFSET_Y + _WIN_R2 * CELL_SIZE + CELL_SIZE / 2;
    
    line(startX, startY, endX, endY); // Chỉ vẽ 1 đường chém ngang là đủ ngầu rồi!
}