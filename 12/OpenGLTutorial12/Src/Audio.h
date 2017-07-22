/**
* @file Audio.h
*/
#ifndef AUDIO_H_INCLUDED
#define AUDIO_H_INCLUDED

namespace Audio {

bool Initialize();
void Update();
void Destroy();
void Play(int playerId, int cueId);

} // namespace Audio

#endif // AUDIO_H_INCLUDED