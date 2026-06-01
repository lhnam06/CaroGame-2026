/*
 * View - Giao dien (bg image + board; optional pixel font & avatars)
 */

#include <iostream>
#include <cctype>
#include <vector>
#include <windows.h>
#include "Defs/Defs.h"
#include "View/View.h"
#include "Audio/Audio.h"
#pragma comment(lib, "Msimg32.lib")

IMAGE imgBackground;
IMAGE imgX;
IMAGE imgO;
IMAGE imgAvatarP1;
IMAGE imgAvatarP2;
IMAGE imgWinnerFrame;

static HDC g_hdcScreen = NULL;
static HDC g_hdcAv1    = NULL;
static HDC g_hdcAv2    = NULL;

static bool g_hasAvatar1     = false;
static bool g_hasAvatar2     = false;
static bool g_hasWinnerFrame = false;
static int  g_infoPanelW     = 0;
static int  g_infoPanelH     = 0;
static int  g_infoPanelX     = 0;
static int  g_infoPanel1Y    = 0;
static int  g_infoPanel2Y    = 0;
static int  g_sidebarX       = 0;
static int  g_sidebarW       = 0;
static int  g_matchBarY      = 0;
static int  g_matchBarH      = 0;
static int  g_undoY          = 0;
static bool g_hasPngX        = false;
static bool g_hasPngO        = false;
static bool g_pixelFontLoaded = false;

static IMAGE imgCellLight[3];
static IMAGE imgCellDark[3];
static IMAGE imgBoardFrame[3];
static bool  g_hasBoardTiles[3] = {};
static const int kFramePadDesign = 18;

static const TCHAR* kBoardThemeFile[3] = { _T("wood"), _T("slate"), _T("gold") };

// User X/O art — replace these PNGs in GUI/ to customize pieces.
static const TCHAR* kStoneXPathCandidates[] = {
    _T("GUI\\hieuUngXWin.png"),
    NULL
};
static const TCHAR* kStoneOPathCandidates[] = {
    _T("GUI\\hieuUngOWin.png"),
    NULL
};

static int SidebarWidth(void) {
    return g_sidebarW + UiScale(28);
}

static bool FileExists(const TCHAR* path);

static const TCHAR* kPixelFontPath = _T("fonts\\PressStart2P-Regular.ttf");

static int AvatarSize(void) { return UiScale(36); }

int UiScale(int designPx) {
    return (designPx * UI_SCALE);
}

int ScreenW(void) { return SCREEN_W; }
int ScreenH(void) { return SCREEN_H; }

bool IsGameFontLoaded(void) { return g_pixelFontLoaded; }

int GameFontPx(int designSize) {
    int px = designSize * UI_SCALE;
    if (px < 10) px = 10;
    return px;
}

void ApplyGameFont(int height, int weight) {
    int px = GameFontPx(height);
    if (g_pixelFontLoaded)
        settextstyle(px, 0, _T("Press Start 2P"), 0, 0, weight, false, false, false);
    else
        settextstyle(px, 0, _T("Consolas"), 0, 0, weight, false, false, false);
}

void DrawPixelTextCentered(int x, int y, int w, int h, const TCHAR* text, COLORREF color, int designSize, bool shadow) {
    setbkmode(TRANSPARENT);
    ApplyGameFont(designSize, FW_BOLD);
    int tw = textwidth(text);
    int th = textheight(text);
    int tx = x + (w - tw) / 2;
    int ty = y + (h - th) / 2;
    if (shadow) {
        settextcolor(RGB(18, 20, 32));
        outtextxy(tx + UiScale(1), ty + UiScale(1), text);
    }
    settextcolor(color);
    outtextxy(tx, ty, text);
}

static int ButtonLabelFontSize(int btnHeightPx) {
    int design = btnHeightPx * 16 / UiScale(52);
    if (design < 14) design = 14;
    if (design > 20) design = 20;
    return design;
}

void DrawActionButton(int x, int y, int w, int h, const TCHAR* text, bool hovered, bool enabled) {
    const int r = UiScale(10);

    if (enabled) {
        setfillcolor(RGB(10, 12, 20));
        fillroundrect(x + UiScale(2), y + UiScale(3), x + w + UiScale(2), y + h + UiScale(3), r, r);
    }

    COLORREF fill, border, textColor;
    if (!enabled) {
        fill      = RGB(34, 38, 54);
        border    = RGB(58, 64, 84);
        textColor = RGB(108, 114, 136);
    } else if (hovered) {
        fill      = RGB(78, 88, 124);
        border    = RGB(255, 232, 130);
        textColor = RGB(255, 255, 255);
    } else {
        fill      = RGB(58, 66, 94);
        border    = RGB(255, 210, 80);
        textColor = RGB(250, 246, 238);
    }

    setfillcolor(fill);
    fillroundrect(x, y, x + w, y + h, r, r);

    setlinecolor(border);
    setlinestyle(PS_SOLID, UiScale(hovered ? 3 : 2));
    roundrect(x, y, x + w, y + h, r, r);

    if (enabled) {
        setfillcolor(hovered ? RGB(255, 250, 230) : RGB(255, 236, 170));
        solidroundrect(x + UiScale(8), y + UiScale(5), x + w - UiScale(8), y + UiScale(9),
                       UiScale(4), UiScale(4));
    }

    DrawPixelTextCentered(x, y, w, h, text, textColor, ButtonLabelFontSize(h), true);
}

