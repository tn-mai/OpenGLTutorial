/**
* @file Audio.cpp
*/
#include "Audio.h"
#include <cri_adx2le.h>
#include <vector>
#include <iostream>
#include <cstdint>

namespace Audio {

CriAtomExVoicePoolHn voicePool;
CriAtomDbasId dbas = CRIATOMDBAS_ILLEGAL_ID;
CriAtomExAcbHn acb;
std::vector<CriAtomExPlayerHn> playerList;

/**
* オーディオシステム用エラーコールバック.
*/
void ErrorCallback(const CriChar8* errid, CriUint32 p1, CriUint32 p2,
  CriUint32* parray)
{
  const CriChar8* err = criErr_ConvertIdToMessage(errid, p1, p2);
  std::cerr << err << std::endl;
}

/**
* オーディオシステム用アロケータ.
*/
void* Allocate(void* obj, CriUint32 size)
{
  return new uint8_t[size];
}

/**
* オーディオシステム用デアロケータ.
*/
void Deallocate(void* obj, void* ptr)
{
  delete[] static_cast<uint8_t*>(ptr);
}

/**
* オーディオシステムを初期化する.
*
* @param acfPath    ACFファイルのパス.
* @param acbPath    ACBファイルのパス.
* @param awbPath    AWBファイルのパス.
* @param dspBusName D-BUS名.
* @param playerCount 再生制御用プレイヤー数. 
*
* @retval true  初期化成功.
* @retval false 初期化失敗.
*/
bool Initialize(const char* acfPath, const char* acbPath, const char* awbPath, const char* dspBusName, size_t playerCount)
{
  // エラーコールバックとメモリ管理関数を登録する.
  criErr_SetCallback(ErrorCallback);
  criAtomEx_SetUserAllocator(Allocate, Deallocate, nullptr);

  // 初期化パラメータを設定してADX2 LEを初期化する.
  CriFsConfig fsConfig;
  CriAtomExConfig_WASAPI libConfig;
  criFs_SetDefaultConfig(&fsConfig);
  criAtomEx_SetDefaultConfig_WASAPI(&libConfig);
  fsConfig.num_loaders = 64;
  fsConfig.max_path = 1024;
  libConfig.atom_ex.fs_config = &fsConfig;
  libConfig.atom_ex.max_virtual_voices = 64;
  criAtomEx_Initialize_WASAPI(&libConfig, nullptr, 0);
  dbas = criAtomDbas_Create(nullptr, nullptr, 0);

  // 設定ファイルを読み込む.
  if (criAtomEx_RegisterAcfFile(nullptr, acfPath, nullptr, 0) == CRI_FALSE) {
    return false;
  }
  criAtomEx_AttachDspBusSetting(dspBusName, nullptr, 0);

  // 再生環境を設定する.
  CriAtomExStandardVoicePoolConfig config;
  criAtomExVoicePool_SetDefaultConfigForStandardVoicePool(&config);
  config.num_voices = 16;
  config.player_config.streaming_flag = CRI_TRUE;
  config.player_config.max_sampling_rate = 48000 * 2;
  voicePool = criAtomExVoicePool_AllocateStandardVoicePool(&config, nullptr, 0);
  acb = criAtomExAcb_LoadAcbFile(nullptr, acbPath, nullptr, awbPath, nullptr, 0);

  // 再生制御用プレイヤーを作成する.
  playerList.resize(playerCount);
  for (auto& e : playerList) {
    e = criAtomExPlayer_Create(nullptr, nullptr, 0);
  }
  return true;
}

/**
* オーディオシステムを破棄する.
*/
void Destroy()
{
  for (auto& e : playerList) {
    if (e) {
      criAtomExPlayer_Destroy(e);
      e = nullptr;
    }
  }
  if (acb) {
    criAtomExAcb_Release(acb);
    acb = nullptr;
  }
  if (voicePool) {
    criAtomExVoicePool_Free(voicePool);
    voicePool = nullptr;
  }
  criAtomEx_UnregisterAcf();
  if (dbas != CRIATOMDBAS_ILLEGAL_ID) {
    criAtomDbas_Destroy(dbas);
    dbas = CRIATOMDBAS_ILLEGAL_ID;
  }
  criAtomEx_Finalize_WASAPI();
}

/**
* オーディオシステムの状態を更新する.
*/
void Update()
{
  criAtomEx_ExecuteMain();
}

/**
* 音声を再生する.
*
* @param playerId 再生に使用するプレイヤーのID.
* @param cueId    再生するキューのID.
*/
void Play(int playerId, int cueId)
{
  if (playerId < 0 || playerId > static_cast<int>(playerList.size())) {
    return;
  }
  criAtomExPlayer_SetCueId(playerList[playerId], acb, cueId);
  criAtomExPlayer_Start(playerList[playerId]);
}

/**
* 音声を停止する.
*
* @param playerId 再生を停止するプレイヤーのID.
*/
void Stop(int playerId)
{
  if (playerId < 0 || playerId > static_cast<int>(playerList.size())) {
    return;
  }
  criAtomExPlayer_Stop(playerList[playerId]);
}

} // namespace Audio
