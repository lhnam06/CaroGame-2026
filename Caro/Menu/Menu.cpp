#include "Defs/Defs.h"
#include "View/View.h"
#include "Menu/Menu.h"
#include "Audio/Audio.h"
#include <graphics.h>
#include <tchar.h>
#include <cstdio>

#include <filesystem>
#include <vector>
#include <string>
#include <chrono>
#include <ctime>

namespace fs = std::filesystem;

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

void DrawTextWithShadow(int x, int y, const TCHAR* text, COLORREF textColor, int fontSize = 24) {
    setbkmode(TRANSPARENT);
    settextstyle(fontSize, 0, _T("Segoe UI"), 0, 0, FW_BOLD, false, false, false);
    settextcolor(RGB(18, 20, 32));
    outtextxy(x + 2, y + 2, text);
    settextcolor(textColor);
    outtextxy(x, y, text);
}

void DrawWoodenButton(int x, int y, int width, int height, const TCHAR* text, bool hovered) {
    // Outer shadow
    setlinecolor(RGB(24, 28, 40));
    setlinestyle(PS_SOLID, 2);
    setfillcolor(hovered ? RGB(52, 60, 88) : RGB(42, 48, 68));
    fillroundrect(x, y, x + width, y + height, 14, 14);

    // Border — brighter when hovered
    setlinecolor(hovered ? RGB(140, 180, 255) : RGB(90, 100, 135));
    setlinestyle(PS_SOLID, hovered ? 2 : 1);
    roundrect(x, y, x + width, y + height, 14, 14);

    // Inner fill
    setfillcolor(hovered ? RGB(65, 75, 108) : RGB(55, 62, 88));
    fillroundrect(x + 4, y + 4, x + width - 4, y + height - 4, 11, 11);

    // Top shimmer line
    setlinecolor(hovered ? RGB(160, 185, 220) : RGB(120, 135, 175));
    setlinestyle(PS_SOLID, 1);
    line(x + 8, y + 6, x + width - 8, y + 6);

    settextstyle(24, 0, _T("Segoe UI"), 0, 0, FW_BOLD, false, false, false);
    int textX = x + width / 2 - textwidth(text) / 2;
    int textY = y + height / 2 - 12;
    DrawTextWithShadow(textX, textY, text, hovered ? RGB(255, 255, 255) : RGB(240, 242, 250), 24);
}

// Biến toàn cục để lưu tên file người chơi đã chọn
std::string _SELECTED_FILE = "";

// Hàm vẽ chữ có bóng

