#include "Defs/Defs.h"
#include "View/View.h"
#include "Model/Model.h"
#include "Audio/Audio.h"
#include "Menu/Menu.h"
#include "SaveLoad/SaveLoad.h"
#include <fstream>
#include <graphics.h>
#include <tchar.h>
#include <string>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <ctime>

#include <windows.h>

extern std::string _SELECTED_FILE;

namespace fs = std::filesystem;

static void NameToUtf8(const TCHAR* wide, std::string& out) {
    out.clear();
    if (!wide || !wide[0]) return;
#ifdef UNICODE
    char buf[128];
    int n = WideCharToMultiByte(CP_UTF8, 0, wide, -1, buf, (int)sizeof(buf), NULL, NULL);
    if (n > 0) out.assign(buf);
#else
    out.assign(wide);
#endif
}

static void Utf8ToName(const std::string& utf8, TCHAR* wide, size_t wideCount) {
    if (wideCount == 0) return;
    wide[0] = 0;
    if (utf8.empty()) return;
#ifdef UNICODE
    MultiByteToWideChar(CP_UTF8, 0, utf8.c_str(), -1, wide, (int)wideCount);
#else
    _tcscpy_s(wide, wideCount, utf8.c_str());
#endif
}

static void SplitPipe2(const std::string& s, std::string& a, std::string& b) {
    size_t p = s.find('|');
    if (p == std::string::npos) {
        a = s;
        b.clear();
        return;
    }
    a = s.substr(0, p);
    b = s.substr(p + 1);
}

static bool ParseBoardRow(const std::string& line, int row, std::ifstream* f) {
    std::istringstream rs(line);
    for (int j = 0; j < BOARD_SIZE; j++) {
        if (!(rs >> _A[row][j].c)) return false;
        _A[row][j].x = row;
        _A[row][j].y = j;
    }
    return true;
}

static void FormatTimeT(TCHAR* buf, size_t bufCount, std::time_t tt) {
    std::tm lt = {};
    if (localtime_s(&lt, &tt) != 0 || bufCount == 0) {
        buf[0] = 0;
        return;
    }
    _tcsftime(buf, bufCount, _T("%Y-%m-%d %H:%M"), &lt);
}

static bool FileTimeToLocalStr(const fs::path& p, TCHAR* out, size_t outCount) {
    std::error_code ec;
    if (!fs::exists(p, ec)) {
        _tcscpy_s(out, outCount, _T("--"));
        return false;
    }
    auto ftime = fs::last_write_time(p, ec);
    if (ec) {
        _tcscpy_s(out, outCount, _T("--"));
        return false;
    }
    auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now());
    std::time_t tt = std::chrono::system_clock::to_time_t(sctp);
    FormatTimeT(out, outCount, tt);
    return true;
}

static void NowShort(TCHAR* buf, size_t n) {
    std::time_t t = std::time(nullptr);
    FormatTimeT(buf, n, t);
}

void SaveGame(void) {
    if (_SELECTED_FILE.empty()) {
        ShowNotifyDialog(_T("SAVE GAME"), _T("Please choose a file first."));
        return;
    }

    std::string savePath = _SELECTED_FILE;
    if (savePath.length() < 4 || _stricmp(savePath.c_str() + savePath.length() - 4, ".txt") != 0) {
        savePath += ".txt";
    }

    std::ofstream f(savePath.c_str());
    if (!f) {
        ShowNotifyDialog(_T("SAVE GAME"), _T("Cannot create file!"));
        return;
    }

    f << _TURN << " " << _X << " " << _Y << " "
      << _WIN_P1 << " " << _WIN_P2 << " " << _MOVE_P1 << " " << _MOVE_P2 << " "
      << _VS_BOT << " " << _CHAR_P1 << " " << _CHAR_P2 << "\n";

    std::string n1, n2;
    NameToUtf8(_NAME_P1, n1);
    NameToUtf8(_NAME_P2, n2);
    f << "#NAMES " << n1 << "|" << n2 << "\n";

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            f << _A[i][j].c << " ";
        }
        f << "\n";
    }

    TCHAR ts[64];
    NowShort(ts, 64);
    f << "TS ";
    f << ts << "\n";

    f.close();

    PlaySaveSound();
    ShowNotifyDialog(_T("SAVE GAME"), _T("Saved successfully!"));
}

void LoadGame(void) {
    if (_SELECTED_FILE == "") return;

    fs::path fp(_SELECTED_FILE);
    TCHAR fileMTime[64];
    FileTimeToLocalStr(fp, fileMTime, 64);

    std::ifstream f(_SELECTED_FILE);
    if (!f) {
        ShowNotifyDialog(_T("LOAD GAME"), _T("Cannot find file!"));
        _SELECTED_FILE = "";
        return;
    }

    std::string line;
    if (!std::getline(f, line)) {
        ShowNotifyDialog(_T("LOAD GAME"), _T("Cannot read file!"));
        _SELECTED_FILE = "";
        return;
    }

    std::istringstream ls(line);
    ls >> _TURN >> _X >> _Y >> _WIN_P1 >> _WIN_P2 >> _MOVE_P1 >> _MOVE_P2;
    if (!(ls >> _VS_BOT))
        _VS_BOT = 0;
    if (!(ls >> _CHAR_P1))
        _CHAR_P1 = 1;
    if (!(ls >> _CHAR_P2))
        _CHAR_P2 = 2;

    _UNDO_TOP = 0;
    _MOVE_COUNT = _MOVE_P1 + _MOVE_P2;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            _MOVE_ORDER[i][j] = 0;
        }
    }

    std::string nextLine;
    int boardRow = 0;
    if (std::getline(f, nextLine)) {
        if (nextLine.size() >= 7 && nextLine.compare(0, 7, "#NAMES ") == 0) {
            std::string payload = nextLine.substr(7);
            std::string n1, n2;
            SplitPipe2(payload, n1, n2);
            Utf8ToName(n1, _NAME_P1, 32);
            Utf8ToName(n2, _NAME_P2, 32);
            boardRow = 0;
        } else {
            if (!ParseBoardRow(nextLine, 0, &f)) {
                ShowNotifyDialog(_T("LOAD GAME"), _T("Cannot read file!"));
                _SELECTED_FILE = "";
                return;
            }
            boardRow = 1;
        }
    }

    for (int i = boardRow; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            f >> _A[i][j].c;
            _A[i][j].x = i;
            _A[i][j].y = j;
        }
    }

    ReloadAvatars();

    std::string savedInFile;
    std::string tail;
    f >> std::ws;
    if (std::getline(f, tail)) {
        if (tail.size() >= 3 && tail[0] == 'T' && tail[1] == 'S' && (tail[2] == ' ' || tail[2] == '\t')) {
            size_t p = tail.find_first_not_of(" \t", 2);
            if (p != std::string::npos)
                savedInFile = tail.substr(p);
        }
    }

    f.close();

    PlayLoadSound();
    PlayGameBGM();

    _SELECTED_FILE = "";

    if (!savedInFile.empty()) {
        ShowNotifyDialog(_T("LOAD GAME"), _T("File selected and loaded successfully!"), 1000);
    } else {
        ShowNotifyDialog(_T("LOAD GAME"), _T("Loaded successfully!"), 1000);
    }
}
