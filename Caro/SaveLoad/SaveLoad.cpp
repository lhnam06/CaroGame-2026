#include "Defs/Defs.h"
#include "View/View.h"
#include "Model/Model.h"
#include "Audio/Audio.h"
#include "SaveLoad/SaveLoad.h"
#include <fstream>
#include <graphics.h>
#include <tchar.h>
#include <string>
#include <sstream>
#include <filesystem>
#include <chrono>
#include <ctime>

extern std::string _SELECTED_FILE;

namespace fs = std::filesystem;

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
    TCHAR filename[256];

    InputBox(filename, 256, _T("Enter filename to save:"), _T("Save Game"));

    if (_tcslen(filename) == 0) return;

    std::basic_string<TCHAR> tFilename(filename);

    if (tFilename.length() < 4 || tFilename.substr(tFilename.length() - 4) != _T(".txt")) {
        tFilename += _T(".txt");
    }

    std::ofstream f(tFilename.c_str());
    if (!f) {
        MessageBox(GetHWnd(), _T("Cannot create file!"), _T("Error"), MB_OK | MB_ICONERROR);
        return;
    }

    f << _TURN << " " << _X << " " << _Y << " "
      << _WIN_P1 << " " << _WIN_P2 << " " << _MOVE_P1 << " " << _MOVE_P2 << " " << _VS_BOT << "\n";

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

    MessageBox(GetHWnd(), _T("Game saved successfully!"), _T("Notification"), MB_OK | MB_ICONINFORMATION);
}

void LoadGame(void) {
    if (_SELECTED_FILE == "") return;

    fs::path fp(_SELECTED_FILE);
    TCHAR fileMTime[64];
    FileTimeToLocalStr(fp, fileMTime, 64);

    std::ifstream f(_SELECTED_FILE);
    if (!f) {
        MessageBox(GetHWnd(), _T("Cannot find file!"), _T("Error"), MB_OK | MB_ICONERROR);
        _SELECTED_FILE = "";
        return;
    }

    std::string line;
    if (!std::getline(f, line)) {
        MessageBox(GetHWnd(), _T("Cannot read file!"), _T("Error"), MB_OK | MB_ICONERROR);
        _SELECTED_FILE = "";
        return;
    }

    std::istringstream ls(line);
    ls >> _TURN >> _X >> _Y >> _WIN_P1 >> _WIN_P2 >> _MOVE_P1 >> _MOVE_P2;
    if (!(ls >> _VS_BOT))
        _VS_BOT = 0;

    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            f >> _A[i][j].c;
            _A[i][j].x = i;
            _A[i][j].y = j;
        }
    }

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

    PlayBGM(_T("Audio\\game_bgm.mp3"));

    _SELECTED_FILE = "";

    TCHAR loadNow[64];
    NowShort(loadNow, 64);

    TCHAR msg[768];
    if (!savedInFile.empty()) {
        _stprintf_s(msg, _T("Game loaded successfully!\n\nFile modified: %s\nSaved (in file): %s\nLoaded at: %s"),
            fileMTime, savedInFile.c_str(), loadNow);
    } else {
        _stprintf_s(msg, _T("Game loaded successfully!\n\nFile modified: %s\nLoaded at: %s"),
            fileMTime, loadNow);
    }

    MessageBox(GetHWnd(), msg, _T("Notification"), MB_OK | MB_ICONINFORMATION);
}