static void SetupScreenLayout(void) {
    RECT work = { 0, 0, 0, 0 };
    SystemParametersInfo(SPI_GETWORKAREA, 0, &work, 0);
    int availW = work.right - work.left;
    int availH = work.bottom - work.top;

    int scaleW = availW / DESIGN_W;
    int scaleH = availH / DESIGN_H;
    int scale = scaleW < scaleH ? scaleW : scaleH;
    if (scale < 1) scale = 1;

    UI_SCALE   = scale;
    SCREEN_W   = DESIGN_W * scale;
    SCREEN_H   = DESIGN_H * scale;

    g_sidebarW = UiScale(272);
    const int marginL = UiScale(20);
    const int gapBoardSidebar = UiScale(22);
    const int marginR = UiScale(16);
    int availForBoard = SCREEN_W - g_sidebarW - marginL - gapBoardSidebar - marginR;
    int cell = availForBoard / BOARD_SIZE;
    if (cell < UiScale(36)) cell = UiScale(36);
    if (cell > UiScale(56)) cell = UiScale(56);
    CELL_SIZE = cell;

    int boardW = BOARD_SIZE * CELL_SIZE;
    int boardH = BOARD_SIZE * CELL_SIZE;
    OFFSET_X = marginL;
    OFFSET_Y = (SCREEN_H - boardH) / 2;
    if (OFFSET_Y < UiScale(36)) OFFSET_Y = UiScale(36);

    g_sidebarX = OFFSET_X + boardW + gapBoardSidebar;

    g_infoPanelW  = g_sidebarW;
    g_matchBarY   = UiScale(72);
    g_matchBarH   = UiScale(96);
    g_infoPanelH  = UiScale(118);
    g_infoPanelX  = g_sidebarX;
    g_infoPanel1Y = g_matchBarY + g_matchBarH + UiScale(10);
    g_infoPanel2Y = g_infoPanel1Y + g_infoPanelH + UiScale(8);
    g_undoY       = g_infoPanel2Y + g_infoPanelH + UiScale(10);
}

static void CenterGameWindow(void) {
    HWND hwnd = GetHWnd();
    if (!hwnd) return;
    int x = (GetSystemMetrics(SM_CXSCREEN) - SCREEN_W) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - SCREEN_H) / 2;
    if (x < 0) x = 0;
    if (y < 0) y = 0;
    SetWindowPos(hwnd, HWND_TOP, x, y, SCREEN_W, SCREEN_H, SWP_SHOWWINDOW);
}

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
    ApplyGameFont(height, weight);
}

static void DrawPixelRect(int x, int y, int w, int h, COLORREF fill, COLORREF border, int borderPx) {
    setfillcolor(fill);
    solidrectangle(x, y, x + w, y + h);
    setlinecolor(border);
    setlinestyle(PS_SOLID, borderPx);
    rectangle(x, y, x + w, y + h);
}

static void ApplyOverlayFont(int designSize) {
    LOGFONT lf = { 0 };
    lf.lfHeight = -GameFontPx(designSize);
    lf.lfWeight = FW_BOLD;
    _tcscpy_s(lf.lfFaceName, _T("Segoe UI"));
    settextstyle(&lf);
}

static bool ColorsClose(DWORD px, COLORREF key, int tol) {
    int r = (int)((px >> 16) & 0xFF);
    int g = (int)((px >> 8) & 0xFF);
    int b = (int)(px & 0xFF);
    return abs(r - (int)GetRValue(key)) <= tol &&
           abs(g - (int)GetGValue(key)) <= tol &&
           abs(b - (int)GetBValue(key)) <= tol;
}

static void MakeEdgeBackgroundTransparent(IMAGE* img) {
    int w = img->getwidth();
    int h = img->getheight();
    DWORD* buf = (DWORD*)GetImageBuffer(img);
    if (!buf || w < 2 || h < 2) return;

    COLORREF key = RGB((buf[0] >> 16) & 0xFF, (buf[0] >> 8) & 0xFF, buf[0] & 0xFF);
    const int tol = 12;
    std::vector<unsigned char> vis((size_t)w * h, 0);
    std::vector<int> stack;

    auto tryPush = [&](int x, int y) {
        if (x < 0 || x >= w || y < 0 || y >= h) return;
        int i = y * w + x;
        if (vis[i]) return;
        if (!ColorsClose(buf[i], key, tol)) return;
        vis[i] = 1;
        stack.push_back(i);
    };

    for (int x = 0; x < w; x++) {
        tryPush(x, 0);
        tryPush(x, h - 1);
    }
    for (int y = 0; y < h; y++) {
        tryPush(0, y);
        tryPush(w - 1, y);
    }

    while (!stack.empty()) {
        int i = stack.back();
        stack.pop_back();
        buf[i] &= 0x00FFFFFF;
        int x = i % w;
        int y = i / w;
        tryPush(x - 1, y);
        tryPush(x + 1, y);
        tryPush(x, y - 1);
        tryPush(x, y + 1);
    }
}

static void DrawSpriteScaled(int dx, int dy, int dw, int dh, IMAGE* img) {
    if (!img || img->getwidth() <= 0 || img->getheight() <= 0) return;
    HDC hdcSrc = GetImageHDC(img);
    BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
    AlphaBlend(g_hdcScreen, dx, dy, dw, dh, hdcSrc, 0, 0, img->getwidth(), img->getheight(), bf);
}

static void DrawSpriteScaledAlpha(int dx, int dy, int dw, int dh, IMAGE* img, BYTE alpha) {
    if (!img || img->getwidth() <= 0 || img->getheight() <= 0) return;
    HDC hdcSrc = GetImageHDC(img);
    BLENDFUNCTION bf = { AC_SRC_OVER, 0, alpha, AC_SRC_ALPHA };
    AlphaBlend(g_hdcScreen, dx, dy, dw, dh, hdcSrc, 0, 0, img->getwidth(), img->getheight(), bf);
}

static bool ImageHasPerPixelAlpha(IMAGE* img) {
    DWORD* buf = (DWORD*)GetImageBuffer(img);
    if (!buf) return false;

    int w = img->getwidth();
    int h = img->getheight();
    if (w <= 0 || h <= 0) return false;

    auto alphaAt = [&](int x, int y) -> BYTE {
        return (BYTE)((buf[y * w + x] >> 24) & 0xFF);
    };

    if (alphaAt(0, 0) == 0) return true;
    if (alphaAt(w - 1, 0) == 0) return true;
    if (alphaAt(0, h - 1) == 0) return true;
    if (alphaAt(w - 1, h - 1) == 0) return true;

    int n = w * h;
    for (int i = 0; i < n; i++) {
        BYTE a = (BYTE)((buf[i] >> 24) & 0xFF);
        if (a > 0 && a < 255) return true;
    }
    return false;
}

