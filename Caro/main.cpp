/*
 * CaroGame - Do an Co So Lap Trinh
 * Truong Dai hoc Khoa hoc Tu nhien TP.HCM
 * C++ - Lap trinh thu tuc (khong su dung OOP)
 */

/*#include <iostream>
#include <cctype>
#include "Defs/Defs.h"
#include "View/View.h"
#include "Model/Model.h"
#include "Control/Control.h"
#include "SaveLoad/SaveLoad.h"
#include "Menu/Menu.h"

#ifdef _WIN32
#include <conio.h>
#endif

int main(void) {
    int validEnter;
    int menuChoice;

#ifdef _WIN32
    FixConsoleWindow();   
#endif

    _WIN_P1 = 0;
    _WIN_P2 = 0;

    menuChoice = ShowMainMenu();
    if (menuChoice == 2) {
        ResetData();
        LoadGame();
    } else if (menuChoice == 3) {
        ExitGame();
        return 0;
    } else {
        StartGame();
    }

    validEnter = 1;
    while (1) {
#ifdef _WIN32
        _COMMAND = toupper(getch());
#else
        _COMMAND = toupper(std::cin.get());
#endif

        if (_COMMAND == 27) {
            ExitGame();
            return 0;
        } else if (_COMMAND == 'L') {
            SaveGame();
        } else if (_COMMAND == 'T') {
            LoadGame();
        } else if (_COMMAND == 'A') {
            MoveLeft();
        } else if (_COMMAND == 'W') {
            MoveUp();
        } else if (_COMMAND == 'S') {
            MoveDown();
        } else if (_COMMAND == 'D') {
            MoveRight();
        } else if (_COMMAND == 13) {
            switch (CheckBoard(_X, _Y)) {
                case -1: std::cout << "X"; break;
                case 1:  std::cout << "O"; break;
                case 0:  validEnter = 0; break;
            }

            if (validEnter == 1) {
                switch (ProcessFinish(TestBoard())) {
                    case -1: case 1: case 0:
                        if (AskContinue() != 'Y') {
                            ExitGame();
                            return 0;
                        }
                        StartGame();
                        break;
                }
            }
            validEnter = 1;
        }
    }

    return 0;
}*/
#include "Audio/Audio.h"
#include <cstdlib> 
#include "Defs/Defs.h"
#include "View/View.h"
#include "Control/Control.h"
#include "Model/Model.h"
#include "Menu/Menu.h"
#include <SaveLoad/SaveLoad.h>

