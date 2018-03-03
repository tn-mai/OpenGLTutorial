/**
* @file GameState.h
*/
#ifndef GAMESTATE_H_INCLUDED
#define GAMESTATE_H_INCLUDED
#include "Entity.h"

namespace GameState {

/// エンティティの衝突グループID.
enum EntityGroupId
{
  EntityGroupId_Background,
  EntityGroupId_Player,
  EntityGroupId_PlayerShot,
  EntityGroupId_Enemy,
  EntityGroupId_EnemyShot,
  EntityGroupId_Others
};

/// 音声プレイヤーのID.
enum AudioPlayerId
{
  AudioPlayerId_Shot, ///< 自機のショット音.
  AudioPlayerId_Bomb, ///< 爆発音.
  AudioPlayerId_BGM, ///<　BGM.
  AudioPlayerId_Max, ///< 再生制御用プレイヤーの数.

  AudioPlayerId_UI = AudioPlayerId_Shot, ///<　ユーザーインターフェイス操作音.
};

/// タイトル画面.
class Title
{
public:
  explicit Title(Entity::Entity* p = nullptr) : pSpaceSphere(p) {}
  void operator()(double delta);
private:
  Entity::Entity* pSpaceSphere = nullptr;
  float timer = 0;
};

/// メインゲーム画面.
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