static void PrepareSpriteImage(IMAGE* img) {
    if (!ImageHasPerPixelAlpha(img))
        MakeEdgeBackgroundTransparent(img);
}

static bool LoadStoneImage(const TCHAR* const* candidates, IMAGE* img, bool* outHas) {
    *outHas = false;
    if (!candidates) return false;

    for (int i = 0; candidates[i] != NULL; i++) {
        if (!FileExists(candidates[i])) continue;

        loadimage(img, candidates[i]);
        if (img->getwidth() <= 0 || img->getheight() <= 0) continue;

        PrepareSpriteImage(img);
        *outHas = true;
        return true;
    }
    return false;
}

static int StoneDrawSize(void) {
    return CELL_SIZE - UiScale(4);
}

static void DrawStatRow(int x, int y, int w, const TCHAR* label, const TCHAR* value, int fontDesign) {
    ApplyOverlayFont(fontDesign - 1);
    setbkmode(TRANSPARENT);
    settextcolor(RGB(160, 168, 190));
    outtextxy(x, y, label);

    ApplyOverlayFont(fontDesign);
    int tw = textwidth(value);
    settextcolor(RGB(248, 244, 236));
    outtextxy(x + w - tw, y, value);
}

static void DrawPlayerInfoBox(int x, int y, int w, int h,
                              COLORREF accent, const TCHAR* title, const TCHAR* stone,
                              IMAGE* avatar, bool hasAv,
                              const TCHAR* name, int moves, int wins, bool activeTurn) {
    const int r       = UiScale(10);
    const int pad     = UiScale(10);
    const int headerH = UiScale(34);

    setfillcolor(RGB(8, 10, 18));
    fillroundrect(x + UiScale(2), y + UiScale(2), x + w + UiScale(2), y + h + UiScale(2), r, r);

    setfillcolor(RGB(32, 38, 56));
    fillroundrect(x, y, x + w, y + h, r, r);

    setfillcolor(RGB(44, 50, 72));
    fillroundrect(x, y, x + w, y + headerH, r, r);
    solidrectangle(x, y + headerH - r, x + w, y + headerH);

    setfillcolor(accent);
    solidrectangle(x, y + UiScale(5), x + UiScale(4), y + h - UiScale(5));

    int chipR = UiScale(12);
    int chipX = x + pad + chipR;
    int chipY = y + headerH / 2;
    setfillcolor(accent);
    fillcircle(chipX, chipY, chipR);
    ApplyOverlayFont(11);
    setbkmode(TRANSPARENT);
    settextcolor(RGB(255, 255, 255));
    int stw = textwidth(stone);
    int sth = textheight(stone);
    outtextxy(chipX - stw / 2, chipY - sth / 2, stone);

    ApplyOverlayFont(12);
    settextcolor(RGB(255, 248, 236));
    outtextxy(x + pad + chipR * 2 + UiScale(8), y + (headerH - textheight(title)) / 2, title);

    if (activeTurn) {
        const TCHAR* badge = _T("TURN");
        ApplyOverlayFont(9);
        int btw = textwidth(badge);
        int bth = textheight(badge);
        int bw  = btw + UiScale(12);
        int bh  = bth + UiScale(6);
        int bx  = x + w - bw - pad;
        int by  = y + (headerH - bh) / 2;
        setfillcolor(RGB(255, 210, 60));
        fillroundrect(bx, by, bx + bw, by + bh, UiScale(5), UiScale(5));
        settextcolor(RGB(28, 24, 16));
        outtextxy(bx + (bw - btw) / 2, by + (bh - bth) / 2, badge);
    }

    setlinecolor(activeTurn ? RGB(255, 210, 60) : RGB(255, 210, 80));
    setlinestyle(PS_SOLID, activeTurn ? UiScale(2) : UiScale(2));
    roundrect(x, y, x + w, y + h, r, r);

    int avColW = w * 30 / 100;
    int statsW = w - avColW - pad * 2 - UiScale(6);
    int bodyTop = y + headerH + UiScale(8);
    int bodyH   = h - headerH - UiScale(14);
    int rowH    = bodyH / 3;
    int fontDesign = 11;

    DrawStatRow(x + pad + UiScale(6), bodyTop, statsW, _T("Name"), name, fontDesign);
    TCHAR buf[32];
    _stprintf_s(buf, _T("%d"), moves);
    DrawStatRow(x + pad + UiScale(6), bodyTop + rowH, statsW, _T("Moves"), buf, fontDesign);
    _stprintf_s(buf, _T("%d"), wins);
    DrawStatRow(x + pad + UiScale(6), bodyTop + rowH * 2, statsW, _T("Wins"), buf, fontDesign);

    int avSz = bodyH - UiScale(2);
    if (avSz > avColW - UiScale(6)) avSz = avColW - UiScale(6);
    if (avSz > UiScale(64)) avSz = UiScale(64);
    int avX = x + w - avColW + (avColW - avSz) / 2;
    int avY = bodyTop + (bodyH - avSz) / 2;

    setlinecolor(RGB(255, 210, 80));
    setlinestyle(PS_SOLID, UiScale(2));
    roundrect(avX - UiScale(3), avY - UiScale(3), avX + avSz + UiScale(3), avY + avSz + UiScale(3),
              UiScale(6), UiScale(6));
    setfillcolor(RGB(24, 28, 42));
    fillroundrect(avX - UiScale(1), avY - UiScale(1), avX + avSz + UiScale(1), avY + avSz + UiScale(1),
                  UiScale(4), UiScale(4));

    if (hasAv && avatar) {
        DrawSpriteScaled(avX, avY, avSz, avSz, avatar);
    }
}

