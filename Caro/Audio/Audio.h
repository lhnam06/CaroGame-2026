#ifndef AUDIO_H
#define AUDIO_H

void InitAudioPaths(void);

void PlayMenuBGM(void);
void PlayGameBGM(void);
void StopBGM();
void PauseBGM();
void ResumeBGM();

void PlayClickSound();
void PlayXSound();
void PlayOSound();
void PlayStoneSound(int piece);
void PlayWinSound();
void PlaySaveSound();
void PlayLoadSound();
void SetVolume(int vol);
int GetVolume(void);
int GetVolumePercent(void);
void SetVolumePercent(int percent);

#endif
