#ifndef AUDIO_H
#define AUDIO_H

#include <tchar.h> // Thêm thư viện này để dùng được chữ _T

void PlayBGM(const TCHAR* filePath);

void StopBGM();    // Tắt hẳn
void PauseBGM();    // Tạm ngưng
void ResumeBGM();   // Phát tiếp

void PlayClickSound();
void PlayXO();
void SetVolume(int vol);
int GetVolume();

#endif