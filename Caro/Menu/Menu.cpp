#include "Defs/Defs.h"
#include "View/View.h"
#include "Menu/Menu.h"
#include "Audio/Audio.h"
#include <graphics.h>
#include <tchar.h>
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <windows.h>

#pragma comment(lib, "Msimg32.lib")

#include <filesystem>
#include <vector>
#include <string>
#include <chrono>
#include <ctime>

namespace fs = std::filesystem;

#define S(v) UiScale(v)

struct MenuRect { int x, y, w, h; };

static MenuRect CenterPanel(int designW, int designH, int designOffsetY = 0) {
    MenuRect r;
    r.w = S(designW);
    r.h = S(designH);
    r.x = (ScreenW() - r.w) / 2;
    r.y = (ScreenH() - r.h) / 2 + S(designOffsetY);
    return r;
}

static int CenterX(int w) { return (ScreenW() - w) / 2; }

static void DrawMenuPanel(const MenuRect& r) {
    setlinecolor(RGB(255, 210, 80));
    setlinestyle(PS_SOLID, S(2));
    setfillcolor(RGB(36, 42, 60));
    fillroundrect(r.x, r.y, r.x + r.w, r.y + r.h, S(20), S(20));
    setfillcolor(RGB(52, 58, 82));
    fillroundrect(r.x + S(10), r.y + S(10), r.x + r.w - S(10), r.y + r.h - S(10), S(15), S(15));
    rectangle(r.x, r.y, r.x + r.w, r.y + r.h);
}

static void DrawTitleCentered(int y, const TCHAR* title, COLORREF color, int fontSize) {
    ApplyGameFont(fontSize, FW_BOLD);
    int tw = textwidth(title);
    DrawTextWithShadow((ScreenW() - tw) / 2, y, title, color, fontSize);
}

static void DrawTextCenteredIn(int panelX, int panelW, int y, const TCHAR* text, COLORREF color, int fontSize) {
    ApplyGameFont(fontSize, FW_BOLD);
    int tw = textwidth(text);
    DrawTextWithShadow(panelX + (panelW - tw) / 2, y, text, color, fontSize);
}

static bool PointInRect(int px, int py, int x, int y, int w, int h) {
    return px >= x && px <= x + w && py >= y && py <= y + h;
}

static IMAGE g_menuBgCache;
static bool  g_menuBgLoaded = false;

static IMAGE* GetMenuBackground(void) {
    if (!g_menuBgLoaded) {
        loadimage(&g_menuBgCache, _T("menu_bg.jpg"), ScreenW(), ScreenH());
        g_menuBgLoaded = true;
    }
    return &g_menuBgCache;
}

static void DrawPixelPanel(int x, int y, int w, int h) {
    setfillcolor(RGB(36, 42, 60));
    solidrectangle(x, y, x + w, y + h);
    setfillcolor(RGB(52, 58, 82));
    solidrectangle(x + S(8), y + S(8), x + w - S(8), y + h - S(8));
    setlinecolor(RGB(255, 210, 80));
    setlinestyle(PS_SOLID, S(2));
    rectangle(x, y, x + w, y + h);
}

struct MenuButton {
    const TCHAR* path;
    int srcW;
    int srcH;
    int x, y, w, h;
    IMAGE img;
};

static void LayoutMenuButtons(MenuButton* buttons, int count, int screenW, int startY, int btnH, int gap) {
    int y = startY;
    for (int i = 0; i < count; i++) {
        buttons[i].h = btnH;
        buttons[i].w = (btnH * buttons[i].srcW) / buttons[i].srcH;
        buttons[i].x = (screenW - buttons[i].w) / 2;
        buttons[i].y = y;
        y += btnH + gap;
    }
}

static void LoadMenuButtonImages(MenuButton* buttons, int count) {
    for (int i = 0; i < count; i++) {
        loadimage(&buttons[i].img, buttons[i].path, buttons[i].w, buttons[i].h);
    }
}

static void DrawMenuButton(const MenuButton& btn) {
    HDC hdcDest = GetImageHDC(NULL);
    HDC hdcSrc  = GetImageHDC(const_cast<IMAGE*>(&btn.img));
    int sw = btn.img.getwidth();
    int sh = btn.img.getheight();
    COLORREF key = GetPixel(hdcSrc, 0, 0);
    TransparentBlt(hdcDest, btn.x, btn.y, btn.w, btn.h, hdcSrc, 0, 0, sw, sh, key);
}

static int HitMenuButton(const MenuButton* buttons, int count, int mx, int my) {
    for (int i = 0; i < count; i++) {
        const MenuButton& b = buttons[i];
        if (mx >= b.x && mx <= b.x + b.w && my >= b.y && my <= b.y + b.h)
            return i;
    }
    return -1;
}

static std::string FileMTimeShort(const fs::path& p) {
    std::error_code ec;
    if (!fs::exists(p, ec)) return "";
    auto ftime = fs::last_write_time(p, ec);
    if (ec) return "";
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
    std::time_t tt = std::chrono::system_clock::to_time_t(sctp);
    std::tm ltm;
    localtime_s(&ltm, &tt);
    char b[48];
    strftime(b, sizeof(b), "%m/%d/%y %H:%M", &ltm);
    return std::string(b);
}

void DrawTextWithShadow(int x, int y, const TCHAR* text, COLORREF textColor, int fontSize) {
    setbkmode(TRANSPARENT);
    ApplyGameFont(fontSize, FW_BOLD);
    settextcolor(RGB(18, 20, 32));
    outtextxy(x + UiScale(1), y + UiScale(1), text);
    settextcolor(textColor);
    outtextxy(x, y, text);
}

void DrawWoodenButton(int x, int y, int width, int height, const TCHAR* text, bool hovered) {
    DrawActionButton(x, y, width, height, text, hovered, true);
}

// Biến toàn cục để lưu tên file người chơi đã chọn
std::string _SELECTED_FILE = "";

// Hàm vẽ chữ có bóng