int main() {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    InitSystem();

    while (true) {
        int choice = ShowMainMenu();
        
        if (choice == 3) {
            break; // Thoát game
        } 
        else if (choice == 1) {
            StartGame(); // Reset data
        }
        else if (choice == 2) {
            LoadGame(); // Kích hoạt chức năng Load
            // (Nếu Load xong thì vòng lặp bên dưới sẽ tự động vẽ lại bàn cờ cũ)
        }

        // --- BẮT ĐẦU VÒNG LẶP GAME CỐT LÕI MÀ BẠN VỪA LÀM THÀNH CÔNG ---
        BeginBatchDraw(); 
        ExMessage msg;
        bool isPlaying = true;

        while (isPlaying) {
            RenderGame();       
            FlushBatchDraw();   

            if (!IsWindow(GetHWnd())) {
                exit(0); // Tắt sạch sẽ chương trình
            }
            if (peekmessage(&msg, EM_KEY | EM_MOUSE)) {
                if (msg.message == WM_KEYDOWN) {
                    if (msg.vkcode == VK_ESCAPE) {
                        isPlaying = false; // Bấm ESC thì thoát ván chơi, quay về Menu
                    }
                    // Hỗ trợ cả WASD và Phím mũi tên (Mũi tên lên, xuống, trái, phải)
                    else if (msg.vkcode == 'W' || msg.vkcode == VK_UP) MoveUp();
                    else if (msg.vkcode == 'S' || msg.vkcode == VK_DOWN) MoveDown();
                    else if (msg.vkcode == 'A' || msg.vkcode == VK_LEFT) MoveLeft();
                    else if (msg.vkcode == 'D' || msg.vkcode == VK_RIGHT) MoveRight();
                    else if (msg.vkcode == 'L') SaveGame(); // Bấm L để lưu ván cờ đang chơi dở
                    else if (msg.vkcode == 'T') LoadGame(); // Bấm T để tải ván cờ khác
                    else if (msg.vkcode == VK_RETURN || msg.vkcode == VK_SPACE)
                    { 
                        // Bấm Enter hoặc Nút Space để đánh cờ
                        if (_A[_X][_Y].c == 0) {
                            _A[_X][_Y].c = (_TURN == 1) ? -1 : 1;
                            PlayXO();
                            // --- KIỂM TRA THẮNG THUA ---
                            int winner = TestBoard(); 
                    
                            if (winner != 2) { 
                                RenderGame();      
                                
                                if (winner == -1 || winner == 1) {
                                    if (winner == -1) _WIN_P1++;
                                    else _WIN_P2++;
                                    DrawWinningLine(winner); // Vẽ đường vàng
                                }
                                FlushBatchDraw(); // Đẩy hình ảnh ra màn hình trước

                                // Gọi bảng Gỗ thông báo thay cho MessageBox
                                PlayClickSound(); // Có thể thay bằng tiếng vỗ tay/win.wav nếu bạn có
                                if (winner == -1) ShowNotifyDialog(_T("MATCH OVER"), _T("Player 1 (X) WINS the game!"));
                                else if (winner == 1) ShowNotifyDialog(_T("MATCH OVER"), _T("Player 2 (O) WINS the game!"));
                                else ShowNotifyDialog(_T("MATCH OVER"), _T("The board is full. It's a DRAW!"));
                        
                                isPlaying = false; 
                            } else {
                                _TURN = !_TURN; 
                            }
                        }
                    }
                }
                else if (msg.message == WM_LBUTTONDOWN) {
                    int mx = msg.x;
                    int my = msg.y;

                    // --- KIỂM TRA XEM CÓ CLICK VÀO NÚT CÀI ĐẶT KHÔNG ---
                    if (mx >= 720 && mx <= 770 && my >= 20 && my <= 70) {
                        PlayClickSound();
                        PauseBGM();
                        int action = ShowPauseMenu(); // Bật bảng Pause lên
                        
                        if (action == 1) {
                            ResumeBGM();
                            // Chọn RESUME: Chạy tiếp vòng lặp, vẽ lại bàn cờ
                            continue;
                        }
                        else if (action == 2) {
                            // Chọn SETTINGS: Mở bảng cài đặt
                            ShowSettingsMenu();
                            ResumeBGM();
                            continue; 
                        }
                        else if (action == 3) {
                            // Chọn EXIT MATCH: Thoát ván chơi, trở về Main Menu
                            isPlaying = false; 
                            PlayBGM(_T("Audio\\bgm.mp3")); // Bật lại nhạc của Menu chính
                            continue;
                        }
                    }
                    int clickedCol = (msg.x - OFFSET_X) / CELL_SIZE;
                    int clickedRow = (msg.y - OFFSET_Y) / CELL_SIZE;

                    // Kiểm tra xem vị trí click có nằm trong giới hạn bàn cờ không
                    if (clickedRow >= 0 && clickedRow < BOARD_SIZE && clickedCol >= 0 && clickedCol < BOARD_SIZE) {
                        // Di chuyển con trỏ xanh lá tới đó
                        _X = clickedRow;
                        _Y = clickedCol;

                        // Thực hiện đánh cờ luôn
                        if (_A[_X][_Y].c == 0) {
                            _A[_X][_Y].c = (_TURN == 1) ? -1 : 1;
                            PlayXO();
                            // --- KIỂM TRA THẮNG THUA ---
                            int winner = TestBoard(); 
                    
                            if (winner != 2) { 
                                RenderGame();      // Vẽ quân cờ cuối cùng trước
                        
                                if (winner == -1 || winner == 1) {
                                    if (winner == -1) _WIN_P1++;
                                    else _WIN_P2++;
                            
                                    DrawWinningLine(winner); // Vẽ đường vàng rực & Khung chữ
                                }
                                else if (winner == 0) {
                                    MessageBox(GetHWnd(), _T("The table is full, the game is a draw!"), _T("DRAW"), MB_OK);
                                }
                        
                                FlushBatchDraw(); // Đẩy hình ảnh ra màn hình trước
                    
                                // Gọi bảng Gỗ thông báo thay cho MessageBox
                                PlayClickSound(); 
                                if (winner == -1) ShowNotifyDialog(_T("MATCH OVER"), _T("Player 1 (X) WINS the game!"));
                                else if (winner == 1) ShowNotifyDialog(_T("MATCH OVER"), _T("Player 2 (O) WINS the game!"));
                                else ShowNotifyDialog(_T("MATCH OVER"), _T("The board is full. It's a DRAW!"));
                    
                                isPlaying = false;
                            } 
                            else {
                            _TURN = !_TURN; // Chưa ai thắng thì đổi lượt
                            }
                        }
                    }
                }
            }
            Sleep(10);
        }
        EndBatchDraw();
    } // Hết vòng lặp tổng

    CloseSystem();
    return 0;
}
