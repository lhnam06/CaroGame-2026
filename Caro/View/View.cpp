/*
 * View - Giao dien (bg image + board; optional pixel font & avatars)
 */

#include <iostream>
#include <cctype>
#include <windows.h>
#include "Defs/Defs.h"
#include "View/View.h"
#pragma comment(lib, "Msimg32.lib")

IMAGE imgBackground;
IMAGE imgX;
IMAGE imgO;
IMAGE imgAvatarP1;
IMAGE imgAvatarP2;

static HDC g_hdcScreen = NULL;
static HDC g_hdcImgX   = NULL;
static HDC g_hdcImgO   = NULL;
static HDC g_hdcAv1    = NULL;
static HDC g_hdcAv2    = NULL;

static bool g_hasAvatar1     = false;
static bool g_hasAvatar2     = false;
static bool g_pixelFontLoaded = false;
static bool g_hasPngX        = false;
static bool g_hasPngO        = false;

static const TCHAR* kPixelFontPath = _T("fonts\\PressStart2P-Regular.ttf");
static const int AVATAR_SIZE = 36;

/* ---------------------------------------------------------------
 * Per-theme board colors
 * 0 = Warm Wood  |  1 = Dark Slate  |  2 = Golden
 * --------------------------------------------------------------- */
struct BoardThemeColors {
    COLORREF frame;
    COLORREF outerFill;
    COLORREF cellA;
    COLORREF cellB;
    COLORREF grid;
};

static const BoardThemeColors kThemes[3] = {
    { RGB(92, 62, 38),   RGB(210, 198, 180), RGB(248, 244, 236), RGB(236, 228, 216), RGB(160, 148, 132) },
    { RGB(28, 32, 46),   RGB(44,  52,  74),  RGB(50,  58,  80),  RGB(38,  44,  62),  RGB(70,  78, 108)  },
    { RGB(120, 80, 20),  RGB(230, 210, 140), RGB(255, 240, 180), RGB(240, 220, 150), RGB(180, 155,  80)  },
};

/* ---------------------------------------------------------------
 * Fixed HUD colors
 * --------------------------------------------------------------- */
static const COLORREF UI_BAR_BG    = RGB(28,  32,  46);
static const COLORREF UI_BAR_EDGE  = RGB(70,  78, 108);
static const COLORREF UI_TEXT_DIM  = RGB(200, 206, 220);
static const COLORREF UI_ACCENT_P1 = RGB(255, 120, 108);
static const COLORREF UI_ACCENT_P2 = RGB(100, 170, 255);

/* ---------------------------------------------------------------
 * Helpers
 * --------------------------------------------------------------- */
static bool FileExists(const TCHAR* path) {
    return GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES;
}

static void TryLoadPixelFont(void) {
    if (!FileExists(kPixelFontPath)) return;
    if (AddFontResourceEx(kPixelFontPath, FR_PRIVATE, NULL))
        g_pixelFontLoaded = true;
}

static void UnloadPixelFont(void) {
    if (!g_pixelFontLoaded) return;
    RemoveFontResourceEx(kPixelFontPath, FR_PRIVATE, NULL);
    g_pixelFontLoaded = false;
}

static void ApplyUiFont(int height, int weight) {
    if (g_pixelFontLoaded)
        settextstyle(height, 0, _T("Press Start 2P"), 0, 0, weight, false, false, false);
    else
        settextstyle(height, 0, _T("Segoe UI"), 0, 0, weight, false, false, false);
}

static void DrawFallbackAvatar(int left, int top, COLORREF face, const TCHAR* label) {
    setfillcolor(face);
    fillroundrect(left, top, left + AVATAR_SIZE, top + AVATAR_SIZE, 8, 8);
    setlinecolor(RGB(20, 24, 36));
    setlinestyle(PS_SOLID, 2);
    roundrect(left, top, left + AVATAR_SIZE, top + AVATAR_SIZE, 8, 8);
    setbkmode(TRANSPARENT);
    ApplyUiFont(16, FW_BOLD);
    settextcolor(RGB(250, 250, 255));
    int tw = textwidth(label);
    int th = textheight(label);
    outtextxy(left + (AVATAR_SIZE - tw) / 2, top + (AVATAR_SIZE - th) / 2, label);
}