static void DrawMatchInfo(void) {
    const int x = g_sidebarX;
    const int y = g_matchBarY;
    const int w = g_sidebarW;
    const int h = g_matchBarH;
    const int r = UiScale(12);

    setfillcolor(RGB(8, 10, 18));
    fillroundrect(x + UiScale(2), y + UiScale(3), x + w + UiScale(2), y + h + UiScale(3), r, r);

    setfillcolor(RGB(32, 38, 56));
    fillroundrect(x, y, x + w, y + h, r, r);

    setfillcolor(RGB(44, 50, 72));
    fillroundrect(x, y, x + w, y + UiScale(36), r, r);
    solidrectangle(x, y + UiScale(36) - r, x + w, y + UiScale(36));

    ApplyOverlayFont(12);
    setbkmode(TRANSPARENT);
    settextcolor(RGB(255, 220, 140));
    const TCHAR* matchTitle = _T("MATCH INFO");
    int tw = textwidth(matchTitle);
    outtextxy(x + (w - tw) / 2, y + UiScale(8), matchTitle);

    setlinecolor(RGB(255, 210, 80));
    setlinestyle(PS_SOLID, UiScale(2));
    roundrect(x, y, x + w, y + h, r, r);

    TCHAR turnLine[64], moveLine[64], scoreLine[64];
    if (_VS_BOT) {
        if (_TURN == 1)
            _stprintf_s(turnLine, _T("%s's turn  (X)"), _NAME_P1);
        else
            _stprintf_s(turnLine, _T("%s's turn  (O)"), _NAME_P2);
        _stprintf_s(scoreLine, _T("Session  %s %d  -  %s %d"), _NAME_P1, _WIN_P1, _NAME_P2, _WIN_P2);
    } else {
        if (_TURN == 1)
            _stprintf_s(turnLine, _T("%s's turn  (X)"), _NAME_P1);
        else
            _stprintf_s(turnLine, _T("%s's turn  (O)"), _NAME_P2);
        _stprintf_s(scoreLine, _T("Session  %s %d  -  %s %d"), _NAME_P1, _WIN_P1, _NAME_P2, _WIN_P2);
    }
    _stprintf_s(moveLine, _T("Moves this game: %d"), _MOVE_COUNT);

    int cx = x + w / 2;
    int y1 = y + UiScale(38);
    int y2 = y + h * 58 / 100;
    int y3 = y + h * 78 / 100;

    COLORREF turnColor = (_TURN == 1) ? UI_ACCENT_P1 : UI_ACCENT_P2;
    ApplyOverlayFont(12);
    settextcolor(turnColor);
    tw = textwidth(turnLine);
    outtextxy(cx - tw / 2, y1, turnLine);

    ApplyOverlayFont(11);
    settextcolor(UI_TEXT_DIM);
    tw = textwidth(moveLine);
    outtextxy(cx - tw / 2, y2, moveLine);
    tw = textwidth(scoreLine);
    outtextxy(cx - tw / 2, y3, scoreLine);
}

/* ---------------------------------------------------------------
 * Board drawing (theme-aware tiles + frame PNG)
 * --------------------------------------------------------------- */
static void LoadBoardAssets(void) {
    int framePad = UiScale(kFramePadDesign);
    int frameSz  = BOARD_SIZE * CELL_SIZE + framePad * 2;

    for (int i = 0; i < 3; i++) {
        g_hasBoardTiles[i] = false;
        TCHAR pathLight[128], pathDark[128], pathFrame[128];
        _stprintf_s(pathLight, 128, _T("GUI\\Board\\cell_light_%s.png"), kBoardThemeFile[i]);
        _stprintf_s(pathDark,  128, _T("GUI\\Board\\cell_dark_%s.png"),  kBoardThemeFile[i]);
        _stprintf_s(pathFrame, 128, _T("GUI\\Board\\board_frame_%s.png"), kBoardThemeFile[i]);
        if (FileExists(pathLight) && FileExists(pathDark) && FileExists(pathFrame)) {
            loadimage(&imgCellLight[i], pathLight, CELL_SIZE, CELL_SIZE);
            loadimage(&imgCellDark[i],  pathDark,  CELL_SIZE, CELL_SIZE);
            loadimage(&imgBoardFrame[i], pathFrame, frameSz, frameSz);
            g_hasBoardTiles[i] = true;
        }
    }
}

void ReloadBoardAssets(void) {
    LoadBoardAssets();
}

static void DrawBoardShadowAndFrame(void) {
    int t  = (_BOARD_THEME >= 0 && _BOARD_THEME < 3) ? _BOARD_THEME : 0;
    const BoardThemeColors& tc = kThemes[t];

    int bx0 = OFFSET_X;
    int by0 = OFFSET_Y;
    int bx1 = OFFSET_X + BOARD_SIZE * CELL_SIZE;
    int by1 = OFFSET_Y + BOARD_SIZE * CELL_SIZE;
    int framePad = UiScale(kFramePadDesign);

    setfillcolor(RGB(8, 8, 14));
    solidrectangle(bx0 + UiScale(4), by0 + UiScale(4), bx1 + UiScale(4), by1 + UiScale(4));

    if (g_hasBoardTiles[t]) {
        putimage(bx0 - framePad, by0 - framePad, &imgBoardFrame[t]);

        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                int px = OFFSET_X + j * CELL_SIZE;
                int py = OFFSET_Y + i * CELL_SIZE;
                IMAGE* tile = ((i + j) & 1) ? &imgCellDark[t] : &imgCellLight[t];
                putimage(px, py, tile);
            }
        }
    } else {
        setlinecolor(tc.frame);
        setlinestyle(PS_SOLID, UiScale(3));
        setfillcolor(tc.outerFill);
        solidrectangle(bx0 - UiScale(6), by0 - UiScale(6), bx1 + UiScale(6), by1 + UiScale(6));
        rectangle(bx0 - UiScale(6), by0 - UiScale(6), bx1 + UiScale(6), by1 + UiScale(6));

        for (int i = 0; i < BOARD_SIZE; i++) {
            for (int j = 0; j < BOARD_SIZE; j++) {
                int px = OFFSET_X + j * CELL_SIZE;
                int py = OFFSET_Y + i * CELL_SIZE;
                setfillcolor(((i + j) & 1) ? tc.cellA : tc.cellB);
                solidrectangle(px + 1, py + 1, px + CELL_SIZE - 1, py + CELL_SIZE - 1);
            }
        }

        setlinecolor(tc.grid);
        setlinestyle(PS_SOLID, 1);
        for (int i = 0; i <= BOARD_SIZE; i++) {
            line(OFFSET_X, OFFSET_Y + i * CELL_SIZE, OFFSET_X + BOARD_SIZE * CELL_SIZE, OFFSET_Y + i * CELL_SIZE);
            line(OFFSET_X + i * CELL_SIZE, OFFSET_Y, OFFSET_X + i * CELL_SIZE, OFFSET_Y + BOARD_SIZE * CELL_SIZE);
        }
    }
}

