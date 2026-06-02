/*
 * CaroGame - Do an Co So Lap Trinh
 * Truong Dai hoc Khoa hoc Tu nhien TP.HCM
 * C++ - Lap trinh thu tuc (khong su dung OOP)
 */

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
    if (winner == -1 || winner == 1) {
        if (winner == -1) _WIN_P1++;
        else              _WIN_P2++;
        PlayWinSound();
        ShowWinScreen(winner);
    } else {
        RenderGame();
        FlushBatchDraw();
        PlayClickSound();
        ShowNotifyDialog(_T("MATCH OVER"), _T("The board is full. It's a DRAW!"));
    }
    // After showing win/draw UI, ask the player if they want to play again.
    // If yes, start a fresh game immediately; otherwise end the current match.
    if (ShowConfirmDialog(_T("PLAY AGAIN"), _T("Do you want to play another match?"))) {
        StartGame();
        isPlaying = true;
    } else {
        isPlaying = false;
    }
}

/* ---------------------------------------------------------------
 * Place a stone, record it, and test for game over.
 * Returns true if the game ended.
 * --------------------------------------------------------------- */
static bool PlaceStone(int r, int c, int piece, bool& isPlaying) {
    _A[r][c].c = piece;
    RecordMove(r, c, piece);
    PlayStoneSound(piece);
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
                        if (ShowSaveMenuUI()) {
                            SaveGame();
                            needsRedraw = true;
                        }
                    } else if (msg.vkcode == 'T') {
                        if (ShowLoadMenuUI()) {
                            LoadGame();
                            needsRedraw = true;
                        }
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
                } else if (msg.message == WM_MOUSEMOVE) {
                    int hoverRow, hoverCol;
                    if (BoardPointToCell(msg.x, msg.y, &hoverRow, &hoverCol)) {
                        if (hoverRow != _X || hoverCol != _Y) {
                            _X = hoverRow;
                            _Y = hoverCol;
                            needsRedraw = true;
                        }
                    }
                } else if (msg.message == WM_LBUTTONDOWN) {
                    int mx = msg.x;
                    int my = msg.y;

                    // Pause / menu button
                    {
                        int menuX = SCREEN_W - UiScale(82);
                        int menuY = UiScale(18);
                        int menuS = UiScale(56);
                        if (mx >= menuX && mx <= menuX + menuS && my >= menuY && my <= menuY + menuS) {
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
                                PlayMenuBGM();
                                skipToNextFrame = true;
                                break;
                            }
                        }
                    }

                    // Undo button in HUD
                    {
                        int undoX, undoY, undoW, undoH;
                        GetUndoButtonRect(&undoX, &undoY, &undoW, &undoH);
                        if (mx >= undoX && mx <= undoX + undoW && my >= undoY && my <= undoY + undoH) {
                            if (DoUndo()) { PlayClickSound(); needsRedraw = true; }
                            break;
                        }
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
            }
        }

        ResetSessionStats();
        EndBatchDraw();
    }

    CloseSystem();
    return 0;
}
