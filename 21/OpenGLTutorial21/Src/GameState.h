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

/// �^�C�g�����.
class Title
{
public:
  void operator()(double delta);
private:
  bool initial = true;
  float timer = 0;
};

/// ���C���Q�[�����.
class MainGame
{
public:
  MainGame();
  void operator()(double delta);
private:
  double interval = 0;

  int stageNo = 0;
  double stageTimer = -1;
};

} // namespace GameState

#endif // GAMESTATE_H_INCLUDED