static void DrawAvatarOrFallback(int left, int top, IMAGE* img, HDC hdcSrc, bool hasImg, COLORREF fbColor, const TCHAR* fbLabel) {
    if (hasImg && hdcSrc) {
        TransparentBlt(g_hdcScreen, left, top, AVATAR_SIZE, AVATAR_SIZE,
                       hdcSrc, 0, 0, img->getwidth(), img->getheight(), RGB(0, 0, 0));
    } else {
        DrawFallbackAvatar(left, top, fbColor, fbLabel);
    }
}

/* ---------------------------------------------------------------
 * Board drawing (theme-aware)
 * --------------------------------------------------------------- */
static void DrawBoardShadowAndFrame(void) {
    int t  = (_BOARD_THEME >= 0 && _BOARD_THEME < 3) ? _BOARD_THEME : 0;
    const BoardThemeColors& tc = kThemes[t];

    int bx0 = OFFSET_X;
    int by0 = OFFSET_Y;
    int bx1 = OFFSET_X + BOARD_SIZE * CELL_SIZE;
    int by1 = OFFSET_Y + BOARD_SIZE * CELL_SIZE;

    // Drop shadow
    setfillcolor(RGB(8, 8, 14));
    fillroundrect(bx0 + 6, by0 + 6, bx1 + 6, by1 + 6, 14, 14);

    // Outer frame
    setlinecolor(tc.frame);
    setlinestyle(PS_SOLID, 3);
    setfillcolor(tc.outerFill);
    fillroundrect(bx0 - 6, by0 - 6, bx1 + 6, by1 + 6, 12, 12);

    // Cells (checkerboard)
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            int px = OFFSET_X + j * CELL_SIZE;
            int py = OFFSET_Y + i * CELL_SIZE;
            setfillcolor(((i + j) & 1) ? tc.cellA : tc.cellB);
            solidrectangle(px + 1, py + 1, px + CELL_SIZE - 1, py + CELL_SIZE - 1);
        }
    }

    // Grid lines
    setlinecolor(tc.grid);
    setlinestyle(PS_SOLID, 1);
    for (int i = 0; i <= BOARD_SIZE; i++) {
        line(OFFSET_X, OFFSET_Y + i * CELL_SIZE, OFFSET_X + BOARD_SIZE * CELL_SIZE, OFFSET_Y + i * CELL_SIZE);
        line(OFFSET_X + i * CELL_SIZE, OFFSET_Y, OFFSET_X + i * CELL_SIZE, OFFSET_Y + BOARD_SIZE * CELL_SIZE);
    }
}

/* ---------------------------------------------------------------
 * System init / close
 * --------------------------------------------------------------- */
void InitSystem(void) {
    initgraph(800, 600);
    TryLoadPixelFont();

    loadimage(&imgBackground, _T("UI_PNG\\bgm.jpg"), 800, 600);

    g_hasPngX = FileExists(_T("UI_PNG\\x_image.png"));
    g_hasPngO = FileExists(_T("UI_PNG\\o_image.png"));

    if (g_hasPngX) {
        loadimage(&imgX, _T("UI_PNG\\x_image.png"), CELL_SIZE - 4, CELL_SIZE - 4);
        g_hdcImgX = GetImageHDC(&imgX);
    }
    if (g_hasPngO) {
        loadimage(&imgO, _T("UI_PNG\\o_image.png"), CELL_SIZE - 4, CELL_SIZE - 4);
        g_hdcImgO = GetImageHDC(&imgO);
    }

    g_hdcScreen = GetImageHDC(NULL);

    if (FileExists(_T("UI_PNG\\avatar_p1.png"))) {
        loadimage(&imgAvatarP1, _T("UI_PNG\\avatar_p1.png"), AVATAR_SIZE, AVATAR_SIZE);
        g_hdcAv1 = GetImageHDC(&imgAvatarP1);
        g_hasAvatar1 = true;
    }
    if (FileExists(_T("UI_PNG\\avatar_p2.png"))) {
        loadimage(&imgAvatarP2, _T("UI_PNG\\avatar_p2.png"), AVATAR_SIZE, AVATAR_SIZE);
        g_hdcAv2 = GetImageHDC(&imgAvatarP2);
        g_hasAvatar2 = true;
    }
}

