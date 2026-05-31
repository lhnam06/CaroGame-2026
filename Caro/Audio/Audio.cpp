#include "Audio.h"
#include <windows.h>
#include <mmsystem.h>
#include <filesystem>
#include <string>
#pragma comment(lib, "Winmm.lib")

namespace fs = std::filesystem;

int currentVolume = 500;

static std::wstring g_pathMenuBgm;
static std::wstring g_pathGameBgm;
static std::wstring g_pathClick;
static std::wstring g_pathX;
static std::wstring g_pathO;
static std::wstring g_pathWin;
static std::wstring g_pathSave;
static std::wstring g_pathLoad;
static bool g_audioReady = false;

static std::wstring AbsWide(const fs::path& p) {
    std::error_code ec;
    return fs::absolute(p, ec).wstring();
}

static void ApplyVolumeToAlias(const wchar_t* alias) {
    wchar_t volCmd[128];
    swprintf_s(volCmd, L"setaudio %s volume to %d", alias, currentVolume);
    mciSendStringW(volCmd, NULL, 0, NULL);
}

static void PlaySfxW(const std::wstring& filePath) {
    if (filePath.empty()) return;

    mciSendStringW(L"close sfx", NULL, 0, NULL);

    wchar_t openCmd[1024];
    swprintf_s(openCmd, L"open \"%s\" type mpegvideo alias sfx", filePath.c_str());
    if (mciSendStringW(openCmd, NULL, 0, NULL) != 0) return;

    ApplyVolumeToAlias(L"sfx");
    mciSendStringW(L"play sfx", NULL, 0, NULL);
}

static void PlayBgmW(const std::wstring& filePath) {
    if (filePath.empty()) return;

    mciSendStringW(L"close bgmusic", NULL, 0, NULL);

    wchar_t openCmd[1024];
    swprintf_s(openCmd, L"open \"%s\" type mpegvideo alias bgmusic", filePath.c_str());
    if (mciSendStringW(openCmd, NULL, 0, NULL) != 0) return;

    ApplyVolumeToAlias(L"bgmusic");
    mciSendStringW(L"play bgmusic repeat", NULL, 0, NULL);
}

void InitAudioPaths(void) {
    if (g_audioReady) return;

    fs::path root = fs::path("GUI") / "amthanh";
    if (!fs::exists(root)) return;

    for (const auto& entry : fs::recursive_directory_iterator(root)) {
        if (!entry.is_regular_file()) continue;
        if (entry.path().extension() != ".mp3") continue;

        std::wstring fn = entry.path().filename().wstring();
        std::wstring full = AbsWide(entry.path());

        if (fn.find(L"lofi") != std::wstring::npos) {
            g_pathMenuBgm = full;
        } else if (fn == L"X.mp3") {
            g_pathX = full;
        } else if (fn == L"O.mp3") {
            g_pathO = full;
        } else if (fn == L"win.mp3") {
            g_pathWin = full;
        } else if (fn == L"save.mp3") {
            g_pathSave = full;
        } else if (fn == L"load.mp3") {
            g_pathLoad = full;
        } else if (fn.find(L"lofi") == std::wstring::npos && fn.find(L"2.mp3") != std::wstring::npos) {
            g_pathGameBgm = full;
        }
    }

    fs::path menuDir = root / "menu";
    if (fs::exists(menuDir)) {
        for (const auto& entry : fs::directory_iterator(menuDir)) {
            if (entry.path().extension() == ".mp3") {
                g_pathClick = AbsWide(entry.path());
                break;
            }
        }
    }

    g_audioReady = true;
}

void PlayMenuBGM(void) { PlayBgmW(g_pathMenuBgm); }
void PlayGameBGM(void) { PlayBgmW(g_pathGameBgm); }

void StopBGM() {
    mciSendStringW(L"stop bgmusic", NULL, 0, NULL);
}

void PauseBGM() {
    mciSendStringW(L"pause bgmusic", NULL, 0, NULL);
}

void ResumeBGM() {
    mciSendStringW(L"resume bgmusic", NULL, 0, NULL);
}

void PlayClickSound() { PlaySfxW(g_pathClick); }
void PlayXSound()      { PlaySfxW(g_pathX); }
void PlayOSound()      { PlaySfxW(g_pathO); }

void PlayStoneSound(int piece) {
    if (piece == -1) PlayXSound();
    else if (piece == 1) PlayOSound();
}

void PlayWinSound()  { PlaySfxW(g_pathWin); }
void PlaySaveSound() { PlaySfxW(g_pathSave); }
void PlayLoadSound() { PlaySfxW(g_pathLoad); }

void SetVolume(int vol) {
    if (vol < 0) vol = 0;
    if (vol > 1000) vol = 1000;

    currentVolume = vol;

    ApplyVolumeToAlias(L"bgmusic");
    ApplyVolumeToAlias(L"sfx");
}

int GetVolume(void) {
    return currentVolume;
}

int GetVolumePercent(void) {
    return currentVolume / 10;
}

void SetVolumePercent(int percent) {
    if (percent < 0) percent = 0;
    if (percent > 100) percent = 100;
    SetVolume(percent * 10);
}
