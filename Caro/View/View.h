/*
 * View - Ham xu ly giao dien man hinh
 */

#ifndef VIEW_H
#define VIEW_H

#include <graphics.h>

void InitSystem(void);
void CloseSystem(void);
void ReloadAvatars(void);

int UiScale(int designPx);
int ScreenW(void);
int ScreenH(void);
int GameFontPx(int designSize);
void ApplyGameFont(int height, int weight = FW_NORMAL);
void DrawPixelTextCentered(int x, int y, int w, int h, const TCHAR* text, COLORREF color, int designSize, bool shadow = true);
void DrawActionButton(int x, int y, int w, int h, const TCHAR* text, bool hovered, bool enabled = true);
bool IsGameFontLoaded(void);

void DrawBoard(void);
void DrawX(int row, int col);
void DrawO(int row, int col);
void DrawPlacementPreview(int row, int col);
bool BoardPointToCell(int mx, int my, int* outRow, int* outCol);
void DrawCursor(int row, int col);
void RenderGame(void);
void DrawPlayerInfo(void);
void DrawWinningLine(int winner);
void DrawWinBanner(int winner);
void ShowWinScreen(int winner);
void ReloadBoardAssets(void);
void GetUndoButtonRect(int* outX, int* outY, int* outW, int* outH);

#endif
