/**
* @file GameState.h
*/
#ifndef GAMESTATE_H_INCLUDED
#define GAMESTATE_H_INCLUDED
#include "Entity.h"

namespace GameState {

static const char varScore[] = "score";
static const char varAutoPilot[] = "auto_pilot";
static const char varInvinsibleSeconds[] = "invinsible_seconds";
static const char varPlayerStock[] = "player_stock";

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
  Entity::Entity* pSpaceSphere = nullptr;
  Entity::Entity* pPlayer = nullptr;
  double interval = 0;
};

} // namespace GameState

#endif // GAMESTATE_H_INCLUDED