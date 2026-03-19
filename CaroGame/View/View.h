/*
 * View - Ham xu ly giao dien man hinh
 */

#ifndef VIEW_H
#define VIEW_H

void FixConsoleWindow(void);
void GotoXY(int x, int y);
void DrawBoard(int pSize);
void DrawPlayerInfo(void);
void ClearMessageArea(void);
void ShowMessage(const char* msg);
void DrawWinEffect(int pWhoWin);
int ProcessFinish(int pWhoWin);
int AskContinue(void);

#endif