// --- HÀM MỚI: Bảng thông tin chung cho ABOUT US và HELP ---
void ShowInfoMenu(const TCHAR* title, const TCHAR* line1, const TCHAR* line2, const TCHAR* line3) {
    IMAGE imgMenuBg; loadimage(&imgMenuBg, _T("menu_bg.jpg"), 800, 600);
    ExMessage msg;
    while (true) {
        BeginBatchDraw();
        putimage(0, 0, &imgMenuBg);
        
        // Vẽ Bảng gỗ nền
        setfillcolor(RGB(36, 42, 60)); fillroundrect(150, 100, 650, 500, 20, 20);
        setfillcolor(RGB(52, 58, 82)); fillroundrect(160, 110, 640, 490, 15, 15);

        // Chữ
        DrawTextWithShadow(250, 140, title, WHITE, 40);
        DrawTextWithShadow(200, 220, line1, RGB(255, 200, 100), 28);
        DrawTextWithShadow(200, 280, line2, WHITE, 24);
        DrawTextWithShadow(200, 330, line3, WHITE, 24);

        // Nút BACK
        setfillcolor(RGB(44, 50, 70)); fillroundrect(325, 410, 475, 460, 10, 10);
        DrawTextWithShadow(365, 420, _T("BACK"), WHITE, 24);

        FlushBatchDraw();
        if (!IsWindow(GetHWnd())) exit(0);

        if (peekmessage(&msg, EM_MOUSE | EM_WINDOW)) {
            if (msg.message == WM_LBUTTONDOWN) {
                // Click nút BACK
                if (msg.x >= 325 && msg.x <= 475 && msg.y >= 410 && msg.y <= 460) 
                {
                    PlayClickSound();
                    return;
                }
            }
        }
        Sleep(10);
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
    IMAGE imgMenuBg; loadimage(&imgMenuBg, _T("menu_bg.jpg"), 800, 600);
    ExMessage msg;

    while (true) {
        std::vector<LoadFileEntry> filteredFiles;
        for (const auto& f : allFiles) {
            if (searchQuery.empty() || f.name.find(searchQuery) != std::string::npos) {
                filteredFiles.push_back(f);
            }
        }
        if (selectedIndex >= (int)filteredFiles.size()) selectedIndex = max(0, (int)filteredFiles.size() - 1);

        BeginBatchDraw();
        putimage(0, 0, &imgMenuBg);

        // Bảng gỗ siêu to
        setfillcolor(RGB(36, 42, 60)); fillroundrect(50, 50, 750, 550, 20, 20);
        setfillcolor(RGB(52, 58, 82)); fillroundrect(60, 60, 740, 540, 15, 15);
        DrawTextWithShadow(260, 70, _T("--- LOAD GAME ---"), WHITE, 40);

        // Bên Phải: Giao diện Search
        setfillcolor(RGB(44, 50, 70)); fillroundrect(450, 150, 700, 200, 10, 10);
        DrawTextWithShadow(460, 165, _T("Search:"), RGB(200, 200, 200), 20);
        TCHAR tSearch[256];
        _stprintf_s(tSearch, 256, _T("%hs"), searchQuery.c_str());
        DrawTextWithShadow(540, 165, tSearch, WHITE, 20);
        DrawTextWithShadow(450, 210, _T("(Type on your keyboard)"), RGB(255, 200, 100), 16);

        // Bên Trái: Danh sách file
        int listY = 150;
        if (filteredFiles.empty()) {
            DrawTextWithShadow(100, listY, _T("No files found."), RGB(200, 200, 200), 20);
        } else {
            for (int i = 0; i < (int)filteredFiles.size() && i < 8; i++) {
                std::string line = filteredFiles[i].name + "  |  " + FileMTimeShort(filteredFiles[i].path);
                if (line.size() > 54) line = line.substr(0, 51) + "...";
                TCHAR tFile[320];
                _stprintf_s(tFile, _T("%hs"), line.c_str());
                if (i == selectedIndex) {
                    setfillcolor(RGB(65, 85, 140));
                    fillroundrect(90, listY - 5, 400, listY + 30, 5, 5);
                }
                DrawTextWithShadow(100, listY, tFile, WHITE, 18);
                listY += 40;
            }
        }

        // Nút bấm dưới cùng
        setfillcolor(RGB(44, 50, 70)); fillroundrect(150, 480, 300, 530, 10, 10);
        DrawTextWithShadow(190, 495, _T("LOAD"), WHITE, 24);
        setfillcolor(RGB(44, 50, 70)); fillroundrect(500, 480, 650, 530, 10, 10);
        DrawTextWithShadow(540, 495, _T("BACK"), WHITE, 24);

        FlushBatchDraw();
        if (!IsWindow(GetHWnd())) exit(0);

        while (peekmessage(&msg, EM_MOUSE | EM_KEY | EM_CHAR | EM_WINDOW)) {
            if (msg.message == WM_LBUTTONDOWN) {
                int mx = msg.x, my = msg.y;
                if (mx >= 500 && mx <= 650 && my >= 480 && my <= 530) {
                    PlayClickSound();
                    return false; // Bấm BACK
                }
                
                if (mx >= 150 && mx <= 300 && my >= 480 && my <= 530) {
                    if (!filteredFiles.empty()) {
                        _SELECTED_FILE = filteredFiles[selectedIndex].name;
                        return true; // Bấm LOAD
                    }
                }
                // Click chọn file
                if (mx >= 90 && mx <= 400 && my >= 145 && my <= 145 + 8 * 40) {
                    int clickedIdx = (my - 145) / 40;
                    if (clickedIdx < filteredFiles.size()) selectedIndex = clickedIdx;
                }
            }
            else if (msg.message == WM_KEYDOWN) {
                if (msg.vkcode == VK_UP) selectedIndex = max(0, selectedIndex - 1);
                if (msg.vkcode == VK_DOWN) selectedIndex = min((int)filteredFiles.size() - 1, selectedIndex + 1);
                if (msg.vkcode == VK_BACK && !searchQuery.empty()) searchQuery.pop_back(); // Nút xóa
                if (msg.vkcode == VK_RETURN && !filteredFiles.empty()) {
                    _SELECTED_FILE = filteredFiles[selectedIndex].name;
                    return true;
                }
            }
            else if (msg.message == WM_CHAR) {
                char c = msg.ch; // Nhận chữ gõ vào
                if (isalnum(c) || c == '_' || c == '-' || c == '.') searchQuery += c;
            }
        }
        Sleep(10);
    }
}

static const TCHAR* ThemeName(int t) {
    if (t == 1) return _T("Dark Slate");
    if (t == 2) return _T("Golden");
    return _T("Warm Wood");
}

void ShowSettingsMenu(void) {
    IMAGE imgMenuBg;
    loadimage(&imgMenuBg, _T("menu_bg.jpg"), 800, 600);

    ExMessage msg;
    int mx = 0, my = 0;

    while (true) {
        if (peekmessage(&msg, EM_MOUSE | EM_WINDOW)) {
            if (msg.message == WM_MOUSEMOVE) { mx = msg.x; my = msg.y; }
            if (!IsWindow(GetHWnd())) exit(0);

            if (msg.message == WM_LBUTTONDOWN) {
                // Volume DOWN
                if (msg.x >= 200 && msg.x <= 350 && msg.y >= 280 && msg.y <= 330) {
                    PlayClickSound();
                    SetVolume(GetVolume() - 100);
                }
                // Volume UP
                else if (msg.x >= 450 && msg.x <= 600 && msg.y >= 280 && msg.y <= 330) {
                    PlayClickSound();
                    SetVolume(GetVolume() + 100);
                }
                // Theme PREV
                else if (msg.x >= 200 && msg.x <= 350 && msg.y >= 360 && msg.y <= 410) {
                    PlayClickSound();
                    _BOARD_THEME = (_BOARD_THEME + 2) % 3;
                }
                // Theme NEXT
                else if (msg.x >= 450 && msg.x <= 600 && msg.y >= 360 && msg.y <= 410) {
                    PlayClickSound();
                    _BOARD_THEME = (_BOARD_THEME + 1) % 3;
                }
                // BACK
                else if (msg.x >= 325 && msg.x <= 475 && msg.y >= 440 && msg.y <= 490) {
                    PlayClickSound();
                    EndBatchDraw();
                    return;
                }
            }
        }

        if (!IsWindow(GetHWnd())) exit(0);

        BeginBatchDraw();
        putimage(0, 0, &imgMenuBg);

        int boardX = 150, boardY = 80, boardW = 500, boardH = 440;
        setfillcolor(RGB(36, 42, 60));
        fillroundrect(boardX, boardY, boardX + boardW, boardY + boardH, 20, 20);
        setfillcolor(RGB(52, 58, 82));
        fillroundrect(boardX + 10, boardY + 10, boardX + boardW - 10, boardY + boardH - 10, 15, 15);

        DrawTextWithShadow(270, 105, _T("--- SETTINGS ---"), WHITE, 40);

        // Volume row
        TCHAR volText[50];
        _stprintf_s(volText, 50, _T("VOLUME: %d %%"), GetVolume() / 10);
        DrawTextWithShadow(310, 215, volText, RGB(255, 200, 100), 28);
        DrawWoodenButton(200, 280, 150, 50, _T("- DOWN"), mx >= 200 && mx <= 350 && my >= 280 && my <= 330);
        DrawWoodenButton(450, 280, 150, 50, _T("+ UP"),   mx >= 450 && mx <= 600 && my >= 280 && my <= 330);

        // Separator
        setlinecolor(RGB(70, 78, 108));
        setlinestyle(PS_SOLID, 1);
        line(boardX + 20, 350, boardX + boardW - 20, 350);

        // Theme row
        TCHAR themeText[60];
        _stprintf_s(themeText, 60, _T("THEME: %s"), ThemeName(_BOARD_THEME));
        DrawTextWithShadow(290, 363, themeText, RGB(255, 200, 100), 22);
        DrawWoodenButton(200, 360, 80, 44, _T("< "),      mx >= 200 && mx <= 280 && my >= 360 && my <= 404);
        DrawWoodenButton(520, 360, 80, 44, _T(" >"),      mx >= 520 && mx <= 600 && my >= 360 && my <= 404);

        // BACK
        DrawWoodenButton(325, 440, 150, 50, _T("BACK"), mx >= 325 && mx <= 475 && my >= 440 && my <= 490);

        FlushBatchDraw();
        Sleep(10);
    }
}

/*
 * Chọn PVP (2 người) hoặc PVE (vs máy). Trả về: 0 = BACK, 1 = PVP, 2 = PVE
 */
int ShowNewGameModeMenu(void) {
    IMAGE imgMenuBg;
    loadimage(&imgMenuBg, _T("menu_bg.jpg"), 800, 600);
    ExMessage msg;
    int mx = 0, my = 0;

    while (true) {
        while (peekmessage(&msg, EM_MOUSE | EM_KEY | EM_WINDOW)) {
            if (msg.message == WM_MOUSEMOVE) { mx = msg.x; my = msg.y; }
            if (!IsWindow(GetHWnd())) exit(0);
            if (msg.message == WM_LBUTTONDOWN) {
                if (msg.x >= 250 && msg.x <= 550 && msg.y >= 245 && msg.y <= 301) {
                    PlayClickSound(); EndBatchDraw(); return 1;
                }
                if (msg.x >= 250 && msg.x <= 550 && msg.y >= 318 && msg.y <= 374) {
                    PlayClickSound(); EndBatchDraw(); return 2;
                }
                if (msg.x >= 300 && msg.x <= 500 && msg.y >= 410 && msg.y <= 460) {
                    PlayClickSound(); EndBatchDraw(); return 0;
                }
            }
            if (msg.message == WM_KEYDOWN && msg.vkcode == VK_ESCAPE) {
                PlayClickSound(); EndBatchDraw(); return 0;
            }
        }
        if (!IsWindow(GetHWnd())) exit(0);

        BeginBatchDraw();
        putimage(0, 0, &imgMenuBg);

        int bx = 150, by = 120, bw = 500, bh = 380;
        setfillcolor(RGB(36, 42, 60));
        fillroundrect(bx, by, bx + bw, by + bh, 22, 22);
        setfillcolor(RGB(52, 58, 82));
        fillroundrect(bx + 12, by + 12, bx + bw - 12, by + bh - 12, 16, 16);

        DrawTextWithShadow(240, 150, _T("NEW GAME"), WHITE, 40);
        DrawTextWithShadow(200, 200, _T("Choose game mode:"), RGB(255, 220, 180), 22);

        DrawWoodenButton(250, 245, 300, 56, _T("PVP (2 players)"),   mx >= 250 && mx <= 550 && my >= 245 && my <= 301);
        DrawWoodenButton(250, 318, 300, 56, _T("PVE (vs computer)"), mx >= 250 && mx <= 550 && my >= 318 && my <= 374);
        DrawWoodenButton(300, 410, 200, 50, _T("BACK"),              mx >= 300 && mx <= 500 && my >= 410 && my <= 460);

        FlushBatchDraw();
        Sleep(10);
    }
}

int ShowMainMenu(void) {
    PlayBGM(_T("Audio\\bgm.mp3")); // Bật nhạc Menu
    IMAGE imgMenuBg, imgNew, imgLoad, imgSettings, imgAbout, imgHelp, imgExit; 
    loadimage(&imgMenuBg, _T("UI_PNG\\menu_bgm.jpg"), 800, 600); 
    loadimage(&imgNew, _T("UI_PNG\\new.jpg"), 210, 53); 
    loadimage(&imgLoad, _T("UI_PNG\\load.jpg"), 210, 53); 
    loadimage(&imgSettings, _T("UI_PNG\\settings.jpg"), 210, 53); 
    loadimage(&imgAbout, _T("UI_PNG\\about.jpg"), 210, 53); 
    loadimage(&imgHelp, _T("UI_PNG\\help.jpg"), 210, 53); 
    loadimage(&imgExit, _T("UI_PNG\\exit.jpg"), 210, 53); 

    ExMessage msg;
    while (true) {
        BeginBatchDraw();
        putimage(0, 0, &imgMenuBg);
        FlushBatchDraw();

        if (!IsWindow(GetHWnd())) exit(0); 

        if (peekmessage(&msg, EM_MOUSE | EM_WINDOW)) {
            if (msg.message == WM_LBUTTONDOWN) {
                int mx = msg.x, my = msg.y;

                // 1. NEW GAME -> chọn PVP hoặc PVE
                if (mx >= 300 && mx <= 510 && my >= 159 && my <= 212) { 
                    PlayClickSound();
                    putimage(300, 159, &imgNew); FlushBatchDraw(); Sleep(200);
                    int mode = ShowNewGameModeMenu();
                    if (mode == 0)
                        continue;
                    _VS_BOT = (mode == 2) ? 1 : 0;
                    PlayBGM(_T("Audio\\game_bgm.mp3"));
                    EndBatchDraw();
                    return 1;
                }
                
                // 2. LOAD GAME
                if (mx >= 300 && mx <= 510 && my >= 220 && my <= 273) { 
                    PlayClickSound();
                    putimage(300, 219, &imgLoad); FlushBatchDraw(); Sleep(300);
                    // Mở giao diện Load, nếu chọn file thành công thì trả về 2
                    if (ShowLoadMenuUI()) { EndBatchDraw(); return 2; }
                }
                
                // 3. SETTINGS
                if (mx >= 300 && mx <= 510 && my >= 284 && my <= 337) { 
                    PlayClickSound();
                    putimage(300, 279, &imgSettings); FlushBatchDraw(); Sleep(300); ShowSettingsMenu(); 
                }

                // 4. ABOUT US
                if (mx >= 300 && mx <= 510 && my >= 342 && my <= 395) { 
                    PlayClickSound();
                    putimage(300, 338, &imgAbout); FlushBatchDraw(); Sleep(300);
                    ShowInfoMenu(_T("   --- ABOUT US ---"), _T("Game: Tic-Tac-Toe in C++"), _T("Developer: Nhom 18"), _T("Version: 1.0 - 2026"));
                }

                // 5. HELP
                if (mx >= 300 && mx <= 510 && my >= 403 && my <= 456) { 
                    PlayClickSound();
                    putimage(300, 398, &imgHelp); FlushBatchDraw(); Sleep(300);
                    ShowInfoMenu(_T("       --- RULES ---"), _T("- Get 5 in a row to win."), _T("- New Game: choose PVP or PVE."), _T("  PVE: you are X, computer is O."));
                }
                
                // 6. EXIT
                if (mx >= 300 && mx <= 510 && my >= 461 && my <= 514) {
                    PlayClickSound();
                    putimage(300, 457, &imgExit); FlushBatchDraw(); Sleep(200);
                    if (ShowConfirmDialog(_T("Exit Game"), _T("Are you sure you want to exit?"))) {
                        exit(0);
                    }
                }
            }
        }
        Sleep(10);
    }
}

// --- HÀM MỚI: BẢNG PAUSE KHI ĐANG CHƠI ---
int ShowPauseMenu(void) {
    ExMessage msg;
    int mx = 0, my = 0;
    while (true) {
        while (peekmessage(&msg, EM_MOUSE | EM_WINDOW)) {
            if (msg.message == WM_MOUSEMOVE) { mx = msg.x; my = msg.y; }
            if (!IsWindow(GetHWnd())) exit(0);
            if (msg.message == WM_LBUTTONDOWN) {
                if (msg.x >= 300 && msg.x <= 500 && msg.y >= 180 && msg.y <= 230) {
                    PlayClickSound(); return 1;
                }
                else if (msg.x >= 300 && msg.x <= 500 && msg.y >= 250 && msg.y <= 300) {
                    PlayClickSound(); return 2;
                }
                else if (msg.x >= 300 && msg.x <= 500 && msg.y >= 320 && msg.y <= 370) {
                    PlayClickSound();
                    if (ShowConfirmDialog(_T("Confirm Exit"), _T("Are you sure you want to exit?")))
                        return 3;
                }
            }
        }

        int boardX = 250, boardY = 100, boardW = 300, boardH = 350;
        setlinecolor(RGB(70, 78, 108)); setlinestyle(PS_SOLID, 2);
        setfillcolor(RGB(36, 42, 60)); fillroundrect(boardX, boardY, boardX + boardW, boardY + boardH, 20, 20);
        setfillcolor(RGB(52, 58, 82)); fillroundrect(boardX + 10, boardY + 10, boardX + boardW - 10, boardY + boardH - 10, 15, 15);

        DrawTextWithShadow(280, 120, _T("--- GAME PAUSED ---"), WHITE, 28);

        DrawWoodenButton(300, 180, 200, 50, _T("RESUME"),     mx >= 300 && mx <= 500 && my >= 180 && my <= 230);
        DrawWoodenButton(300, 250, 200, 50, _T("SETTINGS"),   mx >= 300 && mx <= 500 && my >= 250 && my <= 300);
        DrawWoodenButton(300, 320, 200, 50, _T("EXIT MATCH"), mx >= 300 && mx <= 500 && my >= 320 && my <= 370);

        FlushBatchDraw();
        Sleep(10);
    }
}

// --- HÀM TẠO BẢNG HỎI YES/NO BẰNG GỖ ---
bool ShowConfirmDialog(const TCHAR* title, const TCHAR* message) {
    IMAGE bgCopy;
    getimage(&bgCopy, 0, 0, 800, 600); // Chụp lại màn hình hiện tại để làm nền

    ExMessage msg;
    while (true) {
        BeginBatchDraw();
        putimage(0, 0, &bgCopy); // Dán nền cũ ra

        // Vẽ bảng gỗ nhỏ ở giữa màn hình
        int boxX = 150, boxY = 200, boxW = 500, boxH = 200;
        setlinecolor(RGB(70, 78, 108)); setlinestyle(PS_SOLID, 2);
        setfillcolor(RGB(36, 42, 60)); fillroundrect(boxX, boxY, boxX + boxW, boxY + boxH, 15, 15);
        setfillcolor(RGB(52, 58, 82)); fillroundrect(boxX + 5, boxY + 5, boxX + boxW - 5, boxY + boxH - 5, 10, 10);

        // In chữ
        DrawTextWithShadow(boxX + boxW / 2 - textwidth(title) / 2, boxY + 20, title, RGB(255, 200, 100), 28);
        DrawTextWithShadow(boxX + boxW / 2 - textwidth(message) / 2, boxY + 70, message, WHITE, 20);

        // Vẽ 2 nút YES / NO
        DrawWoodenButton(200, 320, 150, 50, _T("YES"));
        DrawWoodenButton(450, 320, 150, 50, _T("NO"));

        FlushBatchDraw();
        if (!IsWindow(GetHWnd())) exit(0);

        if (peekmessage(&msg, EM_MOUSE | EM_WINDOW)) {
            if (msg.message == WM_LBUTTONDOWN) {
                int mx = msg.x, my = msg.y;
                if (mx >= 200 && mx <= 350 && my >= 320 && my <= 370) {
                    PlayClickSound();
                    return true; // Bấm YES
                }
                if (mx >= 450 && mx <= 600 && my >= 320 && my <= 370) {
                    PlayClickSound();
                    return false; // Bấm NO
                }
            }
        }
        Sleep(10);
    }
}

// --- HÀM TẠO BẢNG THÔNG BÁO (CHỈ CÓ NÚT OK) BẰNG GỖ ---
void ShowNotifyDialog(const TCHAR* title, const TCHAR* message) {
    IMAGE bgCopy;
    getimage(&bgCopy, 0, 0, 800, 600);

    ExMessage msg;
    while (true) {
        BeginBatchDraw();
        putimage(0, 0, &bgCopy);

        int boxX = 150, boxY = 200, boxW = 500, boxH = 200;
        setlinecolor(RGB(70, 78, 108)); setlinestyle(PS_SOLID, 2);
        setfillcolor(RGB(36, 42, 60)); fillroundrect(boxX, boxY, boxX + boxW, boxY + boxH, 15, 15);
        setfillcolor(RGB(52, 58, 82)); fillroundrect(boxX + 5, boxY + 5, boxX + boxW - 5, boxY + boxH - 5, 10, 10);

        DrawTextWithShadow(boxX + boxW / 2 - textwidth(title) / 2, boxY + 20, title, RGB(255, 200, 100), 28);
        DrawTextWithShadow(boxX + boxW / 2 - textwidth(message) / 2, boxY + 70, message, WHITE, 20);

        // Chỉ vẽ 1 nút OK ở giữa
        DrawWoodenButton(325, 320, 150, 50, _T("OK"));

        FlushBatchDraw();
        if (!IsWindow(GetHWnd())) exit(0);

        if (peekmessage(&msg, EM_MOUSE | EM_WINDOW)) {
            if (msg.message == WM_LBUTTONDOWN) {
                int mx = msg.x, my = msg.y;
                if (mx >= 325 && mx <= 475 && my >= 320 && my <= 370) {
                    PlayClickSound();
                    return; // Thoát bảng
                }
            }
        }
        Sleep(10);
    }
}