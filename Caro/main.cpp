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
#include <ctime>
#include "Defs/Defs.h"
#include "View/View.h"
#include "Control/Control.h"
#include "Model/Model.h"
#include "Menu/Menu.h"
#include "SaveLoad/SaveLoad.h"
#include "Bot/Bot.h"

#ifdef _WIN32
#include <windows.h>
#endif

/* ---------------------------------------------------------------
 * Push a move onto the undo stack and record move order
 * --------------------------------------------------------------- */
static void RecordMove(int r, int c, int piece) {
    if (_UNDO_TOP < UNDO_MAX) {
        _UNDO_STACK[_UNDO_TOP] = { r, c, piece };
        _UNDO_TOP++;
    }
    _MOVE_COUNT++;
    _MOVE_ORDER[r][c] = _MOVE_COUNT;
}

/* ---------------------------------------------------------------
 * Pop one move from the undo stack and erase it from the board
 * --------------------------------------------------------------- */
static bool PopMove(void) {
    if (_UNDO_TOP <= 0) return false;
    _UNDO_TOP--;
    int r = _UNDO_STACK[_UNDO_TOP].r;
    int c = _UNDO_STACK[_UNDO_TOP].c;
    _MOVE_ORDER[r][c] = 0;
    _A[r][c].c = 0;
    _MOVE_COUNT--;
    return true;
}

/* ---------------------------------------------------------------
 * Undo the last player + bot pair of moves (PVE) or just the
 * last single move (PVP).  Returns true if anything was undone.
 * --------------------------------------------------------------- */
static bool DoUndo(void) {
    if (_UNDO_TOP <= 0) return false;

    if (_VS_BOT) {
        // Undo bot move first (if it was last)
        if (_UNDO_TOP >= 2) {
            PopMove(); // bot's move
            PopMove(); // player's move
            _TURN = 1; // restore player's turn
        } else {
            PopMove();
            _TURN = 1;
        }
    } else {
        PopMove();
        _TURN = !_TURN;
    }
    return true;
}

/* ---------------------------------------------------------------
 * Handle game-over display and result
 * --------------------------------------------------------------- */
static void HandleGameOver(int winner, bool& isPlaying) {
    RenderGame();
    if (winner == -1 || winner == 1) {
        if (winner == -1) _WIN_P1++;
        else              _WIN_P2++;
        DrawWinningLine(winner);
    }
    FlushBatchDraw();
    PlayClickSound();

    if (winner == -1)
        ShowNotifyDialog(_T("MATCH OVER"), _VS_BOT ? _T("You (X) WINS the game!") : _T("Player 1 (X) WINS the game!"));
    else if (winner == 1)
        ShowNotifyDialog(_T("MATCH OVER"), _VS_BOT ? _T("Computer (O) WINS the game!") : _T("Player 2 (O) WINS the game!"));
    else
        ShowNotifyDialog(_T("MATCH OVER"), _T("The board is full. It's a DRAW!"));

    isPlaying = false;
}

/* ---------------------------------------------------------------
 * Place a stone, record it, and test for game over.
 * Returns true if the game ended.
 * --------------------------------------------------------------- */
static bool PlaceStone(int r, int c, int piece, bool& isPlaying) {
    _A[r][c].c = piece;
    RecordMove(r, c, piece);
    PlayXO();
    int winner = TestBoard();
    if (winner != 2) {
        HandleGameOver(winner, isPlaying);
        return true;
    }
    return false;
}

/* ---------------------------------------------------------------
 * Bot turn
 * --------------------------------------------------------------- */
static void RunBotTurn(bool& isPlaying) {
    int br = 0, bc = 0;
    GetBotMove(br, bc);
    _X = br;
    _Y = bc;
    if (!PlaceStone(br, bc, 1, isPlaying)) {
        _TURN = 1;
    }
}

/* ---------------------------------------------------------------
 * main
 * --------------------------------------------------------------- */