// --- HÀM MỚI: Bảng thông tin chung cho ABOUT US và HELP ---
void ShowInfoMenu(const TCHAR* title, const TCHAR* line1, const TCHAR* line2, const TCHAR* line3) {
    IMAGE* imgMenuBg = GetMenuBackground();
    ExMessage msg;
    const int btnW = S(200);
    const int btnH = S(58);
    int mx = 0, my = 0;

    while (true) {
        MenuRect panel = CenterPanel(560, 440, 0);
        int btnX = panel.x + (panel.w - btnW) / 2;
        int btnY = panel.y + panel.h - S(72);

        if (peekmessage(&msg, EM_MOUSE | EM_WINDOW)) {
            if (msg.message == WM_MOUSEMOVE) { mx = msg.x; my = msg.y; }
            if (!IsWindow(GetHWnd())) exit(0);
            if (msg.message == WM_LBUTTONDOWN) {
                if (PointInRect(msg.x, msg.y, btnX, btnY, btnW, btnH)) {
                    PlayClickSound();
                    return;
                }
            }
        }

        BeginBatchDraw();
        putimage(0, 0, imgMenuBg);
        DrawMenuPanel(panel);
        DrawTitleCentered(panel.y + S(32), title, WHITE, 18);
        DrawTextCenteredIn(panel.x, panel.w, panel.y + S(110), line1, RGB(255, 200, 100), 13);
        DrawTextCenteredIn(panel.x, panel.w, panel.y + S(165), line2, WHITE, 12);
        DrawTextCenteredIn(panel.x, panel.w, panel.y + S(220), line3, WHITE, 12);
        DrawWoodenButton(btnX, btnY, btnW, btnH, _T("BACK"),
            PointInRect(mx, my, btnX, btnY, btnW, btnH));
        FlushBatchDraw();
        Sleep(1);
    }
}

bool ShowLoadMenuUI() {
    struct LoadFileEntry {
        std::string name;
        fs::path path;
    };
    std::vector<LoadFileEntry> allFiles;
    for (const auto& entry : fs::directory_iterator(".")) {
        if (entry.path().extension() == ".txt") {
            allFiles.push_back({ entry.path().filename().string(), entry.path() });
        }
    }

    std::string searchQuery = "";
    int selectedIndex = 0;
    IMAGE* imgMenuBg = GetMenuBackground();
    ExMessage msg;

    const int btnW = S(180);
    const int btnH = S(58);
    const int btnGap = S(28);
    int mx = 0, my = 0;

    while (true) {
        std::vector<LoadFileEntry> filteredFiles;
        for (const auto& f : allFiles) {
            if (searchQuery.empty() || f.name.find(searchQuery) != std::string::npos) {
                filteredFiles.push_back(f);
            }
        }
        if (selectedIndex >= (int)filteredFiles.size()) selectedIndex = max(0, (int)filteredFiles.size() - 1);

        MenuRect panel = CenterPanel(760, 560, 0);
        int innerX = panel.x + S(24);
        int innerW = panel.w - S(48);
        int listW  = innerW * 58 / 100;
        int searchX = innerX + listW + S(16);
        int searchW = innerW - listW - S(16);
        int contentY = panel.y + S(76);
        int btnRowW = btnW * 2 + btnGap;
        int loadX = panel.x + (panel.w - btnRowW) / 2;
        int backX = loadX + btnW + btnGap;
        int btnY  = panel.y + panel.h - S(68);

        BeginBatchDraw();
        putimage(0, 0, imgMenuBg);
        DrawMenuPanel(panel);
        DrawTitleCentered(panel.y + S(26), _T("LOAD GAME"), WHITE, 18);

        setfillcolor(RGB(44, 50, 70));
        fillroundrect(searchX, contentY, searchX + searchW, contentY + S(72), S(10), S(10));
        DrawTextWithShadow(searchX + S(12), contentY + S(12), _T("Search:"), RGB(200, 200, 200), 12);
        TCHAR tSearch[256];
        _stprintf_s(tSearch, 256, _T("%hs"), searchQuery.c_str());
        DrawTextWithShadow(searchX + S(12), contentY + S(36), tSearch, WHITE, 12);
        DrawTextWithShadow(searchX + S(12), contentY + S(58), _T("(Type on keyboard)"), RGB(255, 200, 100), 10);

        int listY = contentY;
        if (filteredFiles.empty()) {
            DrawTextWithShadow(innerX + S(12), listY, _T("No files found."), RGB(200, 200, 200), 12);
        } else {
            for (int i = 0; i < (int)filteredFiles.size() && i < 8; i++) {
                std::string line = filteredFiles[i].name + "  |  " + FileMTimeShort(filteredFiles[i].path);
                if (line.size() > 54) line = line.substr(0, 51) + "...";
                TCHAR tFile[320];
                _stprintf_s(tFile, _T("%hs"), line.c_str());
                if (i == selectedIndex) {
                    setfillcolor(RGB(65, 85, 140));
                    fillroundrect(innerX, listY - S(4), innerX + listW, listY + S(30), S(5), S(5));
                }
                DrawTextWithShadow(innerX + S(12), listY, tFile, WHITE, 11);
                listY += S(36);
            }
        }

        DrawWoodenButton(loadX, btnY, btnW, btnH, _T("LOAD"),
            PointInRect(mx, my, loadX, btnY, btnW, btnH));
        DrawWoodenButton(backX, btnY, btnW, btnH, _T("BACK"),
            PointInRect(mx, my, backX, btnY, btnW, btnH));

        FlushBatchDraw();
        if (!IsWindow(GetHWnd())) exit(0);

        while (peekmessage(&msg, EM_MOUSE | EM_KEY | EM_CHAR | EM_WINDOW)) {
            if (msg.message == WM_MOUSEMOVE) { mx = msg.x; my = msg.y; }
            if (msg.message == WM_LBUTTONDOWN) {
                int mx = msg.x, my = msg.y;
                if (PointInRect(mx, my, backX, btnY, btnW, btnH)) {
                    PlayClickSound();
                    return false;
                }
                if (PointInRect(mx, my, loadX, btnY, btnW, btnH)) {
                    if (!filteredFiles.empty()) {
                        _SELECTED_FILE = filteredFiles[selectedIndex].name;
                        return true;
                    }
                }
                if (mx >= innerX && mx <= innerX + listW && my >= contentY - S(4) && my <= contentY + S(8 * 36)) {
                    int clickedIdx = (my - contentY) / S(36);
                    if (clickedIdx >= 0 && clickedIdx < (int)filteredFiles.size()) selectedIndex = clickedIdx;
                }
            }
            else if (msg.message == WM_KEYDOWN) {
                if (msg.vkcode == VK_UP) selectedIndex = max(0, selectedIndex - 1);
                if (msg.vkcode == VK_DOWN) selectedIndex = min((int)filteredFiles.size() - 1, selectedIndex + 1);
                if (msg.vkcode == VK_BACK && !searchQuery.empty()) searchQuery.pop_back();
                if (msg.vkcode == VK_RETURN && !filteredFiles.empty()) {
                    _SELECTED_FILE = filteredFiles[selectedIndex].name;
                    return true;
                }
            }
            else if (msg.message == WM_CHAR) {
                char c = msg.ch;
                if (isalnum(c) || c == '_' || c == '-' || c == '.') searchQuery += c;
            }
        }
        Sleep(1);
    }
}

