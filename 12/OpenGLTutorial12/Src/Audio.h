/**
* @file Audio.h
*/
#ifndef AUDIO_H_INCLUDED
#define AUDIO_H_INCLUDED

namespace Audio {

static const size_t playerMax = 8;

bool Initialize(const char* acfPath, const char* acbPath, const char* awbPath, const char* dspBusName);
void Update();
void Destroy();
void Play(int playerId, int cueId);
void Stop(int playerId);

} // namespace Audio

#endif // AUDIO_H_INCLUDED