/* ---------------------------------------------------------------
 * System init / close
 * --------------------------------------------------------------- */
static void BuildCharacterPath(int id, TCHAR* buf, size_t bufCount) {
    _stprintf_s(buf, (unsigned)bufCount, _T("GUI\\Characters\\%d.png"), id);
}

void ReloadAvatars(void) {
    TCHAR path[128];
    g_hasAvatar1 = false;
    g_hasAvatar2 = false;

    BuildCharacterPath(_CHAR_P1, path, 128);
    if (FileExists(path)) {
        loadimage(&imgAvatarP1, path);
        MakeEdgeBackgroundTransparent(&imgAvatarP1);
        g_hdcAv1 = GetImageHDC(&imgAvatarP1);
        g_hasAvatar1 = true;
    }

    BuildCharacterPath(_CHAR_P2, path, 128);
    if (FileExists(path)) {
        loadimage(&imgAvatarP2, path);
        MakeEdgeBackgroundTransparent(&imgAvatarP2);
        g_hdcAv2 = GetImageHDC(&imgAvatarP2);
        g_hasAvatar2 = true;
    }
}

void InitSystem(void) {
    SetupScreenLayout();
    initgraph(SCREEN_W, SCREEN_H);
    TryLoadPixelFont();
    CenterGameWindow();
    InitAudioPaths();

    loadimage(&imgBackground, _T("GUI\\Background\\man_hinh_nen_khi_choi.png"), SCREEN_W, SCREEN_H);

    LoadStoneImage(kStoneXPathCandidates, &imgX, &g_hasPngX);
    LoadStoneImage(kStoneOPathCandidates, &imgO, &g_hasPngO);

    g_hdcScreen = GetImageHDC(NULL);
    LoadBoardAssets();
    ReloadAvatars();

    if (FileExists(_T("GUI\\khungvaHienThiNguoiWin\\playerWin.png"))) {
        loadimage(&imgWinnerFrame, _T("GUI\\khungvaHienThiNguoiWin\\playerWin.png"), UiScale(640), UiScale(640));
        g_hasWinnerFrame = true;
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

    if (g_hasPngX) {
        int stoneSz = StoneDrawSize();
        int px = OFFSET_X + col * CELL_SIZE + (CELL_SIZE - stoneSz) / 2;
        int py = OFFSET_Y + row * CELL_SIZE + (CELL_SIZE - stoneSz) / 2;
        DrawSpriteScaled(px, py, stoneSz, stoneSz, &imgX);
        return;
    }

    DrawGlossyStone(cx, cy, r, RGB(180, 40, 40), RGB(220, 80, 60));
}

void DrawO(int row, int col) {
    int cx = OFFSET_X + col * CELL_SIZE + CELL_SIZE / 2;
    int cy = OFFSET_Y + row * CELL_SIZE + CELL_SIZE / 2;
    int r  = CELL_SIZE / 2 - 4;

    if (g_hasPngO) {
        int stoneSz = StoneDrawSize();
        int px = OFFSET_X + col * CELL_SIZE + (CELL_SIZE - stoneSz) / 2;
        int py = OFFSET_Y + row * CELL_SIZE + (CELL_SIZE - stoneSz) / 2;
        DrawSpriteScaled(px, py, stoneSz, stoneSz, &imgO);
        return;
    }

    DrawGlossyStone(cx, cy, r, RGB(28, 28, 28), RGB(70, 70, 70));
}

static void DrawStoneShadow(int row, int col, int piece) {
    int stoneSz = StoneDrawSize();
    int px = OFFSET_X + col * CELL_SIZE + (CELL_SIZE - stoneSz) / 2;
    int py = OFFSET_Y + row * CELL_SIZE + (CELL_SIZE - stoneSz) / 2;
    const BYTE kGhostAlpha = 96;

    if (piece == -1 && g_hasPngX) {
        DrawSpriteScaledAlpha(px, py, stoneSz, stoneSz, &imgX, kGhostAlpha);
        return;
    }
    if (piece == 1 && g_hasPngO) {
        DrawSpriteScaledAlpha(px, py, stoneSz, stoneSz, &imgO, kGhostAlpha);
        return;
    }

    int cx = OFFSET_X + col * CELL_SIZE + CELL_SIZE / 2;
    int cy = OFFSET_Y + row * CELL_SIZE + CELL_SIZE / 2;
    int r  = CELL_SIZE / 2 - 4;
    if (piece == -1)
        DrawGlossyStone(cx, cy, r, RGB(100, 50, 50), RGB(120, 70, 60));
    else
        DrawGlossyStone(cx, cy, r, RGB(50, 50, 50), RGB(70, 70, 70));
}

static bool CanPreviewPlacement(int row, int col) {
    if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) return false;
    if (_A[row][col].c != 0) return false;
    if (_VS_BOT && _TURN != 1) return false;
    return true;
}