int main() {
    SetConsoleOutputCP(65001);
    SetConsoleCP(65001);
    srand((unsigned)time(nullptr));
    InitSystem();

    while (true) {
        int choice = ShowMainMenu();
        if (choice == 3) break;
        if (choice == 1)      StartGame();
        else if (choice == 2) LoadGame();

        BeginBatchDraw();
        ExMessage msg;
        bool isPlaying   = true;
        bool needsRedraw = true;

        while (isPlaying) {
            // Bot turn
            if (_VS_BOT && _TURN == 0) {
                RunBotTurn(isPlaying);
                if (!isPlaying) continue;
                needsRedraw = true;
            }

            if (!IsWindow(GetHWnd())) exit(0);

            bool skipToNextFrame = false;
            while (peekmessage(&msg, EM_KEY | EM_MOUSE)) {
                if (msg.message == WM_KEYDOWN) {
                    if (msg.vkcode == VK_ESCAPE) {
                        isPlaying = false;
                    } else if (msg.vkcode == 'W' || msg.vkcode == VK_UP) {
                        MoveUp();   needsRedraw = true;
                    } else if (msg.vkcode == 'S' || msg.vkcode == VK_DOWN) {
                        MoveDown(); needsRedraw = true;
                    } else if (msg.vkcode == 'A' || msg.vkcode == VK_LEFT) {
                        MoveLeft(); needsRedraw = true;
                    } else if (msg.vkcode == 'D' || msg.vkcode == VK_RIGHT) {
                        MoveRight(); needsRedraw = true;
                    } else if (msg.vkcode == 'L') {
                        SaveGame(); needsRedraw = true;
                    } else if (msg.vkcode == 'T') {
                        LoadGame(); needsRedraw = true;
                    } else if (msg.vkcode == VK_BACK) {
                        // Backspace = undo shortcut
                        if (DoUndo()) needsRedraw = true;
                    } else if (msg.vkcode == VK_RETURN || msg.vkcode == VK_SPACE) {
                        if (_A[_X][_Y].c == 0) {
                            int piece = (_TURN == 1) ? -1 : 1;
                            if (!PlaceStone(_X, _Y, piece, isPlaying)) {
                                _TURN = !_TURN;
                                needsRedraw = true;
                            }
                        }
                    }
                } else if (msg.message == WM_LBUTTONDOWN) {
                    int mx = msg.x;
                    int my = msg.y;

                    // Pause / menu button
                    if (mx >= 718 && mx <= 770 && my >= 18 && my <= 70) {
                        PlayClickSound();
                        PauseBGM();
                        int action = ShowPauseMenu();
                        if (action == 1) {
                            ResumeBGM();
                            needsRedraw = true;
                            skipToNextFrame = true;
                            break;
                        } else if (action == 2) {
                            ShowSettingsMenu();
                            ResumeBGM();
                            needsRedraw = true;
                            skipToNextFrame = true;
                            break;
                        } else if (action == 3) {
                            isPlaying = false;
                            PlayBGM(_T("Audio\\bgm.mp3"));
                            skipToNextFrame = true;
                            break;
                        }
                    }

                    // Undo button in HUD (646..700, 526+8..526+54)
                    if (mx >= 646 && mx <= 700 && my >= 534 && my <= 580) {
                        if (DoUndo()) { PlayClickSound(); needsRedraw = true; }
                        break;
                    }

                    // Board click
                    int clickedCol = (mx - OFFSET_X) / CELL_SIZE;
                    int clickedRow = (my - OFFSET_Y) / CELL_SIZE;

                    if (clickedRow >= 0 && clickedRow < BOARD_SIZE &&
                        clickedCol >= 0 && clickedCol < BOARD_SIZE) {
                        _X = clickedRow;
                        _Y = clickedCol;
                        needsRedraw = true;

                        if (_A[_X][_Y].c == 0 && (_TURN == 1 || !_VS_BOT)) {
                            int piece = (_TURN == 1) ? -1 : 1;
                            if (!PlaceStone(_X, _Y, piece, isPlaying)) {
                                _TURN = !_TURN;
                            }
                        }
                    }
                }
            }

            if (skipToNextFrame) continue;

            if (isPlaying && needsRedraw) {
                RenderGame();
                FlushBatchDraw();
                needsRedraw = false;
            } else if (isPlaying) {
                Sleep(16);
            }
        }
        EndBatchDraw();
    }

    CloseSystem();
    return 0;
}
