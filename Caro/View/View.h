/*
 * View - Ham xu ly giao dien man hinh
 */

#ifndef VIEW_H
#define VIEW_H

#include <graphics.h> // Thư viện EasyX

void InitSystem(void);  // Mở cửa sổ game
void CloseSystem(void); // Đóng cửa sổ game
void DrawBoard(void);   // Vẽ lưới bàn cờ
void DrawX(int row, int col);      // Vẽ quân X
void DrawO(int row, int col);      // Vẽ quân O
void DrawCursor(int row, int col); // Vẽ khung báo hiệu đang chọn ô nào
void RenderGame(void);             // Hàm tổng hợp: Xóa màn hình -> Vẽ lưới -> Vẽ cờ -> Vẽ trỏ
void DrawPlayerInfo(void);
void DrawWinningLine(int winner);


#endif
