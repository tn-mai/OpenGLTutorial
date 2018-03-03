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
* �I�[�f�B�I�V�X�e���p�G���[�R�[���o�b�N.
*/
void ErrorCallback(const CriChar8* errid, CriUint32 p1, CriUint32 p2,
  CriUint32* parray)
{
  const CriChar8* err = criErr_ConvertIdToMessage(errid, p1, p2);
  std::cerr << err << std::endl;
}

/**
* �I�[�f�B�I�V�X�e���p�A���P�[�^.
*/
void* Allocate(void* obj, CriUint32 size)
{
  return new uint8_t[size];
}

/**
* �I�[�f�B�I�V�X�e���p�f�A���P�[�^.
*/
void Deallocate(void* obj, void* ptr)
{
  delete[] static_cast<uint8_t*>(ptr);
}

/**
* �I�[�f�B�I�V�X�e��������������.
*
* @param acfPath    ACF�t�@�C���̃p�X.
* @param acbPath    ACB�t�@�C���̃p�X.
* @param awbPath    AWB�t�@�C���̃p�X.
* @param dspBusName D-BUS��.
* @param playerCount �Đ�����p�v���C���[��. 
*
* @retval true  ����������.
* @retval false ���������s.
*/
bool Initialize(const char* acfPath, const char* acbPath, const char* awbPath, const char* dspBusName, size_t playerCount)
{
  // �G���[�R�[���o�b�N�ƃ������Ǘ��֐���o�^����.
  criErr_SetCallback(ErrorCallback);
  criAtomEx_SetUserAllocator(Allocate, Deallocate, nullptr);

  // �������p�����[�^��ݒ肵��ADX2 LE������������.
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

  // �ݒ�t�@�C����ǂݍ���.
  if (criAtomEx_RegisterAcfFile(nullptr, acfPath, nullptr, 0) == CRI_FALSE) {
    return false;
  }
  criAtomEx_AttachDspBusSetting(dspBusName, nullptr, 0);

  // �Đ�����ݒ肷��.
  CriAtomExStandardVoicePoolConfig config;
  criAtomExVoicePool_SetDefaultConfigForStandardVoicePool(&config);
  config.num_voices = 16;
  config.player_config.streaming_flag = CRI_TRUE;
  config.player_config.max_sampling_rate = 48000 * 2;
  voicePool = criAtomExVoicePool_AllocateStandardVoicePool(&config, nullptr, 0);
  acb = criAtomExAcb_LoadAcbFile(nullptr, acbPath, nullptr, awbPath, nullptr, 0);

  // �Đ�����p�v���C���[���쐬����.
  playerList.resize(playerCount);
  for (auto& e : playerList) {
    e = criAtomExPlayer_Create(nullptr, nullptr, 0);
  }
  return true;
}

/**
* �I�[�f�B�I�V�X�e����j������.
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
* �I�[�f�B�I�V�X�e���̏�Ԃ��X�V����.
*/
void Update()
{
  criAtomEx_ExecuteMain();
}

/**
* �������Đ�����.
*
* @param playerId �Đ��Ɏg�p����v���C���[��ID.
* @param cueId    �Đ�����L���[��ID.
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
* �������~����.
*
* @param playerId �Đ����~����v���C���[��ID.
*/
void Stop(int playerId)
{
  if (playerId < 0 || playerId > static_cast<int>(playerList.size())) {
    return;
  }
  criAtomExPlayer_Stop(playerList[playerId]);
}

} // namespace Audio