static const TCHAR* ThemeName(int t) {
    if (t == 1) return _T("Cave Rock");
    if (t == 2) return _T("Champion Arena");
    return _T("Grass Field");
}

void ShowSettingsMenu(void) {
    IMAGE* imgMenuBg = GetMenuBackground();

    ExMessage msg;
    int mx = 0, my = 0;

    const int btnW = S(170);
    const int btnH = S(58);
    const int smallBtnW = S(90);
    const int smallBtnH = S(52);

    while (true) {
        MenuRect panel = CenterPanel(560, 480, 0);
        int pairW = btnW * 2 + S(40);
        int pairX = panel.x + (panel.w - pairW) / 2;
        int volY  = panel.y + S(150);
        int themeY = panel.y + S(280);
        int themePairW = smallBtnW * 2 + S(220);
        int themeLeftX = panel.x + (panel.w - themePairW) / 2;
        int themeRightX = themeLeftX + smallBtnW + S(220);
        int backW = S(180);
        int backX = panel.x + (panel.w - backW) / 2;
        int backY = panel.y + panel.h - S(72);

        if (peekmessage(&msg, EM_MOUSE | EM_WINDOW)) {
            if (msg.message == WM_MOUSEMOVE) { mx = msg.x; my = msg.y; }
            if (!IsWindow(GetHWnd())) exit(0);

            if (msg.message == WM_LBUTTONDOWN) {
                if (PointInRect(msg.x, msg.y, pairX, volY, btnW, btnH)) {
                    PlayClickSound();
                    SetVolumePercent(GetVolumePercent() - 10);
                }
                else if (PointInRect(msg.x, msg.y, pairX + btnW + S(40), volY, btnW, btnH)) {
                    PlayClickSound();
                    SetVolumePercent(GetVolumePercent() + 10);
                }
                else if (PointInRect(msg.x, msg.y, themeLeftX, themeY, smallBtnW, smallBtnH)) {
                    PlayClickSound();
                    _BOARD_THEME = (_BOARD_THEME + 2) % 3;
                    ReloadBoardAssets();
                }
                else if (PointInRect(msg.x, msg.y, themeRightX, themeY, smallBtnW, smallBtnH)) {
                    PlayClickSound();
                    _BOARD_THEME = (_BOARD_THEME + 1) % 3;
                    ReloadBoardAssets();
                }
                else if (PointInRect(msg.x, msg.y, backX, backY, backW, btnH)) {
                    PlayClickSound();
                    EndBatchDraw();
                    return;
                }
            }
        }

        if (!IsWindow(GetHWnd())) exit(0);

        BeginBatchDraw();
        putimage(0, 0, imgMenuBg);
        DrawMenuPanel(panel);
        DrawTitleCentered(panel.y + S(28), _T("SETTINGS"), WHITE, 18);

        TCHAR volText[50];
        _stprintf_s(volText, 50, _T("VOLUME: %d %%"), GetVolumePercent());
        DrawTextCenteredIn(panel.x, panel.w, panel.y + S(95), volText, RGB(255, 200, 100), 14);

        DrawWoodenButton(pairX, volY, btnW, btnH, _T("- DOWN"),
            PointInRect(mx, my, pairX, volY, btnW, btnH));
        DrawWoodenButton(pairX + btnW + S(40), volY, btnW, btnH, _T("+ UP"),
            PointInRect(mx, my, pairX + btnW + S(40), volY, btnW, btnH));

        setlinecolor(RGB(70, 78, 108));
        setlinestyle(PS_SOLID, 1);
        line(panel.x + S(24), panel.y + S(240), panel.x + panel.w - S(24), panel.y + S(240));

        TCHAR themeText[60];
        _stprintf_s(themeText, 60, _T("THEME: %s"), ThemeName(_BOARD_THEME));
        DrawTextCenteredIn(panel.x, panel.w, panel.y + S(252), themeText, RGB(255, 200, 100), 13);
        DrawWoodenButton(themeLeftX, themeY, smallBtnW, smallBtnH, _T("<"),
            PointInRect(mx, my, themeLeftX, themeY, smallBtnW, smallBtnH));
        DrawWoodenButton(themeRightX, themeY, smallBtnW, smallBtnH, _T(">"),
            PointInRect(mx, my, themeRightX, themeY, smallBtnW, smallBtnH));

        DrawWoodenButton(backX, backY, backW, btnH, _T("BACK"),
            PointInRect(mx, my, backX, backY, backW, btnH));

        FlushBatchDraw();
        Sleep(1);
    }
}

/*
 * Chọn PVP (2 người) hoặc PVE (vs máy). Trả về: 0 = BACK, 1 = PVP, 2 = PVE
 */
int ShowNewGameModeMenu(void) {
    IMAGE* imgMenuBg = GetMenuBackground();
    ExMessage msg;
    int mx = 0, my = 0;

    const int btnW = S(340);
    const int btnH = S(58);
    const int backW = S(220);
    const int backH = S(58);

    while (true) {
        MenuRect panel = CenterPanel(520, 420, 0);
        int btnX  = panel.x + (panel.w - btnW) / 2;
        int pvpY  = panel.y + S(130);
        int pveY  = panel.y + S(210);
        int backX = panel.x + (panel.w - backW) / 2;
        int backY = panel.y + panel.h - S(72);

        while (peekmessage(&msg, EM_MOUSE | EM_KEY | EM_WINDOW)) {
            if (msg.message == WM_MOUSEMOVE) { mx = msg.x; my = msg.y; }
            if (!IsWindow(GetHWnd())) exit(0);
            if (msg.message == WM_LBUTTONDOWN) {
                if (PointInRect(msg.x, msg.y, btnX, pvpY, btnW, btnH)) {
                    PlayClickSound(); EndBatchDraw(); return 1;
                }
                if (PointInRect(msg.x, msg.y, btnX, pveY, btnW, btnH)) {
                    PlayClickSound(); EndBatchDraw(); return 2;
                }
                if (PointInRect(msg.x, msg.y, backX, backY, backW, backH)) {
                    PlayClickSound(); EndBatchDraw(); return 0;
                }
            }
            if (msg.message == WM_KEYDOWN && msg.vkcode == VK_ESCAPE) {
                PlayClickSound(); EndBatchDraw(); return 0;
            }
        }
        if (!IsWindow(GetHWnd())) exit(0);

        BeginBatchDraw();
        putimage(0, 0, imgMenuBg);
        DrawMenuPanel(panel);
        DrawTitleCentered(panel.y + S(32), _T("NEW GAME"), WHITE, 18);
        DrawTextCenteredIn(panel.x, panel.w, panel.y + S(82), _T("Choose game mode:"), RGB(255, 220, 180), 13);

        DrawWoodenButton(btnX, pvpY, btnW, btnH, _T("PVP"),
            PointInRect(mx, my, btnX, pvpY, btnW, btnH));
        DrawWoodenButton(btnX, pveY, btnW, btnH, _T("PVE"),
            PointInRect(mx, my, btnX, pveY, btnW, btnH));
        DrawWoodenButton(backX, backY, backW, backH, _T("BACK"),
            PointInRect(mx, my, backX, backY, backW, backH));

        FlushBatchDraw();
        Sleep(1);
    }
}

