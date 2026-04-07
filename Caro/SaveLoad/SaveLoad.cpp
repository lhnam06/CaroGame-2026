#include "Defs/Defs.h"
#include "View/View.h"
#include "Model/Model.h"
#include "Audio/Audio.h"
#include "SaveLoad/SaveLoad.h"
#include <fstream>
#include <graphics.h>
#include <tchar.h>
#include <string> 

extern std::string _SELECTED_FILE;

void SaveGame(void) {
    TCHAR filename[256];
    
    // Đổi lại câu nhắc cho gọn, không cần ép người ta gõ .txt nữa
    InputBox(filename, 256, _T("Enter filename to save:"), _T("Save Game"));
    
    if (_tcslen(filename) == 0) return; // Nếu bấm Cancel thì thoát

    // --- XỬ LÝ TỰ ĐỘNG THÊM ĐUÔI .TXT ---
    std::basic_string<TCHAR> tFilename(filename);
    
    // Kiểm tra xem chuỗi có ngắn hơn 4 ký tự, hoặc 4 ký tự cuối không phải là ".txt"
    if (tFilename.length() < 4 || tFilename.substr(tFilename.length() - 4) != _T(".txt")) {
        tFilename += _T(".txt"); // Tự động dán thêm .txt vào đuôi
    }

    // Mở file với tên đã được chuẩn hóa
    std::ofstream f(tFilename.c_str());
    if (!f) {
        MessageBox(GetHWnd(), _T("Cannot create file!"), _T("Error"), MB_OK | MB_ICONERROR);
        return;
    }

    // Tiến hành lưu dữ liệu
    f << _TURN << " " << _X << " " << _Y << " "
      << _WIN_P1 << " " << _WIN_P2 << " " << _MOVE_P1 << " " << _MOVE_P2 << "\n";
      
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            f << _A[i][j].c << " ";
        }
        f << "\n";
    }
    f.close();
    
    MessageBox(GetHWnd(), _T("Game saved successfully!"), _T("Notification"), MB_OK | MB_ICONINFORMATION);
}

void LoadGame(void) {
    // 1. Kiểm tra xem người chơi đã thực sự chọn file bên Menu chưa
    if (_SELECTED_FILE == "") return;

    // 2. Mở đúng cái tên file đã chọn
    std::ifstream f(_SELECTED_FILE);
    if (!f) {
        MessageBox(GetHWnd(), _T("Cannot find file!"), _T("Error"), MB_OK | MB_ICONERROR);
        _SELECTED_FILE = ""; // Reset biến tạm
        return;
    }

    // 3. Đọc dữ liệu ra bàn cờ
    f >> _TURN >> _X >> _Y >> _WIN_P1 >> _WIN_P2 >> _MOVE_P1 >> _MOVE_P2;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            f >> _A[i][j].c;
            _A[i][j].x = i; 
            _A[i][j].y = j; 
        }
    }
    PlayBGM(_T("Audio\\game_bgm.mp3"));
    f.close();
    
    // 4. Xóa tên file vừa nhớ đi để lần sau người chơi load file khác nó không bị kẹt
    _SELECTED_FILE = ""; 
    
    MessageBox(GetHWnd(), _T("Game loaded successfully!"), _T("Notification"), MB_OK | MB_ICONINFORMATION);
}