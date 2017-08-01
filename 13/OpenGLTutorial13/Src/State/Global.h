/**
* @file Globa.h
*/
#ifndef APP_GLOBAL_H_INCLUDED
#define APP_GLOBAL_H_INCLUDED
#include "../Entity.h"

namespace Global {

static const char varScore[] = "score";
static const char varAutoPilot[] = "auto_pilot";
static const char varInvinsibleSeconds[] = "invinsible_seconds";
static const char varPlayerStock[] = "player_stock";
static const char varEnemyLevel[] = "enemy_level";

/// エンティティの衝突グループID.
enum EntityGroupId {
  EntityGroupId_Background,
  EntityGroupId_Player,
  EntityGroupId_PlayerShot,
  EntityGroupId_Enemy,
  EntityGroupId_EnemyShot,
  EntityGroupId_Others,
};

} // namespace Global

#endif // APP_GLOBAL_H_INCLUDED
