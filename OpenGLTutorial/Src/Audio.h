/**
* @file Audio.h
*/
#ifndef AUDIO_H_INCLUDED
#define AUDIO_H_INCLUDED

namespace Audio {

bool Initialize(const char* acfPath, const char* acbPath, const char* acwPath, const char* dspBusName, size_t playerCount);
void Update();
void Destroy();
void Play(int playerId, int cueId);
void Stop(int playerId);

} // namespace Audio

#endif // AUDIO_H_INCLUDED