void DrawPlacementPreview(int row, int col) {
    if (!CanPreviewPlacement(row, col)) return;
    int piece = (_TURN == 1) ? -1 : 1;
    DrawStoneShadow(row, col, piece);
}

bool BoardPointToCell(int mx, int my, int* outRow, int* outCol) {
    int col = (mx - OFFSET_X) / CELL_SIZE;
    int row = (my - OFFSET_Y) / CELL_SIZE;
    if (row < 0 || row >= BOARD_SIZE || col < 0 || col >= BOARD_SIZE) return false;
    if (outRow) *outRow = row;
    if (outCol) *outCol = col;
    return true;
}

/* ---------------------------------------------------------------
 * Move-order number overlay (drawn after all stones)
 * --------------------------------------------------------------- */
static void DrawMoveNumbers(void) {
    setbkmode(TRANSPARENT);
    ApplyUiFont(7, FW_BOLD);
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (_MOVE_ORDER[i][j] <= 0) continue;
            int cx = OFFSET_X + j * CELL_SIZE + CELL_SIZE / 2;
            int cy = OFFSET_Y + i * CELL_SIZE + CELL_SIZE / 2;
            TCHAR buf[8];
            _stprintf_s(buf, _T("%d"), _MOVE_ORDER[i][j]);
            int tw = textwidth(buf);
            int th = textheight(buf);
            settextcolor(_A[i][j].c == 1 ? RGB(220, 220, 220) : RGB(255, 255, 255));
            outtextxy(cx - tw / 2, cy - th / 2, buf);
        }
    }
}

void DrawCursor(int row, int col) {
    int x = OFFSET_X + col * CELL_SIZE;
    int y = OFFSET_Y + row * CELL_SIZE;
    int pad = UiScale(2);

    setlinecolor(RGB(255, 220, 60));
    setlinestyle(PS_SOLID, UiScale(3));
    rectangle(x - pad, y - pad, x + CELL_SIZE + pad, y + CELL_SIZE + pad);

    setlinecolor(RGB(255, 255, 255));
    setlinestyle(PS_SOLID, UiScale(1));
    rectangle(x + pad, y + pad, x + CELL_SIZE - pad, y + CELL_SIZE - pad);
}

void GetUndoButtonRect(int* outX, int* outY, int* outW, int* outH) {
    int w = g_sidebarW;
    int h = UiScale(44);
    if (outW) *outW = w;
    if (outH) *outH = h;
    if (outX) *outX = g_sidebarX;
    if (outY) *outY = g_undoY;
}

void DrawPlayerInfo(void) {
    DrawMatchInfo();

    DrawPlayerInfoBox(g_infoPanelX, g_infoPanel1Y, g_infoPanelW, g_infoPanelH,
                      UI_ACCENT_P1, _NAME_P1, _T("X"),
                      &imgAvatarP1, g_hasAvatar1,
                      _NAME_P1, _MOVE_P1, _WIN_P1, _TURN == 1);
    DrawPlayerInfoBox(g_infoPanelX, g_infoPanel2Y, g_infoPanelW, g_infoPanelH,
                      UI_ACCENT_P2, _NAME_P2, _T("O"),
                      &imgAvatarP2, g_hasAvatar2,
                      _NAME_P2, _MOVE_P2, _WIN_P2, _TURN != 1);

    bool canUndo = (_UNDO_TOP >= 1);
    int undoX, undoY, undoW, undoH;
    GetUndoButtonRect(&undoX, &undoY, &undoW, &undoH);
    DrawActionButton(undoX, undoY, undoW, undoH, _T("UNDO"), false, canUndo);
}

void DrawHudMenuButton(void) {
    const int menuX = SCREEN_W - UiScale(82);
    const int menuY = UiScale(18);
    const int s     = UiScale(56);

    setfillcolor(RGB(10, 12, 20));
    fillroundrect(menuX + UiScale(2), menuY + UiScale(3), menuX + s + UiScale(2), menuY + s + UiScale(3),
                  UiScale(10), UiScale(10));
    setfillcolor(RGB(58, 66, 94));
    fillroundrect(menuX, menuY, menuX + s, menuY + s, UiScale(10), UiScale(10));
    setlinecolor(RGB(255, 210, 80));
    setlinestyle(PS_SOLID, UiScale(2));
    roundrect(menuX, menuY, menuX + s, menuY + s, UiScale(10), UiScale(10));

    setfillcolor(RGB(255, 240, 180));
    int lx = menuX + UiScale(14);
    int lw = s - UiScale(28);
    int barH = UiScale(4);
    solidroundrect(lx, menuY + UiScale(16), lx + lw, menuY + UiScale(16) + barH, UiScale(2), UiScale(2));
    solidroundrect(lx, menuY + UiScale(26), lx + lw, menuY + UiScale(26) + barH, UiScale(2), UiScale(2));
    solidroundrect(lx, menuY + UiScale(36), lx + lw, menuY + UiScale(36) + barH, UiScale(2), UiScale(2));
}

void RenderGame(void) {
    DrawBoard();

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (_A[i][j].c == -1)     DrawX(i, j);
            else if (_A[i][j].c == 1) DrawO(i, j);
        }
    }

    DrawPlacementPreview(_X, _Y);
    DrawMoveNumbers();
    DrawCursor(_X, _Y);
    DrawPlayerInfo();
    DrawHudMenuButton();
}

/* ---------------------------------------------------------------
 * Winning line — 3-layer glow effect
 * --------------------------------------------------------------- */