void CloseSystem(void) {
    UnloadPixelFont();
    closegraph();
}

void DrawBoard(void) {
    putimage(0, 0, &imgBackground);
    DrawBoardShadowAndFrame();
}

/* ---------------------------------------------------------------
 * Stone rendering — glossy circles with PNG overlay if available
 * --------------------------------------------------------------- */
static void DrawGlossyStone(int cx, int cy, int r, COLORREF baseColor, COLORREF midColor) {
    // Base filled circle
    setfillcolor(baseColor);
    setlinecolor(RGB(10, 10, 10));
    setlinestyle(PS_SOLID, 1);
    fillcircle(cx, cy, r);

    // Lighter mid ring (gives spherical depth)
    setfillcolor(midColor);
    fillcircle(cx - r / 5, cy - r / 5, r * 2 / 3);

    // Tiny specular highlight
    setfillcolor(RGB(255, 255, 255));
    fillcircle(cx - r / 3, cy - r / 3, r / 5);
}

void DrawX(int row, int col) {
    int cx = OFFSET_X + col * CELL_SIZE + CELL_SIZE / 2;
    int cy = OFFSET_Y + row * CELL_SIZE + CELL_SIZE / 2;
    int r  = CELL_SIZE / 2 - 4;

    // Glossy red circle base
    DrawGlossyStone(cx, cy, r, RGB(180, 40, 40), RGB(220, 80, 60));

    // Draw PNG sprite on top if available (transparent blit)
    if (g_hasPngX && g_hdcImgX) {
        int px = OFFSET_X + col * CELL_SIZE + 2;
        int py = OFFSET_Y + row * CELL_SIZE + 2;
        int w  = imgX.getwidth();
        int h  = imgX.getheight();
        TransparentBlt(g_hdcScreen, px, py, w, h, g_hdcImgX, 0, 0, w, h, RGB(0, 0, 0));
    }
}

void DrawO(int row, int col) {
    int cx = OFFSET_X + col * CELL_SIZE + CELL_SIZE / 2;
    int cy = OFFSET_Y + row * CELL_SIZE + CELL_SIZE / 2;
    int r  = CELL_SIZE / 2 - 4;

    // Glossy dark circle base
    DrawGlossyStone(cx, cy, r, RGB(28, 28, 28), RGB(70, 70, 70));

    // Draw PNG sprite on top if available (transparent blit)
    if (g_hasPngO && g_hdcImgO) {
        int px = OFFSET_X + col * CELL_SIZE + 2;
        int py = OFFSET_Y + row * CELL_SIZE + 2;
        int w  = imgO.getwidth();
        int h  = imgO.getheight();
        TransparentBlt(g_hdcScreen, px, py, w, h, g_hdcImgO, 0, 0, w, h, RGB(0, 0, 0));
    }
}

/* ---------------------------------------------------------------
 * Move-order number overlay (drawn after all stones)
 * --------------------------------------------------------------- */
static void DrawMoveNumbers(void) {
    setbkmode(TRANSPARENT);
    settextstyle(11, 0, _T("Segoe UI"), 0, 0, FW_BOLD, false, false, false);
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (_MOVE_ORDER[i][j] <= 0) continue;
            int cx = OFFSET_X + j * CELL_SIZE + CELL_SIZE / 2;
            int cy = OFFSET_Y + i * CELL_SIZE + CELL_SIZE / 2;
            TCHAR buf[8];
            _stprintf_s(buf, _T("%d"), _MOVE_ORDER[i][j]);
            int tw = textwidth(buf);
            int th = textheight(buf);
            // Contrast: white numbers on black stones, black on red
            settextcolor(_A[i][j].c == 1 ? RGB(220, 220, 220) : RGB(255, 255, 255));
            outtextxy(cx - tw / 2, cy - th / 2, buf);
        }
    }
}

