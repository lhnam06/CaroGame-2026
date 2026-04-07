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


void DrawTextWithShadow(int x, int y, const TCHAR* text, COLORREF textColor, int fontSize = 24) {
    setbkmode(TRANSPARENT);
    settextstyle(fontSize, 0, _T("Segoe UI"), 0, 0, FW_BOLD, false, false, false);
    settextcolor(BLACK); outtextxy(x + 3, y + 3, text);
    settextcolor(textColor); outtextxy(x, y, text);
}

void DrawWoodenButton(int x, int y, int width, int height, const TCHAR* text) {
    // 1. Viền ngoài (Màu nâu đen) bo góc
    setlinecolor(BLACK);
    setlinestyle(PS_SOLID, 2);
    setfillcolor(RGB(50, 30, 20)); // Nâu tối
    fillroundrect(x, y, x + width, y + height, 15, 15); // Bo tròn 15 pixel
    
    // 2. Lõi trong (Màu nâu sáng hơn) bo góc
    setfillcolor(RGB(120, 70, 40)); // Nâu gỗ
    fillroundrect(x + 5, y + 5, x + width - 5, y + height - 5, 10, 10);
    
    // 3. Vẽ chữ trắng có bóng đen ở giữa nút
    settextstyle(24, 0, _T("Segoe UI"), 0, 0, FW_BOLD, false, false, false);
    int textX = x + width / 2 - textwidth(text) / 2;
    int textY = y + height / 2 - 12;
    DrawTextWithShadow(textX, textY, text, WHITE, 24);
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
        setfillcolor(RGB(60, 35, 20)); fillroundrect(150, 100, 650, 500, 20, 20);
        setfillcolor(RGB(100, 60, 35)); fillroundrect(160, 110, 640, 490, 15, 15);

        // Chữ
        DrawTextWithShadow(250, 140, title, WHITE, 40);
        DrawTextWithShadow(200, 220, line1, RGB(255, 200, 100), 28);
        DrawTextWithShadow(200, 280, line2, WHITE, 24);
        DrawTextWithShadow(200, 330, line3, WHITE, 24);

        // Nút BACK
        setfillcolor(RGB(50, 30, 20)); fillroundrect(325, 410, 475, 460, 10, 10);
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
    std::vector<std::string> allFiles;
    // Quét tìm tất cả file .txt trong folder game
    for (const auto& entry : std::filesystem::directory_iterator(".")) {
        if (entry.path().extension() == ".txt") {
            allFiles.push_back(entry.path().filename().string());
        }
    }

    std::string searchQuery = "";
    int selectedIndex = 0;
    IMAGE imgMenuBg; loadimage(&imgMenuBg, _T("menu_bg.jpg"), 800, 600);
    ExMessage msg;

    while (true) {
        // Lọc danh sách file theo chuỗi Search
        std::vector<std::string> filteredFiles;
        for (const auto& f : allFiles) {
            if (searchQuery.empty() || f.find(searchQuery) != std::string::npos) {
                filteredFiles.push_back(f);
            }
        }
        if (selectedIndex >= filteredFiles.size()) selectedIndex = max(0, (int)filteredFiles.size() - 1);

        BeginBatchDraw();
        putimage(0, 0, &imgMenuBg);

        // Bảng gỗ siêu to
        setfillcolor(RGB(60, 35, 20)); fillroundrect(50, 50, 750, 550, 20, 20);
        setfillcolor(RGB(100, 60, 35)); fillroundrect(60, 60, 740, 540, 15, 15);
        DrawTextWithShadow(260, 70, _T("--- LOAD GAME ---"), WHITE, 40);

        // Bên Phải: Giao diện Search
        setfillcolor(RGB(50, 30, 20)); fillroundrect(450, 150, 700, 200, 10, 10);
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
            for (int i = 0; i < filteredFiles.size() && i < 8; i++) {
                TCHAR tFile[256];
                _stprintf_s(tFile, 256, _T("%hs"), filteredFiles[i].c_str());
                if (i == selectedIndex) {
                    setfillcolor(RGB(150, 80, 40)); // Đổi màu khi được chọn
                    fillroundrect(90, listY - 5, 400, listY + 30, 5, 5);
                }
                DrawTextWithShadow(100, listY, tFile, WHITE, 20);
                listY += 40;
            }
        }

        // Nút bấm dưới cùng
        setfillcolor(RGB(50, 30, 20)); fillroundrect(150, 480, 300, 530, 10, 10);
        DrawTextWithShadow(190, 495, _T("LOAD"), WHITE, 24);
        setfillcolor(RGB(50, 30, 20)); fillroundrect(500, 480, 650, 530, 10, 10);
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
                        _SELECTED_FILE = filteredFiles[selectedIndex];
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
                    _SELECTED_FILE = filteredFiles[selectedIndex];
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

void ShowSettingsMenu(void) {
    IMAGE imgMenuBg;
    loadimage(&imgMenuBg, _T("menu_bg.jpg"), 800, 600); // Dùng chung ảnh phong cảnh

    ExMessage msg;
    while (true) {
        BeginBatchDraw();
        
        // 1. Dán ảnh phong cảnh làm nền
        putimage(0, 0, &imgMenuBg);

        // 2. Vẽ "Bảng gỗ" lót phía sau (Làm to hơn và nằm ngang để chứa các nút)
        int boardX = 150, boardY = 100, boardW = 500, boardH = 400;
        setfillcolor(RGB(60, 35, 20)); // Nâu tối
        fillroundrect(boardX, boardY, boardX + boardW, boardY + boardH, 20, 20);
        setfillcolor(RGB(100, 60, 35)); // Nâu bảng
        fillroundrect(boardX + 10, boardY + 10, boardX + boardW - 10, boardY + boardH - 10, 15, 15);

        // 3. Vẽ Tiêu đề có bóng râm
        DrawTextWithShadow(280, 130, _T("--- SETTINGS ---"), WHITE, 40);

        // 4. Hiển thị âm lượng (Dùng chữ vàng cam cho nổi)
        TCHAR volText[50];
        _stprintf_s(volText, 50, _T("VOLUME: %d %%"), GetVolume() / 10);
        DrawTextWithShadow(320, 220, volText, RGB(255, 200, 100), 30);

        // 5. Các nút chỉnh âm bằng GỖ
        DrawWoodenButton(200, 300, 150, 50, _T("- DOWN"));
        DrawWoodenButton(450, 300, 150, 50, _T("+ UP"));
        DrawWoodenButton(325, 400, 150, 50, _T("BACK"));

        FlushBatchDraw();

        // Kiểm tra thoát bằng nút (X)
        if (!IsWindow(GetHWnd())) {
            exit(0); 
        }

        // Bắt sự kiện chuột
        DrawWoodenButton(200, 300, 150, 50, _T("- DOWN"));
        DrawWoodenButton(450, 300, 150, 50, _T("+ UP"));
        DrawWoodenButton(325, 400, 150, 50, _T("BACK"));

        FlushBatchDraw();

        if (!IsWindow(GetHWnd())) exit(0); 

        if (peekmessage(&msg, EM_MOUSE | EM_WINDOW)) {
            if (msg.message == WM_LBUTTONDOWN) {
                int mx = msg.x, my = msg.y;
                
                // Nút Giảm: Lấy âm lượng hiện tại trừ đi 100
                if (mx >= 200 && mx <= 350 && my >= 300 && my <= 350) {
                    SetVolume(GetVolume() - 100); 
                }
                // Nút Tăng: Lấy âm lượng hiện tại cộng thêm 100
                else if (mx >= 450 && mx <= 600 && my >= 300 && my <= 350) {
                    SetVolume(GetVolume() + 100);
                }
                // Nút Quay lại
                else if (mx >= 325 && mx <= 475 && my >= 400 && my <= 450) {
                    PlayClickSound();
                    EndBatchDraw();
                    return; 
                }
            }
        }
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

                // 1. NEW GAME
                if (mx >= 300 && mx <= 510 && my >= 159 && my <= 212) { 
                    PlayClickSound();
                    putimage(300, 159, &imgNew); FlushBatchDraw(); Sleep(300);
                    PlayBGM(_T("Audio\\game_bgm.mp3"));
                    EndBatchDraw(); return 1; 
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
                    ShowInfoMenu(_T("       --- RULES ---"), _T("- Get 5 in a row to win."), _T("- You can win horizontally,"), _T("  vertically, or diagonally."));
                }
                
                // 6. EXIT (Có bảng xác nhận Tiếng Anh)
                if (mx >= 300 && mx <= 510 && my >= 461 && my <= 514) { 
                    PlayClickSound();
                    putimage(300, 457, &imgExit); FlushBatchDraw(); Sleep(300);
                    
                    int result = MessageBox(GetHWnd(), _T("Are you sure you want to exit game?"), _T("Exit Confirmation"), MB_YESNO | MB_ICONQUESTION);
                    if (result == IDYES) {
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
    while (true) {
        // Không xóa màn hình, chỉ vẽ một bảng gỗ đè lên ngay giữa bàn cờ
        int boardX = 250, boardY = 100, boardW = 300, boardH = 350;
        
        setlinecolor(BLACK); setlinestyle(PS_SOLID, 2);
        setfillcolor(RGB(60, 35, 20)); fillroundrect(boardX, boardY, boardX + boardW, boardY + boardH, 20, 20);
        setfillcolor(RGB(100, 60, 35)); fillroundrect(boardX + 10, boardY + 10, boardX + boardW - 10, boardY + boardH - 10, 15, 15);

        DrawTextWithShadow(280, 120, _T("--- GAME PAUSED ---"), WHITE, 28);

        // 3 Nút bấm xịn xò
        DrawWoodenButton(300, 180, 200, 50, _T("RESUME"));
        DrawWoodenButton(300, 250, 200, 50, _T("SETTINGS"));
        DrawWoodenButton(300, 320, 200, 50, _T("EXIT MATCH"));

        FlushBatchDraw();

        if (!IsWindow(GetHWnd())) exit(0);

        if (peekmessage(&msg, EM_MOUSE | EM_WINDOW)) {
            if (msg.message == WM_LBUTTONDOWN) {
                int mx = msg.x, my = msg.y;
                
                // 1. Click RESUME (Quay lại chơi tiếp)
                if (mx >= 300 && mx <= 500 && my >= 180 && my <= 230) {
                    PlayClickSound();
                    return 1;
                }
                // 2. Click SETTINGS (Mở cài đặt âm thanh)
                else if (mx >= 300 && mx <= 500 && my >= 250 && my <= 300) {
                    PlayClickSound();
                    return 2;
                }
                // 3. Click EXIT MATCH (Thoát ván cờ hiện tại)
                else if (mx >= 300 && mx <= 500 && my >= 320 && my <= 370) {
                    PlayClickSound();
                    
                    // Hiện bảng xác nhận bằng Tiếng Anh như bạn muốn
                    bool result = ShowConfirmDialog(_T("Confirm Exit"), _T("Are you sure you want to exit?"));
                    
                    if (result == true) {
                        return 3;
                    }
                }
            }
        }
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
        setlinecolor(BLACK); setlinestyle(PS_SOLID, 2);
        setfillcolor(RGB(60, 35, 20)); fillroundrect(boxX, boxY, boxX + boxW, boxY + boxH, 15, 15);
        setfillcolor(RGB(100, 60, 35)); fillroundrect(boxX + 5, boxY + 5, boxX + boxW - 5, boxY + boxH - 5, 10, 10);

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
        setlinecolor(BLACK); setlinestyle(PS_SOLID, 2);
        setfillcolor(RGB(60, 35, 20)); fillroundrect(boxX, boxY, boxX + boxW, boxY + boxH, 15, 15);
        setfillcolor(RGB(100, 60, 35)); fillroundrect(boxX + 5, boxY + 5, boxX + boxW - 5, boxY + boxH - 5, 10, 10);

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