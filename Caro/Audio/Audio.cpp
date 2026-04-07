#include "Audio.h" 
#include <windows.h>
#include <mmsystem.h>
#include <cstdio>
#pragma comment(lib, "Winmm.lib")

int currentVolume = 500; 

// --- HÀM PHÁT NHẠC NỀN THÔNG MINH ---
void PlayBGM(const TCHAR* filePath) {
    // 1. Đóng bài nhạc cũ đang phát (nếu có) để không bị đè âm thanh
    mciSendString(_T("close bgmusic"), NULL, 0, NULL); 
    
    // 2. Mở bài nhạc mới
    TCHAR openCmd[256];
    _stprintf_s(openCmd, 256, _T("open \"%s\" type mpegvideo alias bgmusic"), filePath);
    mciSendString(openCmd, NULL, 0, NULL);
    
    // 3. Áp dụng mức âm lượng hiện tại
    SetVolume(currentVolume);
    
    // 4. Phát lặp lại
    mciSendString(_T("play bgmusic repeat"), NULL, 0, NULL);
}

void StopBGM() {
    mciSendString(_T("stop bgmusic"), NULL, 0, NULL);
}

void PauseBGM() {
    mciSendString(_T("pause bgmusic"), NULL, 0, NULL);
}

void ResumeBGM() {
    mciSendString(_T("resume bgmusic"), NULL, 0, NULL);
}

void PlayClickSound() {
    mciSendString(_T("close clickSound"), NULL, 0, NULL);
    mciSendString(_T("open \"Audio\\click.mp3\" alias clickSound"), NULL, 0, NULL);
    mciSendString(_T("setaudio clickSound volume to 1000"), NULL, 0, NULL);
    mciSendString(_T("play clickSound"), NULL, 0, NULL);
}

void PlayXO() {
    mciSendString(_T("close clickSound"), NULL, 0, NULL);
    mciSendString(_T("open \"Audio\\XO.mp3\" alias clickSound"), NULL, 0, NULL);
    mciSendString(_T("setaudio clickSound volume to 1000"), NULL, 0, NULL);
    mciSendString(_T("play clickSound"), NULL, 0, NULL);
}

void SetVolume(int vol) {
    if (vol < 0) vol = 0;
    if (vol > 1000) vol = 1000;
    
    currentVolume = vol;
    
    TCHAR volCmd[100];
    _stprintf_s(volCmd, 100, _T("setaudio bgmusic volume to %d"), currentVolume);
    mciSendString(volCmd, NULL, 0, NULL);
}

int GetVolume() {
    return currentVolume;
}