/* ---------------------------------------------------------------
 * Cursor
 * --------------------------------------------------------------- */
void DrawCursor(int row, int col) {
    int x = OFFSET_X + col * CELL_SIZE;
    int y = OFFSET_Y + row * CELL_SIZE;

    setlinecolor(RGB(0, 60, 70));
    setlinestyle(PS_SOLID, 5);
    roundrect(x - 1, y - 1, x + CELL_SIZE + 1, y + CELL_SIZE + 1, 8, 8);

    setlinecolor(RGB(0, 220, 235));
    setlinestyle(PS_SOLID, 2);
    roundrect(x + 2, y + 2, x + CELL_SIZE - 2, y + CELL_SIZE - 2, 6, 6);
}

/* ---------------------------------------------------------------
 * HUD: player info bar + undo button
 * --------------------------------------------------------------- */
void DrawPlayerInfo(void) {
    const int barTop = 526;
    const int barBot = 592;
    const int avY    = barTop + 8;
    const int col1AvX = 20;
    const int col1Tx  = col1AvX + AVATAR_SIZE + 10;
    const int col2AvX = 268;
    const int col2Tx  = col2AvX + AVATAR_SIZE + 10;

    setfillcolor(UI_BAR_BG);
    fillroundrect(16, barTop, 784, barBot, 16, 16);
    setlinecolor(UI_BAR_EDGE);
    setlinestyle(PS_SOLID, 2);
    roundrect(16, barTop, 784, barBot, 16, 16);

    setbkmode(TRANSPARENT);

    if (_VS_BOT) {
        DrawAvatarOrFallback(col1AvX, avY, &imgAvatarP1, g_hdcAv1, g_hasAvatar1, UI_ACCENT_P1, _T("X"));
        DrawAvatarOrFallback(col2AvX, avY, &imgAvatarP2, g_hdcAv2, g_hasAvatar2, UI_ACCENT_P2, _T("O"));
    } else {
        DrawAvatarOrFallback(col1AvX, avY, &imgAvatarP1, g_hdcAv1, g_hasAvatar1, UI_ACCENT_P1, _T("1"));
        DrawAvatarOrFallback(col2AvX, avY, &imgAvatarP2, g_hdcAv2, g_hasAvatar2, UI_ACCENT_P2, _T("2"));
    }

    ApplyUiFont(g_pixelFontLoaded ? 11 : 16, FW_NORMAL);
    TCHAR p1Line[120], p2Line[120], turnInfo[100];
    if (_VS_BOT) {
        _stprintf_s(p1Line,   _T("You (X)  wins:%d"), _WIN_P1);
        _stprintf_s(p2Line,   _T("CPU (O)  wins:%d"), _WIN_P2);
        _stprintf_s(turnInfo, _TURN == 1 ? _T("Your turn") : _T("CPU turn"));
    } else {
        _stprintf_s(p1Line,   _T("Player 1 (X)  %d"), _WIN_P1);
        _stprintf_s(p2Line,   _T("Player 2 (O)  %d"), _WIN_P2);
        _stprintf_s(turnInfo, _TURN == 1 ? _T("P1 turn") : _T("P2 turn"));
    }

    settextcolor(UI_TEXT_DIM);
    outtextxy(col1Tx, barTop + 10, p1Line);
    outtextxy(col2Tx, barTop + 10, p2Line);

    // Turn pill
    ApplyUiFont(g_pixelFontLoaded ? 10 : 18, FW_BOLD);
    int tw       = textwidth(turnInfo);
    int pillLeft  = 540;
    int pillTop   = barTop + 22;
    int pillRight = pillLeft + tw + 20;
    int pillBot   = barTop + 50;

    COLORREF pillBg = (_TURN == 1) ? UI_ACCENT_P1 : UI_ACCENT_P2;
    setfillcolor(pillBg);
    fillroundrect(pillLeft - 4, pillTop - 4, pillRight + 4, pillBot + 4, 12, 12);
    settextcolor(RGB(18, 18, 24));
    outtextxy(pillLeft + 6, pillTop, turnInfo);

    // Undo button (640..700, barTop+10..barTop+50)
    const int undoX  = 646;
    const int undoY  = barTop + 8;
    const int undoW  = 54;
    const int undoH  = 46;
    bool canUndo = (_UNDO_TOP >= 1);
    COLORREF undoBg   = canUndo ? RGB(55, 62, 88) : RGB(36, 42, 58);
    COLORREF undoEdge = canUndo ? RGB(140, 155, 200) : RGB(60, 68, 90);
    setfillcolor(undoBg);
    fillroundrect(undoX, undoY, undoX + undoW, undoY + undoH, 10, 10);
    setlinecolor(undoEdge);
    setlinestyle(PS_SOLID, 1);
    roundrect(undoX, undoY, undoX + undoW, undoY + undoH, 10, 10);
    setbkmode(TRANSPARENT);
    settextstyle(14, 0, _T("Segoe UI"), 0, 0, FW_BOLD, false, false, false);
    settextcolor(canUndo ? RGB(220, 226, 255) : RGB(90, 96, 120));
    int utw = textwidth(_T("UNDO"));
    outtextxy(undoX + (undoW - utw) / 2, undoY + 14, _T("UNDO"));
}