static bool CharacterImageExists(int id) {
    TCHAR path[128];
    _stprintf_s(path, 128, _T("GUI\\Characters\\%d.png"), id);
    return GetFileAttributes(path) != INVALID_FILE_ATTRIBUTES;
}

static int FirstAvailableCharacter(int excludeId) {
    for (int i = 1; i <= CHAR_COUNT; i++) {
        if (i != excludeId && CharacterImageExists(i))
            return i;
    }
    return 1;
}

static const TCHAR* PartnerCodename(int id) {
    static const TCHAR* kNames[] = {
        _T("???"),
        _T("Pikachu"),      _T("Pikachu"),      _T("Psyduck"),      _T("Charizard"),
        _T("Flareon"),      _T("Eevee"),        _T("Meowth"),       _T("Squirtle"),
        _T("Charmander"),   _T("Bulbasaur"),    _T("Pikachu"),      _T("Umbreon"),
        _T("Machamp"),      _T("Wartortle"),    _T("Snorlax"),      _T("Greninja"),
        _T("Deerling"),     _T("Lugia"),        _T("Horsea"),       _T("Tepig"),
        _T("Gengar"),       _T("Gardevoir"),    _T("Charizard X")
    };
    if (id < 1 || id > CHAR_COUNT) return kNames[0];
    return kNames[id];
}

static int PartnerDexNumber(int id) {
    static const int kDex[] = {
        0,
        25, 25, 54, 6, 136, 133, 52, 7, 4, 1, 25, 197, 68, 8, 143,
        658, 585, 249, 116, 498, 94, 282, 6
    };
    if (id < 1 || id > CHAR_COUNT) return 0;
    return kDex[id];
}

static bool CharSpriteColorsClose(DWORD px, COLORREF key, int tol) {
    int r = (int)((px >> 16) & 0xFF);
    int g = (int)((px >> 8) & 0xFF);
    int b = (int)(px & 0xFF);
    return abs(r - (int)GetRValue(key)) <= tol &&
           abs(g - (int)GetGValue(key)) <= tol &&
           abs(b - (int)GetBValue(key)) <= tol;
}

