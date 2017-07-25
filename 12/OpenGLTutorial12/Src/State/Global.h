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

/// �G���e�B�e�B�̏Փ˃O���[�vID.
enum EntityGroupId {
  EntityGroupId_Background,
  EntityGroupId_Player,
  EntityGroupId_PlayerShot,
  EntityGroupId_Enemy,
  EntityGroupId_EnemyShot,
  EntityGroupId_Others,
};

/// �Փ˃f�[�^���X�g.
static const Entity::CollisionData collisionDataList[] = {
  { glm::vec3(), glm::vec3() },
  { glm::vec3(-0.5f, -1.0f, -0.5f), glm::vec3(0.5f, 1.0f, 0.5f) },
  { glm::vec3(-0.5f, -0.5f, -1.0f), glm::vec3(0.5f, 0.5f, 1.0f) },
  { glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f) },
  { glm::vec3(-0.25f, -0.25f, -0.25f), glm::vec3(0.25f, 0.25f, 0.25f) },
};

} // namespace Global

#endif // APP_GLOBAL_H_INCLUDED
