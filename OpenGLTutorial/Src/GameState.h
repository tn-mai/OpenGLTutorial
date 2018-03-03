/**
* @file GameState.h
*/
#ifndef GAMESTATE_H_INCLUDED
#define GAMESTATE_H_INCLUDED
#include "Entity.h"

namespace GameState {

/// �G���e�B�e�B�̏Փ˃O���[�vID.
enum EntityGroupId
{
  EntityGroupId_Background,
  EntityGroupId_Player,
  EntityGroupId_PlayerShot,
  EntityGroupId_Enemy,
  EntityGroupId_EnemyShot,
  EntityGroupId_Others
};

/// �����v���C���[��ID.
enum AudioPlayerId
{
  AudioPlayerId_Shot, ///< ���@�̃V���b�g��.
  AudioPlayerId_Bomb, ///< ������.
  AudioPlayerId_BGM, ///<�@BGM.
  AudioPlayerId_Max, ///< �Đ�����p�v���C���[�̐�.

  AudioPlayerId_UI = AudioPlayerId_Shot, ///<�@���[�U�[�C���^�[�t�F�C�X���쉹.
};

/// �^�C�g�����.
class Title
{
public:
  explicit Title(Entity::Entity* p = nullptr) : pSpaceSphere(p) {}
  void operator()(double delta);
private:
  Entity::Entity* pSpaceSphere = nullptr;
  float timer = 0;
};

/// ���C���Q�[�����.
class MainGame
{
public:
  explicit MainGame(Entity::Entity* p);
  void operator()(double delta);
private:
  bool isInitialized = false;
  double interval = 0;
  Entity::Entity* pPlayer = nullptr;
  Entity::Entity* pSpaceSphere = nullptr;
};

} // namespace GameState

#endif // GAMESTATE_H_INCLUDED