void DrawWinningLine(int winner) {
    int startX = OFFSET_X + _WIN_C1 * CELL_SIZE + CELL_SIZE / 2;
    int startY = OFFSET_Y + _WIN_R1 * CELL_SIZE + CELL_SIZE / 2;
    int endX   = OFFSET_X + _WIN_C2 * CELL_SIZE + CELL_SIZE / 2;
    int endY   = OFFSET_Y + _WIN_R2 * CELL_SIZE + CELL_SIZE / 2;

    setlinecolor(RGB(20, 10, 0));
    setlinestyle(PS_SOLID, 14);
    line(startX + 3, startY + 3, endX + 3, endY + 3);

    setlinecolor(RGB(255, 210, 60));
    setlinestyle(PS_SOLID, 7);
    line(startX, startY, endX, endY);

    setlinecolor(RGB(255, 248, 210));
    setlinestyle(PS_SOLID, 2);
    line(startX, startY, endX, endY);

    IMAGE* effectImg = NULL;
    if (winner == -1 && g_hasPngX)      effectImg = &imgX;
    else if (winner == 1 && g_hasPngO)  effectImg = &imgO;

    if (effectImg && effectImg->getwidth() > 0) {
        int midX = (startX + endX) / 2;
        int midY = (startY + endY) / 2;
        int ew = StoneDrawSize() + UiScale(20);
        DrawSpriteScaled(midX - ew / 2, midY - ew / 2, ew, ew, effectImg);
    }
}

void DrawWinBanner(int winner) {
    if (!g_hasWinnerFrame || imgWinnerFrame.getwidth() <= 0 || imgWinnerFrame.getheight() <= 0) return;

    int bw = imgWinnerFrame.getwidth();
    int bh = imgWinnerFrame.getheight();
    int bx = (ScreenW() - bw) / 2;
    int by = (ScreenH() - bh) / 2;
    putimage(bx, by, &imgWinnerFrame);

    const TCHAR* winnerName = (winner == -1) ? _NAME_P1 : _NAME_P2;
    if (!winnerName || !winnerName[0]) {
        winnerName = (winner == -1) ? _T("Player 1") : _T("Player 2");
    }

    auto ApplyChampionFont = [&](int fontSize) {
        ApplyGameFont(fontSize, FW_BOLD);
    };

    auto DrawTightTextLine = [&](const std::basic_string<TCHAR>& line, int x, int y, COLORREF color, int spacingAdjustPx) {
        ApplyChampionFont(32);
        setbkmode(TRANSPARENT);
        settextcolor(RGB(18, 20, 32));

        int totalW = 0;
        for (size_t i = 0; i < line.size(); ++i) {
            TCHAR ch[2] = { line[i], 0 };
            totalW += textwidth(ch);
            if (i + 1 < line.size()) totalW += spacingAdjustPx;
        }

        int curX = x - totalW / 2;
        for (size_t i = 0; i < line.size(); ++i) {
            TCHAR ch[2] = { line[i], 0 };
            int cw = textwidth(ch);
            outtextxy(curX + UiScale(2), y + UiScale(2), ch);
            curX += cw;
            if (i + 1 < line.size()) curX += spacingAdjustPx;
        }

        curX = x - totalW / 2;
        settextcolor(color);
        for (size_t i = 0; i < line.size(); ++i) {
            TCHAR ch[2] = { line[i], 0 };
            int cw = textwidth(ch);
            outtextxy(curX, y, ch);
            curX += cw;
            if (i + 1 < line.size()) curX += spacingAdjustPx;
        }
    };

    TCHAR championText[128];
    _stprintf_s(championText, _T("%s is champion"), winnerName);

    auto drawCenteredLine = [&](const TCHAR* text, int cx, int y, COLORREF color, int fontSize) {
        ApplyChampionFont(fontSize);
        setbkmode(TRANSPARENT);
        int tw = textwidth(text);
        settextcolor(RGB(18, 20, 32));
        outtextxy(cx - tw / 2 + UiScale(2), y + UiScale(2), text);
        settextcolor(color);
        outtextxy(cx - tw / 2, y, text);
    };

    auto wrapAndDraw = [&](const TCHAR* text, int x, int y, int w, int h, int fontSize, COLORREF color) {
        auto buildLines = [&](std::vector<std::basic_string<TCHAR>>& lines) {
            lines.clear();
            std::basic_string<TCHAR> current;
            std::basic_string<TCHAR> word;
            for (const TCHAR* p = text; ; ++p) {
                TCHAR ch = *p;
                bool atEnd = (ch == 0);
                if (!atEnd && ch != _T(' ')) {
                    word.push_back(ch);
                    continue;
                }

                if (!word.empty()) {
                    std::basic_string<TCHAR> candidate = current;
                    if (!candidate.empty()) candidate += _T(' ');
                    candidate += word;

                    if (current.empty() || textwidth(candidate.c_str()) <= w) {
                        current = candidate;
                    } else {
                        if (!current.empty()) lines.push_back(current);
                        current = word;
                        if (textwidth(current.c_str()) > w) {
                            std::basic_string<TCHAR> chopped;
                            for (size_t i = 0; i < current.size(); ++i) {
                                chopped.push_back(current[i]);
                                if (textwidth(chopped.c_str()) > w && chopped.size() > 1) {
                                    chopped.pop_back();
                                    if (!chopped.empty()) lines.push_back(chopped);
                                    chopped.clear();
                                    chopped.push_back(current[i]);
                                }
                            }
                            current = chopped;
                        }
                    }
                    word.clear();
                }

                if (atEnd) break;
                if (ch == _T(' ')) {
                    if (!current.empty()) current.push_back(_T(' '));
                }
            }
            if (!word.empty()) {
                if (current.empty()) current = word;
                else {
                    std::basic_string<TCHAR> candidate = current;
                    candidate += _T(' ');
                    candidate += word;
                    if (textwidth(candidate.c_str()) <= w) current = candidate;
                    else {
                        lines.push_back(current);
                        current = word;
                    }
                }
            }
            if (!current.empty()) lines.push_back(current);
        };

        int chosenFont = fontSize;
        std::vector<std::basic_string<TCHAR>> lines;
        for (; chosenFont >= 24; --chosenFont) {
            ApplyChampionFont(chosenFont);
            buildLines(lines);

            int lineH = textheight(_T("A"));
            int totalH = (int)lines.size() * lineH + ((int)lines.size() - 1) * UiScale(4);
            int maxW = 0;
            for (const auto& line : lines) {
                int tw = textwidth(line.c_str());
                if (tw > maxW) maxW = tw;
            }
            if (maxW <= w && totalH <= h) break;
        }

        if (lines.empty()) return;

        int lineH = textheight(_T("A"));
        int totalH = (int)lines.size() * lineH + ((int)lines.size() - 1) * UiScale(4);
        int drawY = y + (h - totalH) / 2;
        int centerX = x + w / 2;
        int spacingAdjustPx = -UiScale(1);
        for (const auto& line : lines) {
            DrawTightTextLine(line, centerX, drawY, color, spacingAdjustPx);
            drawY += lineH + UiScale(4);
        }
    };

    int textX = bx + bw / 2;
    int textY = by + (int)(bh * 0.82f);
    int textW = (int)(bw * 0.72f);
    int textH = (int)(bh * 0.12f);
    drawCenteredLine(_T("WINNER!"), textX, by + UiScale(14), RGB(255, 180, 60), 18);
    wrapAndDraw(championText, bx + (bw - textW) / 2, textY, textW, textH, 32, WHITE);
}

