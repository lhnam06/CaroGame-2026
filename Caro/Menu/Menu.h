/*
 * Menu - Man hinh menu chinh (yeu cau 4.5)
 */

#ifndef MENU_H
#define MENU_H


void DrawTextWithShadow(int x, int y, const TCHAR* text, COLORREF textColor, int fontSize);
// hovered=true lights up the button border on mouse-over
void DrawWoodenButton(int x, int y, int width, int height, const TCHAR* text, bool hovered = false);
void ShowInfoMenu(const TCHAR* title, const TCHAR* line1, const TCHAR* line2, const TCHAR* line3);
bool ShowLoadMenuUI();
int ShowNewGameModeMenu(void);
int ShowMainMenu(void);
void ShowSettingsMenu(void);
int ShowPauseMenu(void);
bool ShowConfirmDialog(const TCHAR* title, const TCHAR* message);
void ShowNotifyDialog(const TCHAR* title, const TCHAR* message);


#endif