/* ---------------------------------------------------------------
 * HUD: hamburger menu button
 * --------------------------------------------------------------- */
void DrawHudMenuButton(void) {
    const int menuX = 718;
    const int menuY = 18;
    const int s     = 52;

    setfillcolor(RGB(38, 44, 62));
    fillroundrect(menuX, menuY, menuX + s, menuY + s, 12, 12);
    setlinecolor(RGB(110, 125, 165));
    setlinestyle(PS_SOLID, 2);
    roundrect(menuX, menuY, menuX + s, menuY + s, 12, 12);

    setfillcolor(RGB(230, 233, 242));
    fillroundrect(menuX + 14, menuY + 16, menuX + 38, menuY + 20, 2, 2);
    fillroundrect(menuX + 14, menuY + 26, menuX + 38, menuY + 30, 2, 2);
    fillroundrect(menuX + 14, menuY + 36, menuX + 38, menuY + 40, 2, 2);
}

/* ---------------------------------------------------------------
 * Full game render
 * --------------------------------------------------------------- */
void RenderGame(void) {
    cleardevice();
    DrawBoard();

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (_A[i][j].c == -1)     DrawX(i, j);
            else if (_A[i][j].c == 1) DrawO(i, j);
        }
    }

    DrawMoveNumbers();
    DrawCursor(_X, _Y);
    DrawPlayerInfo();
    DrawHudMenuButton();
}

/* ---------------------------------------------------------------
 * Winning line — 3-layer glow effect
 * --------------------------------------------------------------- */
void DrawWinningLine(int winner) {
    (void)winner;

    int startX = OFFSET_X + _WIN_C1 * CELL_SIZE + CELL_SIZE / 2;
    int startY = OFFSET_Y + _WIN_R1 * CELL_SIZE + CELL_SIZE / 2;
    int endX   = OFFSET_X + _WIN_C2 * CELL_SIZE + CELL_SIZE / 2;
    int endY   = OFFSET_Y + _WIN_R2 * CELL_SIZE + CELL_SIZE / 2;

    // Layer 1: wide dark shadow
    setlinecolor(RGB(20, 10, 0));
    setlinestyle(PS_SOLID, 14);
    line(startX + 3, startY + 3, endX + 3, endY + 3);

    // Layer 2: golden core
    setlinecolor(RGB(255, 210, 60));
    setlinestyle(PS_SOLID, 7);
    line(startX, startY, endX, endY);

    // Layer 3: bright white shimmer (simulated 60% opacity via pale yellow)
    setlinecolor(RGB(255, 248, 210));
    setlinestyle(PS_SOLID, 2);
    line(startX, startY, endX, endY);
}