static void DrawStoneScaledAt(int row, int col, IMAGE* img, float scale) {
    if (!img || img->getwidth() <= 0 || img->getheight() <= 0) return;
    int cellCenterX = OFFSET_X + col * CELL_SIZE + CELL_SIZE / 2;
    int cellCenterY = OFFSET_Y + row * CELL_SIZE + CELL_SIZE / 2;
    int base = StoneDrawSize();
    int drawW = (int)(base * scale);
    int drawH = drawW;
    DrawSpriteScaled(cellCenterX - drawW / 2, cellCenterY - drawH / 2, drawW, drawH, img);
}

static void CollectWinningCells(std::vector<std::pair<int,int>>& out) {
    out.clear();
    int r1 = _WIN_R1, c1 = _WIN_C1, r2 = _WIN_R2, c2 = _WIN_C2;
    int dr = 0, dc = 0;
    if (r2 > r1) dr = 1; else if (r2 < r1) dr = -1; else dr = 0;
    if (c2 > c1) dc = 1; else if (c2 < c1) dc = -1; else dc = 0;
    int r = r1, c = c1;
    while (true) {
        out.push_back({r,c});
        if (r == r2 && c == c2) break;
        r += dr; c += dc;
        if ((int)out.size() > BOARD_SIZE) break;
    }
}

void ShowWinScreenUntilDismiss(int winner) {
    ExMessage msg;

    // Build winning cells (should be 5).
    std::vector<std::pair<int,int>> winCells;
    CollectWinningCells(winCells);

    IMAGE* effectImg = NULL;
    if (winner == -1 && g_hasPngX)      effectImg = &imgX;
    else if (winner == 1 && g_hasPngO)  effectImg = &imgO;

    // Animate each cell enlarging slightly in sequence.
    const int stepsPerCell = 8;
    const int msPerFrame = 20; // ~50fps
    const float startScale = 1.0f;
    const float endScale = 1.18f; // slightly larger

    for (size_t idx = 0; idx < winCells.size(); ++idx) {
        for (int step = 0; step <= stepsPerCell; ++step) {
            float t = (float)step / (float)stepsPerCell;
            float scale = startScale + (endScale - startScale) * t;

            BeginBatchDraw();
            RenderGame();
            DrawWinningLine(winner);

            // draw previously highlighted cells at endScale
            for (size_t j = 0; j < idx; ++j) {
                if (effectImg) DrawStoneScaledAt(winCells[j].first, winCells[j].second, effectImg, endScale);
            }

            // current cell animating
            if (effectImg) DrawStoneScaledAt(winCells[idx].first, winCells[idx].second, effectImg, scale);

            FlushBatchDraw();

            // allow skipping animation on input
            if (peekmessage(&msg, EM_KEY | EM_MOUSE | EM_WINDOW)) {
                while (peekmessage(&msg, EM_KEY | EM_MOUSE | EM_WINDOW)) {
                    if (msg.message == WM_LBUTTONDOWN || msg.message == WM_KEYDOWN) {
                        // skip remaining animation
                        idx = winCells.size();
                        break;
                    }
                }
                if (idx >= winCells.size()) break;
            }

            Sleep(msPerFrame);
        }
        if (idx >= winCells.size()) break;
        // small pause after finishing this cell
        DWORD tstart = GetTickCount();
        while (GetTickCount() - tstart < 80) {
            if (peekmessage(&msg, EM_KEY | EM_MOUSE | EM_WINDOW)) {
                while (peekmessage(&msg, EM_KEY | EM_MOUSE | EM_WINDOW)) {
                    if (msg.message == WM_LBUTTONDOWN || msg.message == WM_KEYDOWN) { idx = winCells.size(); break; }
                }
                if (idx >= winCells.size()) break;
            }
            Sleep(5);
        }
    }

    // After all highlighted, show banner and wait for dismiss.
    while (true) {
        BeginBatchDraw();
        RenderGame();
        DrawWinningLine(winner);
        // draw all highlighted at final scale
        if (effectImg) {
            for (auto &p : winCells) DrawStoneScaledAt(p.first, p.second, effectImg, endScale);
        }
        DrawWinBanner(winner);
        FlushBatchDraw();

        if (!IsWindow(GetHWnd())) exit(0);
        while (peekmessage(&msg, EM_KEY | EM_MOUSE | EM_WINDOW)) {
            if (msg.message == WM_LBUTTONDOWN || msg.message == WM_KEYDOWN) return;
        }
        Sleep(16);
    }
}
