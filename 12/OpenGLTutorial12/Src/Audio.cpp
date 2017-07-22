/**
* @file Audio.cpp
*/
#include "Audio.h"
#include "../Res/Audio/SampleSound_acf.h"
#include <cri_adx2le.h>
#include <cstdint>
#include <Windows.h>

 namespace Audio {

 CriAtomExVoicePoolHn voicePool;
 CriAtomDbasId dbas;
 CriAtomExAcbHn acb;
 CriAtomExPlayerHn player[8];

 static void ErrorCallback(const CriChar8* errid, CriUint32 p1, CriUint32 p2, CriUint32* parray)
 {
   const CriChar8* err = criErr_ConvertIdToMessage(errid, p1, p2);
   OutputDebugStringA(err);
   OutputDebugStringA("\n");
 }

 void* Allocate(void* obj, CriUint32 size)  { return new uint8_t[size]; }
 void Deallocate(void* obj, void* ptr)  { delete[] static_cast<uint8_t*>(ptr); }

 bool Initialize()
 {
   criErr_SetCallback(ErrorCallback);
   criAtomEx_SetUserAllocator(Allocate, Deallocate, nullptr);

   CriFsConfig fsConfig;
   CriAtomExConfig_WASAPI libConfig;
   criFs_SetDefaultConfig(&fsConfig);
   criAtomEx_SetDefaultConfig_WASAPI(&libConfig);
   fsConfig.num_loaders = 64;
   libConfig.atom_ex.fs_config = &fsConfig;
   libConfig.atom_ex.max_virtual_voices = 64;
   criAtomEx_Initialize_WASAPI(&libConfig, nullptr, 0);

   dbas = criAtomDbas_Create(nullptr, nullptr, 0);
   if (criAtomEx_RegisterAcfFile(nullptr, "Res/Audio/SampleSound.acf", nullptr, 0) == CRI_FALSE) {
     return false;
   }
   criAtomEx_AttachDspBusSetting(CRI_SAMPLESOUND_ACF_DSPSETTING_DSPBUSSETTING_0, nullptr, 0);

   CriAtomExStandardVoicePoolConfig config;
   criAtomExVoicePool_SetDefaultConfigForStandardVoicePool(&config);
   config.num_voices = 16;
   config.player_config.streaming_flag = CRI_TRUE;
   config.player_config.max_sampling_rate = 48000 * 2;
   voicePool = criAtomExVoicePool_AllocateStandardVoicePool(&config, nullptr, 0);
   acb = criAtomExAcb_LoadAcbFile(nullptr, "Res/Audio/SampleCueSheet.acb", nullptr, nullptr, nullptr, 0);
   for (auto& e : player) {
     e = criAtomExPlayer_Create(nullptr, nullptr, 0);
   }
   return true;
 }

 void Play(int playerId, int cueId)
 {
   criAtomExPlayer_SetCueId(player[playerId], acb, cueId);
   criAtomExPlayer_Start(player[playerId]);
 }

 void Update()
 {
   criAtomEx_ExecuteMain();
 }

 void Destroy()
 {
   for (auto& e : player) {
     criAtomExPlayer_Destroy(e);
   }
   criAtomExAcb_Release(acb);
   criAtomExVoicePool_Free(voicePool);
   criAtomEx_UnregisterAcf();
   criAtomEx_Finalize_WASAPI();
 }

} //namespace Audio