static void CharSpriteMakeTransparent(IMAGE* img) {
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
        if (!CharSpriteColorsClose(buf[i], key, tol)) return;
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

static void CharSpriteCleanupFringe(IMAGE* img) {
    CharSpriteMakeTransparent(img);

    DWORD* buf = (DWORD*)GetImageBuffer(img);
    int w = img->getwidth();
    int h = img->getheight();
    if (!buf || w <= 0 || h <= 0) return;

    for (int i = 0; i < w * h; i++) {
        if (((buf[i] >> 24) & 0xFF) == 0) continue;

        int r = (int)((buf[i] >> 16) & 0xFF);
        int g = (int)((buf[i] >> 8) & 0xFF);
        int b = (int)(buf[i] & 0xFF);

        // Remove cyan/blue editor outlines that look like UI glow borders.
        if (b > r + 35 && b > g + 15 && g > 70) {
            buf[i] &= 0x00FFFFFF;
        }
    }
}

static void DrawMenuSprite(int dx, int dy, int dw, int dh, IMAGE* img) {
    if (!img || img->getwidth() <= 0 || img->getheight() <= 0) return;
    HDC hdcDest = GetImageHDC(NULL);
    HDC hdcSrc  = GetImageHDC(img);
    BLENDFUNCTION bf = { AC_SRC_OVER, 0, 255, AC_SRC_ALPHA };
    AlphaBlend(hdcDest, dx, dy, dw, dh, hdcSrc, 0, 0, img->getwidth(), img->getheight(), bf);
}

static void DrawPokeballIcon(int cx, int cy, int r, bool selected) {
    setfillcolor(RGB(248, 248, 248));
    fillcircle(cx, cy, r);
    setfillcolor(selected ? RGB(255, 203, 5) : RGB(238, 21, 21));
    setlinecolor(RGB(32, 32, 40));
    setlinestyle(PS_SOLID, S(2));
    fillpie(cx - r, cy - r, cx + r, cy + r, 180, 360);
    setfillcolor(RGB(32, 32, 40));
    solidrectangle(cx - r, cy - S(2), cx + r, cy + S(2));
    fillcircle(cx, cy, r / 4);
    circle(cx, cy, r / 4);
    circle(cx, cy, r);
}

static void DrawSelectedRosterMarker(int x, int y, int size) {
    setfillcolor(RGB(255, 203, 5));
    solidroundrect(x + S(5), y + size - S(9), x + size - S(5), y + size - S(3), S(2), S(2));
    DrawPokeballIcon(x + size - S(13), y + S(5), S(5), true);
}

static void DrawPokemonPanel(const MenuRect& r) {
    setfillcolor(RGB(8, 10, 18));
    fillroundrect(r.x + S(4), r.y + S(5), r.x + r.w + S(4), r.y + r.h + S(5), S(22), S(22));
    setfillcolor(RGB(24, 28, 44));
    fillroundrect(r.x, r.y, r.x + r.w, r.y + r.h, S(22), S(22));
    setfillcolor(RGB(238, 21, 21));
    solidrectangle(r.x + S(12), r.y + S(12), r.x + r.w - S(12), r.y + S(28));
    setfillcolor(RGB(255, 203, 5));
    solidrectangle(r.x + S(12), r.y + S(28), r.x + r.w - S(12), r.y + S(34));
    setlinecolor(RGB(255, 210, 80));
    setlinestyle(PS_SOLID, S(3));
    roundrect(r.x, r.y, r.x + r.w, r.y + r.h, S(22), S(22));
}

static void DrawTrainerCard(int x, int y, int w, int h, int charId, IMAGE* portrait, bool hasPortrait,
                            const TCHAR* trainerName, COLORREF accent) {
    const int headerH = S(36);
    const int footerH = S(58);
    const int portraitMax = S(148);

    setfillcolor(RGB(16, 20, 34));
    fillroundrect(x, y, x + w, y + h, S(14), S(14));
    setfillcolor(accent);
    solidroundrect(x, y, x + w, y + headerH, S(14), S(14));
    solidrectangle(x, y + S(14), x + w, y + headerH);

    ApplyGameFont(11, FW_BOLD);
    setbkmode(TRANSPARENT);
    settextcolor(RGB(255, 255, 255));
    outtextxy(x + S(14), y + S(10), _T("TRAINER CARD"));

    int portraitSz = portraitMax;
    if (h - headerH - footerH - S(72) < portraitSz)
        portraitSz = h - headerH - footerH - S(72);
    if (portraitSz < S(96)) portraitSz = S(96);

    int px = x + (w - portraitSz) / 2;
    int py = y + headerH + S(10);

    setfillcolor(RGB(10, 12, 22));
    fillroundrect(px - S(6), py - S(6), px + portraitSz + S(6), py + portraitSz + S(6), S(10), S(10));
    setlinecolor(RGB(255, 210, 80));
    setlinestyle(PS_SOLID, S(2));
    roundrect(px - S(4), py - S(4), px + portraitSz + S(4), py + portraitSz + S(4), S(8), S(8));
    DrawPokeballIcon(px + portraitSz - S(10), py + S(10), S(6), false);

    if (hasPortrait && portrait)
        DrawMenuSprite(px, py, portraitSz, portraitSz, portrait);

    int infoY = py + portraitSz + S(12);
    TCHAR idLine[32];
    _stprintf_s(idLine, 32, _T("#%03d"), PartnerDexNumber(charId));
    DrawTextCenteredIn(x, w, infoY, idLine, RGB(255, 220, 140), 11);
    DrawTextCenteredIn(x, w, infoY + S(22), PartnerCodename(charId), RGB(255, 203, 5), 13);

    int footerY = y + h - footerH;
    setfillcolor(RGB(22, 26, 40));
    solidrectangle(x + S(8), footerY, x + w - S(8), y + h - S(8));
    setlinecolor(RGB(255, 210, 80));
    setlinestyle(PS_SOLID, 1);
    line(x + S(12), footerY, x + w - S(12), footerY);

    ApplyGameFont(9, FW_BOLD);
    settextcolor(RGB(160, 170, 190));
    DrawTextCenteredIn(x, w, footerY + S(8), _T("TRAINER"), RGB(160, 170, 190), 9);
    DrawTextCenteredIn(x, w, footerY + S(26), trainerName[0] ? trainerName : _T("???"), RGB(248, 244, 236), 12);
}

static void DrawNameField(int x, int y, int w, int h, const TCHAR* label, const TCHAR* value, bool focused) {
    ApplyGameFont(10, FW_BOLD);
    setbkmode(TRANSPARENT);
    settextcolor(RGB(255, 220, 140));
    outtextxy(x, y - S(26), label);

    setfillcolor(RGB(12, 14, 24));
    fillroundrect(x, y, x + w, y + h, S(8), S(8));
    setlinecolor(focused ? RGB(255, 203, 5) : RGB(255, 210, 80));
    setlinestyle(PS_SOLID, focused ? S(3) : S(2));
    roundrect(x, y, x + w, y + h, S(8), S(8));

    ApplyGameFont(13, FW_BOLD);
    settextcolor(RGB(248, 244, 236));
    outtextxy(x + S(14), y + (h - textheight(_T("A"))) / 2, value[0] ? value : _T("Type your name..."));

    if (focused && (GetTickCount() / 500) % 2 == 0) {
        int tw = value[0] ? textwidth(value) : textwidth(_T("Type your name..."));
        if (!value[0]) tw = S(4);
        setlinecolor(RGB(255, 255, 255));
        setlinestyle(PS_SOLID, S(2));
        line(x + S(14) + tw + S(2), y + S(8), x + S(14) + tw + S(2), y + h - S(8));
    }
    setlinestyle(PS_SOLID, 1);
}

static void AppendTrainerChar(TCHAR* buf, size_t bufCount, char c) {
    size_t len = _tcslen(buf);
    if (len + 1 >= bufCount) return;
    if (len == 0 && c == ' ') return;
    buf[len] = (TCHAR)c;
    buf[len + 1] = 0;
}

/*
 * Pokemon-themed trainer setup: name + partner pick.
 * Returns false = BACK, true = confirmed (writes charId and nameOut).
 */
static bool ShowTrainerSetup(int playerNo, int excludeId, TCHAR* nameOut, size_t nameCount, int* charIdOut) {
    const int THUMB   = S(72);
    const int COLS    = 6;
    const int GAP     = S(10);

    static IMAGE s_thumbs[CHAR_COUNT + 1];
    static IMAGE s_portrait[CHAR_COUNT + 1];
    static bool  s_thumbLoaded[CHAR_COUNT + 1]  = {};
    static bool  s_portraitLoaded[CHAR_COUNT + 1] = {};
    static int   s_thumbLoadPass = 0;
    const int kThumbLoadPass = 2;

    if (s_thumbLoadPass < kThumbLoadPass) {
        for (int id = 1; id <= CHAR_COUNT; id++) {
            s_thumbLoaded[id] = false;
            s_portraitLoaded[id] = false;
        }
        s_thumbLoadPass = kThumbLoadPass;
    }

    for (int id = 1; id <= CHAR_COUNT; id++) {
        if (s_thumbLoaded[id] || !CharacterImageExists(id)) continue;
        TCHAR path[128];
        _stprintf_s(path, 128, _T("GUI\\Characters\\%d.png"), id);
        loadimage(&s_thumbs[id], path, THUMB, THUMB);
        CharSpriteCleanupFringe(&s_thumbs[id]);
        s_thumbLoaded[id] = true;
    }

    IMAGE* imgMenuBg = GetMenuBackground();
    int selected = FirstAvailableCharacter(excludeId);
    ExMessage msg;
    int mx = 0, my = 0;

    const int backW = S(160);
    const int okW   = S(240);
    const int btnH  = S(56);

    COLORREF accent = (playerNo == 1) ? RGB(238, 21, 21) : RGB(52, 120, 220);
    TCHAR title[64];
    TCHAR subtitle[64];
    if (playerNo == 1) {
        _tcscpy_s(title, _T("CHOOSE YOUR PARTNER"));
        _tcscpy_s(subtitle, _T("Trainer Registration"));
    } else {
        _tcscpy_s(title, _T("RIVAL TRAINER"));
        _tcscpy_s(subtitle, _T("Select opponent partner"));
    }

    while (true) {
        MenuRect panel = CenterPanel(1040, 680, 0);
        int cardW = S(280);
        int cardH = S(460);
        int cardX = panel.x + S(28);
        int cardY = panel.y + S(88);

        int gridX = panel.x + S(330);
        int gridY = panel.y + S(176);
        int nameX = gridX;
        int nameY = panel.y + S(118);
        int nameW = panel.w - S(360);
        int nameH = S(44);

        int btnRowW = backW + okW + S(36);
        int backX = panel.x + panel.w - btnRowW - S(28);
        int okX   = backX + backW + S(36);
        int btnY  = panel.y + panel.h - S(72);

        while (peekmessage(&msg, EM_MOUSE | EM_KEY | EM_CHAR | EM_WINDOW)) {
            if (msg.message == WM_MOUSEMOVE) { mx = msg.x; my = msg.y; }
            if (!IsWindow(GetHWnd())) exit(0);

            if (msg.message == WM_LBUTTONDOWN) {
                if (PointInRect(msg.x, msg.y, okX, btnY, okW, btnH)) {
                    PlayClickSound();
                    if (_tcslen(nameOut) == 0) {
                        if (playerNo == 1) _tcscpy_s(nameOut, nameCount, _T("Red"));
                        else _tcscpy_s(nameOut, nameCount, _T("Blue"));
                    }
                    *charIdOut = selected;
                    EndBatchDraw();
                    return true;
                }
                if (PointInRect(msg.x, msg.y, backX, btnY, backW, btnH)) {
                    PlayClickSound();
                    EndBatchDraw();
                    return false;
                }
                for (int id = 1; id <= CHAR_COUNT; id++) {
                    if (id == excludeId || !s_thumbLoaded[id]) continue;
                    int idx = id - 1;
                    int x = gridX + (idx % COLS) * (THUMB + GAP);
                    int y = gridY + (idx / COLS) * (THUMB + GAP);
                    if (PointInRect(msg.x, msg.y, x, y, THUMB, THUMB)) {
                        PlayClickSound();
                        selected = id;
                        break;
                    }
                }
            }
            if (msg.message == WM_KEYDOWN) {
                if (msg.vkcode == VK_LEFT) {
                    int next = selected;
                    do { next = (next - 2 + CHAR_COUNT) % CHAR_COUNT + 1; } while (next == excludeId || !s_thumbLoaded[next]);
                    selected = next;
                } else if (msg.vkcode == VK_RIGHT) {
                    int next = selected;
                    do { next = next % CHAR_COUNT + 1; } while (next == excludeId || !s_thumbLoaded[next]);
                    selected = next;
                } else if (msg.vkcode == VK_UP) {
                    int next = selected - COLS;
                    while (next >= 1 && (next == excludeId || !s_thumbLoaded[next])) next -= COLS;
                    if (next >= 1 && next <= CHAR_COUNT) selected = next;
                } else if (msg.vkcode == VK_DOWN) {
                    int next = selected + COLS;
                    while (next <= CHAR_COUNT && (next == excludeId || !s_thumbLoaded[next])) next += COLS;
                    if (next >= 1 && next <= CHAR_COUNT) selected = next;
                } else if (msg.vkcode == VK_BACK) {
                    size_t len = _tcslen(nameOut);
                    if (len > 0) nameOut[len - 1] = 0;
                } else if (msg.vkcode == VK_RETURN) {
                    PlayClickSound();
                    if (_tcslen(nameOut) == 0) {
                        if (playerNo == 1) _tcscpy_s(nameOut, nameCount, _T("Red"));
                        else _tcscpy_s(nameOut, nameCount, _T("Blue"));
                    }
                    *charIdOut = selected;
                    EndBatchDraw();
                    return true;
                } else if (msg.vkcode == VK_ESCAPE) {
                    PlayClickSound();
                    EndBatchDraw();
                    return false;
                }
            } else if (msg.message == WM_CHAR) {
                char c = msg.ch;
                if (c == 8) {
                    size_t len = _tcslen(nameOut);
                    if (len > 0) nameOut[len - 1] = 0;
                } else if (isprint((unsigned char)c) && c != '|') {
                    AppendTrainerChar(nameOut, nameCount, c);
                }
            }
        }

        if (!IsWindow(GetHWnd())) exit(0);

        if (!s_portraitLoaded[selected]) {
            TCHAR path[128];
            _stprintf_s(path, 128, _T("GUI\\Characters\\%d.png"), selected);
            loadimage(&s_portrait[selected], path);
            CharSpriteCleanupFringe(&s_portrait[selected]);
            s_portraitLoaded[selected] = true;
        }

        BeginBatchDraw();
        putimage(0, 0, imgMenuBg);
        DrawPokemonPanel(panel);

        DrawTitleCentered(panel.y + S(38), title, RGB(255, 255, 255), 20);

        ApplyGameFont(11, FW_BOLD);
        setbkmode(TRANSPARENT);
        settextcolor(RGB(255, 220, 160));
        outtextxy(gridX, panel.y + S(72), subtitle);

        DrawTrainerCard(cardX, cardY, cardW, cardH, selected, &s_portrait[selected], true, nameOut, accent);
        DrawNameField(nameX, nameY, nameW, nameH, _T("TRAINER NAME"), nameOut, true);

        ApplyGameFont(10, FW_BOLD);
        settextcolor(RGB(180, 190, 210));
        outtextxy(gridX, gridY - S(24), _T("PARTNER ROSTER"));

        for (int id = 1; id <= CHAR_COUNT; id++) {
            if (!s_thumbLoaded[id]) continue;
            int idx = id - 1;
            int x = gridX + (idx % COLS) * (THUMB + GAP);
            int y = gridY + (idx / COLS) * (THUMB + GAP);
            bool blocked = (id == excludeId);
            bool picked  = (id == selected);

            setfillcolor(blocked ? RGB(28, 30, 40) : RGB(18, 22, 36));
            fillroundrect(x, y, x + THUMB, y + THUMB, S(8), S(8));

            if (!blocked)
                DrawMenuSprite(x, y, THUMB, THUMB, &s_thumbs[id]);
            else {
                settextcolor(RGB(140, 140, 150));
                ApplyGameFont(10, FW_BOLD);
                setbkmode(TRANSPARENT);
                outtextxy(x + THUMB / 2 - S(12), y + THUMB / 2 - S(6), _T("X"));
            }

            if (picked)
                DrawSelectedRosterMarker(x, y, THUMB);
        }

        DrawWoodenButton(backX, btnY, backW, btnH, _T("BACK"),
            PointInRect(mx, my, backX, btnY, backW, btnH));
        DrawWoodenButton(okX, btnY, okW, btnH, _T("START BATTLE"),
            PointInRect(mx, my, okX, btnY, okW, btnH));

        FlushBatchDraw();
        Sleep(1);
    }
}

static bool ShowCpuOpponentReveal(int cpuId) {
    IMAGE* imgMenuBg = GetMenuBackground();
    static IMAGE s_portrait;
    static int s_lastId = -1;

    if (s_lastId != cpuId) {
        TCHAR path[128];
        _stprintf_s(path, 128, _T("GUI\\Characters\\%d.png"), cpuId);
        loadimage(&s_portrait, path);
        CharSpriteCleanupFringe(&s_portrait);
        s_lastId = cpuId;
    }

    ExMessage msg;
    const int btnW = S(260);
    const int btnH = S(56);

    while (true) {
        MenuRect panel = CenterPanel(560, 520, 0);
        int btnX = panel.x + (panel.w - btnW) / 2;
        int btnY = panel.y + panel.h - S(72);
        int cardW = S(280);
        int cardX = panel.x + (panel.w - cardW) / 2;

        if (peekmessage(&msg, EM_MOUSE | EM_KEY | EM_WINDOW)) {
            if (!IsWindow(GetHWnd())) exit(0);
            if (msg.message == WM_LBUTTONDOWN && PointInRect(msg.x, msg.y, btnX, btnY, btnW, btnH)) {
                PlayClickSound();
                return true;
            }
            if (msg.message == WM_KEYDOWN && (msg.vkcode == VK_RETURN || msg.vkcode == VK_SPACE)) {
                PlayClickSound();
                return true;
            }
            if (msg.message == WM_KEYDOWN && msg.vkcode == VK_ESCAPE)
                return false;
        }

        BeginBatchDraw();
        putimage(0, 0, imgMenuBg);
        DrawPokemonPanel(panel);
        DrawTitleCentered(panel.y + S(32), _T("WILD TRAINER APPEARED!"), RGB(255, 255, 255), 17);
        DrawTextCenteredIn(panel.x, panel.w, panel.y + S(62), _T("Gym Leader challenge incoming..."), RGB(255, 220, 160), 11);

        DrawTrainerCard(cardX, panel.y + S(88), cardW, S(340), cpuId, &s_portrait, true, _NAME_P2, RGB(52, 120, 220));

        DrawWoodenButton(btnX, btnY, btnW, btnH, _T("ACCEPT CHALLENGE"), false);
        FlushBatchDraw();
        Sleep(1);
    }
}

bool ShowCharacterSelectFlow(void) {
    if (!ShowTrainerSetup(1, 0, _NAME_P1, 32, &_CHAR_P1))
        return false;

    if (_VS_BOT) {
        int cpu;
        do {
            cpu = (rand() % CHAR_COUNT) + 1;
        } while (cpu == _CHAR_P1 || !CharacterImageExists(cpu));
        _CHAR_P2 = cpu;
        _tcscpy_s(_NAME_P2, 32, _T("Gym Leader"));
        if (!ShowCpuOpponentReveal(cpu))
            return false;
    } else {
        if (!ShowTrainerSetup(2, _CHAR_P1, _NAME_P2, 32, &_CHAR_P2))
            return false;
    }

    ReloadAvatars();
    return true;
}

int ShowMainMenu(void) {
    PlayMenuBGM();

    const int btnH    = S(58);
    const int btnGap  = S(18);
    const int startY  = ScreenH() * 38 / 100;

    MenuButton buttons[] = {
        { _T("GUI\\Background\\MENU_BUTTONS\\NEW GAME.png"),  1440, 349, 0, 0, 0, 0, {} },
        { _T("GUI\\Background\\MENU_BUTTONS\\LOAD GAME.png"), 1403, 338, 0, 0, 0, 0, {} },
        { _T("GUI\\Background\\MENU_BUTTONS\\SETTINGS.png"),  1383, 321, 0, 0, 0, 0, {} },
        { _T("GUI\\Background\\MENU_BUTTONS\\ABOUT US.png"),  1361, 323, 0, 0, 0, 0, {} },
        { _T("GUI\\Background\\MENU_BUTTONS\\HELP.png"),       511, 171, 0, 0, 0, 0, {} },
        { _T("GUI\\Background\\MENU_BUTTONS\\EXIT.png"),       516, 171, 0, 0, 0, 0, {} },
    };
    const int btnCount = (int)(sizeof(buttons) / sizeof(buttons[0]));

    IMAGE imgMenuBg;
    loadimage(&imgMenuBg, _T("GUI\\Background\\man_hinh_nen_menu.jpg"), ScreenW(), ScreenH());
    LayoutMenuButtons(buttons, btnCount, ScreenW(), startY, btnH, btnGap);
    LoadMenuButtonImages(buttons, btnCount);

    ExMessage msg;
    while (true) {
        BeginBatchDraw();
        putimage(0, 0, &imgMenuBg);
        for (int i = 0; i < btnCount; i++)
            DrawMenuButton(buttons[i]);
        FlushBatchDraw();

        if (!IsWindow(GetHWnd())) exit(0);

        if (peekmessage(&msg, EM_MOUSE | EM_WINDOW)) {
            if (msg.message == WM_LBUTTONDOWN) {
                int hit = HitMenuButton(buttons, btnCount, msg.x, msg.y);
                if (hit < 0) continue;

                PlayClickSound();
                DrawMenuButton(buttons[hit]);
                FlushBatchDraw();

                if (hit == 0) {
                    int mode = ShowNewGameModeMenu();
                    if (mode == 0)
                        continue;
                    _VS_BOT = (mode == 2) ? 1 : 0;
                    if (!ShowCharacterSelectFlow())
                        continue;
                    PlayGameBGM();
                    EndBatchDraw();
                    return 1;
                }
                if (hit == 1) {
                    if (ShowLoadMenuUI()) { EndBatchDraw(); return 2; }
                }
                if (hit == 2) {
                    ShowSettingsMenu();
                }
                if (hit == 3) {
                    ShowInfoMenu(_T("ABOUT US"), _T("Game: Tic-Tac-Toe in C++"), _T("Developer: Nhom 18"), _T("Version: 1.0 - 2026"));
                }
                if (hit == 4) {
                    ShowInfoMenu(_T("RULES"), _T("- Get 5 in a row to win."), _T("- New Game: choose PVP or PVE."), _T("PVE: you are X, computer is O."));
                }
                if (hit == 5) {
                    if (ShowConfirmDialog(_T("Exit Game"), _T("Are you sure you want to exit?")))
                        exit(0);
                }
            }
        }
        Sleep(1);
    }
}

// --- HÀM MỚI: BẢNG PAUSE KHI ĐANG CHƠI ---
int ShowPauseMenu(void) {
    ExMessage msg;
    int mx = 0, my = 0;

    const int btnW = S(240);
    const int btnH = S(58);
    const int btnGap = S(16);

    while (true) {
        MenuRect panel = CenterPanel(380, 360, 0);
        int btnX = panel.x + (panel.w - btnW) / 2;
        int resumeY = panel.y + S(90);
        int settingsY = resumeY + btnH + btnGap;
        int exitY = settingsY + btnH + btnGap;

        while (peekmessage(&msg, EM_MOUSE | EM_WINDOW)) {
            if (msg.message == WM_MOUSEMOVE) { mx = msg.x; my = msg.y; }
            if (!IsWindow(GetHWnd())) exit(0);
            if (msg.message == WM_LBUTTONDOWN) {
                if (PointInRect(msg.x, msg.y, btnX, resumeY, btnW, btnH)) {
                    PlayClickSound(); return 1;
                }
                else if (PointInRect(msg.x, msg.y, btnX, settingsY, btnW, btnH)) {
                    PlayClickSound(); return 2;
                }
                else if (PointInRect(msg.x, msg.y, btnX, exitY, btnW, btnH)) {
                    PlayClickSound();
                    if (ShowConfirmDialog(_T("Confirm Exit"), _T("Are you sure you want to exit?")))
                        return 3;
                }
            }
        }

        DrawMenuPanel(panel);
        DrawTitleCentered(panel.y + S(28), _T("GAME PAUSED"), WHITE, 16);

        DrawWoodenButton(btnX, resumeY, btnW, btnH, _T("RESUME"),
            PointInRect(mx, my, btnX, resumeY, btnW, btnH));
        DrawWoodenButton(btnX, settingsY, btnW, btnH, _T("SETTINGS"),
            PointInRect(mx, my, btnX, settingsY, btnW, btnH));
        DrawWoodenButton(btnX, exitY, btnW, btnH, _T("EXIT MATCH"),
            PointInRect(mx, my, btnX, exitY, btnW, btnH));

        FlushBatchDraw();
        Sleep(1);
    }
}

// --- HÀM TẠO BẢNG HỎI YES/NO BẰNG GỖ ---
bool ShowConfirmDialog(const TCHAR* title, const TCHAR* message) {
    IMAGE bgCopy;
    getimage(&bgCopy, 0, 0, ScreenW(), ScreenH());

    ExMessage msg;
    const int btnW = S(170);
    const int btnH = S(58);
    const int btnGap = S(36);
    int mx = 0, my = 0;

    while (true) {
        MenuRect box = CenterPanel(540, 240, 0);
        int btnRowW = btnW * 2 + btnGap;
        int yesX = box.x + (box.w - btnRowW) / 2;
        int noX  = yesX + btnW + btnGap;
        int btnY = box.y + box.h - S(68);

        if (peekmessage(&msg, EM_MOUSE | EM_WINDOW)) {
            if (msg.message == WM_MOUSEMOVE) { mx = msg.x; my = msg.y; }
            if (!IsWindow(GetHWnd())) exit(0);
            if (msg.message == WM_LBUTTONDOWN) {
                if (PointInRect(msg.x, msg.y, yesX, btnY, btnW, btnH)) {
                    PlayClickSound();
                    return true;
                }
                if (PointInRect(msg.x, msg.y, noX, btnY, btnW, btnH)) {
                    PlayClickSound();
                    return false;
                }
            }
        }

        BeginBatchDraw();
        putimage(0, 0, &bgCopy);
        DrawMenuPanel(box);
        DrawTextCenteredIn(box.x, box.w, box.y + S(28), title, RGB(255, 200, 100), 16);
        DrawTextCenteredIn(box.x, box.w, box.y + S(78), message, WHITE, 12);

        DrawWoodenButton(yesX, btnY, btnW, btnH, _T("YES"),
            PointInRect(mx, my, yesX, btnY, btnW, btnH));
        DrawWoodenButton(noX, btnY, btnW, btnH, _T("NO"),
            PointInRect(mx, my, noX, btnY, btnW, btnH));

        FlushBatchDraw();
        Sleep(1);
    }
}

// --- HÀM TẠO BẢNG THÔNG BÁO (CHỈ CÓ NÚT OK) BẰNG GỖ ---
void ShowNotifyDialog(const TCHAR* title, const TCHAR* message) {
    IMAGE bgCopy;
    getimage(&bgCopy, 0, 0, ScreenW(), ScreenH());

    ExMessage msg;
    const int btnW = S(200);
    const int btnH = S(58);
    int mx = 0, my = 0;

    while (true) {
        MenuRect box = CenterPanel(540, 240, 0);
        int btnX = box.x + (box.w - btnW) / 2;
        int btnY = box.y + box.h - S(68);

        if (peekmessage(&msg, EM_MOUSE | EM_WINDOW)) {
            if (msg.message == WM_MOUSEMOVE) { mx = msg.x; my = msg.y; }
            if (!IsWindow(GetHWnd())) exit(0);
            if (msg.message == WM_LBUTTONDOWN) {
                if (PointInRect(msg.x, msg.y, btnX, btnY, btnW, btnH)) {
                    PlayClickSound();
                    return;
                }
            }
        }

        BeginBatchDraw();
        putimage(0, 0, &bgCopy);
        DrawMenuPanel(box);
        DrawTextCenteredIn(box.x, box.w, box.y + S(28), title, RGB(255, 200, 100), 16);
        DrawTextCenteredIn(box.x, box.w, box.y + S(78), message, WHITE, 12);
        DrawWoodenButton(btnX, btnY, btnW, btnH, _T("OK"),
            PointInRect(mx, my, btnX, btnY, btnW, btnH));

        FlushBatchDraw();
        Sleep(1);
    }
}