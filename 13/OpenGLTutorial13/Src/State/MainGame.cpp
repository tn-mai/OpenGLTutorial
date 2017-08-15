/**
* @file MainGame.cpp
*/
#include "MainGame.h"
#include "GameOver.h"
#include "Global.h"
#include "../GameEngine.h"
#include "../../Res/Audio/SampleCueSheet.h"
#include <algorithm>

namespace State {

/// 衝突データリスト.
static const Entity::CollisionData collisionDataList[] = {
  {},
  { glm::vec3(-0.5f, -1.0f, -0.5f), glm::vec3(0.5f, 1.0f, 0.5f) },
  { glm::vec3(-0.5f, -0.5f, -1.0f), glm::vec3(0.5f, 0.5f, 1.0f) },
  { glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f) },
  { glm::vec3(-0.25f, -0.25f, -0.25f), glm::vec3(0.25f, 0.25f, 0.25f) },
};

/**
* 敵弾の更新.
*/
void UpdateEnemyShot(Entity::Entity& entity, double delta)
{
  const glm::vec3 pos = entity.Position();
  if (pos.x < -40.0f || pos.x > 40.0f || pos.z < -2.0f || pos.z > 40.0f) {
    GameEngine& game = GameEngine::Instance();
    game.RemoveEntity(&entity);
    return;
  }
  glm::vec3 rot = glm::eulerAngles(entity.Rotation());
  rot.z += glm::radians(90.0f) * static_cast<float>(delta);
  entity.Rotation(glm::quat(rot));
}

/**
* 移動目標に追いつく位置を計算する.
*
* @param follower       追従開始座標.
* @param followingSpeed 追従速度.
* @param target         目標の現在位置.
* @param targetVelocity 目標の移動速度.
*
* @return 追いつくことができればその位置、できなければtargetを返す.
*/
glm::vec3 CalcCatchUpPosition(const glm::vec3& follower, const float followingSpeed, const glm::vec3& target, const glm::vec3& targetVelocity)
{
  // V0x*t + P0x = V1x*t + P1x
  //   V0x = V1x + (P1x - P0x)/t
  // V0y*t + P0y = V1y*t + P1y
  //   (V0y - V1y)t = P1y - P0y
  //   t = (P1y - P0y)/(V0y - V1y)
  //   V0x = V1x + (P1x - P0x)/(P1y - P0y)(V0y - V1y)
  //
  // V0^2 = V0x^2 + V0y^2
  //   V0x^2 = V0^2 - V0y^2
  //   V0x=sqrt(V0^2 - V0y^2)
  //
  // sqrt(V0^2 - V0y^2) = V1x + (P1x - P0x)/(P1y - P0y)(V0y - V1y)
  // V0^2 - V0y^2 = (V1x + (P1x - P0x)/(P1y - P0y)(V0y - V1y))^2
  //            = (V1x - N*V1y + N*V0y)^2
  //            = (VN + N*V0y)^2
  //            = VN^2 + 2(VN*N*V0y) + (N*V0y)^2
  //
  // 0 = (N*V0y)^2 + 2(VN*N*V0y) + VN^2 - V0^2 + V0y^2
  // 0 = (N^2+1)V0y^2 + (2*VN*N)V0y + (VN^2 - V0^2)
  //
  const glm::vec3 P0 = follower;
  const glm::vec3 P1 = target;
  const glm::vec3 V1 = targetVelocity;
  const float V0 = followingSpeed;
  const float N = (P1.x - P0.x) / (P1.z - P0.z);
  const float VN = V1.x - N * V1.z;
  const float a = N * N + 1;
  const float b = 2 * VN * N;
  const float c =  VN * VN - V0 * V0;
  const float D = b * b - 4 * a * c;
  glm::vec3 targetPos = P1;
  if (D >= 0) {
    const float sq = std::sqrt(D);
    const float t0 = (P1.z - P0.z) / ((-b + sq) / (2 * a) - V1.z);
    const float t1 = (P1.z - P0.z) / ((-b - sq) / (2 * a) - V1.z);
    const float t = std::max(t0, t1);
    targetPos += V1 * t;
  }
  return targetPos;
}

enum class EnemyType
{
  Toroid,
  ToroidAcute,
  Cardioid,
};

/**
* 敵の更新.
*/
struct UpdateToroid {
  explicit UpdateToroid(const Entity::Entity* t, EnemyType tp, int level = 0) : target(t), type(tp)
  {
    GameEngine& game = GameEngine::Instance();
    shotInterval = std::max(0.1, 1.0 - (level % 20) * 0.05);
    shotCount = std::min(5, level / 20 + 1);
  }

  void operator()(Entity::Entity& entity, double delta)
  {
    const float V0 = 16.0f;

    GameEngine& game = GameEngine::Instance();
    glm::vec3 pos = entity.Position();
    if (pos.z < -2.0f || pos.x < -40.0f || pos.x > 40.0f) {
      game.RemoveEntity(&entity);
      return;
    } else if (isEscape || (pos.z < 35.0f && std::abs(pos.x - target->Position().x) <= 3.0f)) {
      isEscape = true;
      bool doShot = false;
      glm::vec3 v = entity.Velocity();
      if (accelX) {
        v.x += accelX * static_cast<float>(delta);
        entity.Velocity(v);
        shotTimer -= delta;
        if (shotTimer <= 0) {
          shotTimer = shotInterval;
          doShot = true;
        }
      } else {
        accelX = v.x * -8.0f;
        shotTimer = shotInterval;
        doShot = true;
      }
      entity.Velocity(v);
      glm::quat q = glm::rotate(glm::quat(), -accelX * 0.2f * static_cast<float>(delta), glm::vec3(0, 0, 1));
      entity.Rotation(q * entity.Rotation());

      const glm::bvec3 flags = glm::lessThan(pos, glm::vec3(12, 100, 40)) && glm::greaterThan(pos, glm::vec3(-12, -100, 0));
      if (doShot && glm::all(flags)) {
        glm::vec3 targetPos = CalcCatchUpPosition(entity.Position(), V0, target->Position(), target->Velocity());
        targetPos.x += static_cast<float>(std::normal_distribution<>(0, 1.5f)(game.Rand()));
        targetPos.z += static_cast<float>(std::normal_distribution<>(0, 1.5f)(game.Rand()));
        targetPos = glm::min(glm::vec3(11, 100, 20), glm::max(targetPos, glm::vec3(-11, -100, 1)));
        const glm::vec3 velocity = glm::normalize(targetPos - entity.Position()) * V0;
        static const float rotList[][2] = {
          { glm::radians(0.0f), glm::radians(0.0f) },
          { glm::radians(-15.0f), glm::radians(30.0f) },
          { glm::radians(-15.0f), glm::radians(15.0f) },
          { glm::radians(-30.0f), glm::radians(20.0f) },
          { glm::radians(-30.0f), glm::radians(15.0f) },
          { glm::radians(-30.0f), glm::radians(15.0f) },
        };
        float rot = rotList[shotCount - 1][0];
        for (int i = 0; i < shotCount; ++i) {
          if (Entity::Entity* p = game.AddEntity(Global::EntityGroupId_EnemyShot, pos, "Spario", "Res/Model/Toroid.dds", UpdateEnemyShot)) {
            p->Velocity(glm::rotate(glm::quat(glm::vec3(0, rot, 0)), velocity));
            p->Color(glm::vec4(3, 3, 3, 1));
            p->Collision(collisionDataList[Global::EntityGroupId_EnemyShot]);
          }
          rot += rotList[shotCount - 1][1];
        }
      }
    } else if (type == EnemyType::Toroid) {
      float rot = glm::angle(entity.Rotation());
      rot += glm::radians(120.0f) * static_cast<float>(delta);
      if (rot > glm::pi<float>() * 2.0f) {
        rot -= glm::pi<float>() * 2.0f;
      }
      entity.Rotation(glm::angleAxis(rot, glm::vec3(0, 1, 0)));
    }
  }
  const Entity::Entity* target;
  EnemyType type;
  bool isEscape = false;
  float accelX = 0;
  double shotInterval;
  int shotCount;
  double shotTimer;
};

/**
* 自機の弾の更新.
*/
void UpdatePlayerShot(Entity::Entity& entity, double delta)
{
  glm::vec3 pos = entity.Position();
  if (std::abs(pos.x) > 40 || pos.z < -4 || pos.z > 40.0f) {
    GameEngine& game = GameEngine::Instance();
    game.RemoveEntity(&entity);
    return;
  }
}

/**
* 自機の更新
*/
struct UpdatePlayer
{
  void operator()(Entity::Entity& entity, double delta)
  {
    GameEngine& game = GameEngine::Instance();
    double& invinsibleSeconds = game.UserVariable(Global::varInvinsibleSeconds);
    if (invinsibleSeconds > 0) {
      invinsibleSeconds -= delta;
      if (invinsibleSeconds <= 0) {
        invinsibleSeconds = 0;
        entity.Color(glm::vec4(1));
      } else {
        entity.Color(glm::vec4(1, 1, 1, 0.5f));
      }
    }
    double& autoPilot = game.UserVariable(Global::varAutoPilot);
    if (autoPilot) {
      glm::vec3 pos = entity.Position();
      pos.z += static_cast<float>(20 * delta);
      if (pos.z >= 2) {
        pos.z = 2;
        if (autoPilot == 1) {
          autoPilot = 0;
        }
      }
      entity.Position(pos);
      return;
    }
    const GamePad gamepad = game.GetGamePad(0);
    glm::vec2 vec;
    float rotZ = 0;
    if (gamepad.buttons & GamePad::DPAD_LEFT) {
      vec.x = 1;
      rotZ = -glm::radians(30.0f);
    } else if (gamepad.buttons & GamePad::DPAD_RIGHT) {
      vec.x = -1;
      rotZ = glm::radians(30.0f);
    }
    if (gamepad.buttons & GamePad::DPAD_UP) {
      vec.y = 1;
    } else if (gamepad.buttons & GamePad::DPAD_DOWN) {
      vec.y = -1;
    }
    const float lengthSq = glm::dot(vec, vec);
    if (lengthSq) {
      vec = glm::normalize(vec) * 12.0f;
    }
    entity.Velocity(glm::vec3(vec.x, 0, vec.y));
    entity.Rotation(glm::quat(glm::vec3(0, 0, rotZ)));

    glm::vec3 pos3 = entity.Position();
    pos3 = glm::min(glm::vec3(11, 100, 20), glm::max(pos3, glm::vec3(-11, -100, 1)));
    entity.Position(pos3);

    if (gamepad.buttons & GamePad::A) {
      shotInterval -= delta;
      if (shotInterval <= 0) {
        shotInterval = 0.125;
        game.PlayAudio(0, CRI_SAMPLECUESHEET_PLAYERSHOT);
        glm::vec3 pos = entity.Position();
        pos.x -= 0.3f;
        for (int i = 0; i < 2; ++i) {
          if (Entity::Entity* p = game.AddEntity(Global::EntityGroupId_PlayerShot, pos, "NormalShot", "Res/Model/Player.bmp", UpdatePlayerShot)) {
            p->Velocity(glm::vec3(0, 0, 80));
            p->Color(glm::vec4(3, 3, 3, 1));
            p->Collision(collisionDataList[Global::EntityGroupId_PlayerShot]);
          }
          pos.x += 0.6f;
        }
      }
    } else {
      shotInterval = 0;
    }
  }

  double shotInterval = 0;
};

/**
* 爆発の更新.
*/
struct UpdateBlast {
  void operator()(Entity::Entity& entity, double delta) {
    timer += delta;
    if (timer >= 0.5) {
      GameEngine::Instance().RemoveEntity(&entity);
      return;
    }
    const double tmp = timer * 4;
    entity.Scale(glm::vec3(static_cast<float>(1 + tmp)));
    static const glm::vec4 color[] = {
      glm::vec4(1.0f, 1.0f, 0.75f, 1) * 2.0f,
      glm::vec4(1.0f, 0.5f, 0.1f, 1) * 2.0f,
      glm::vec4(0.25f, 0.1f, 0.1f, 0) * 2.0f,
      glm::vec4(0.25f, 0.1f, 0.1f, 0) * 2.0f,
    };
    const float fract = static_cast<float>(std::fmod(tmp, 1));
    const glm::vec4 col0 = color[static_cast<int>(tmp)];
    const glm::vec4 col1 = color[static_cast<int>(tmp) + 1];
    const glm::vec4 newColor = col0 * glm::vec4(1 - fract) + col1 * glm::vec4(fract);
    entity.Color(newColor);
    glm::vec3 euler = glm::eulerAngles(entity.Rotation());
    euler.y += glm::radians(120.0f) * static_cast<float>(delta);
    entity.Rotation(glm::quat(euler));
  }
  double timer = 0;
};

/**
* 自機の弾と敵の衝突処理.
*/
void CollidePlayerShotAndEnemyHandler(Entity::Entity& lhs, Entity::Entity& rhs)
{
  GameEngine& game = GameEngine::Instance();
  if (Entity::Entity* p = game.AddEntity(Global::EntityGroupId_Others, rhs.Position(), "Blast", "Res/Model/Toroid.dds", UpdateBlast())) {
    static const std::uniform_real_distribution<float> rotRange(0.0f, 359.0f);
    p->Rotation(glm::quat(glm::vec3(0, rotRange(game.Rand()), 0)));
  }
  game.UserVariable(Global::varScore) += 100;
  game.PlayAudio(1, CRI_SAMPLECUESHEET_BOMB);
  lhs.Destroy();
  rhs.Destroy();
}

/**
* 自機と敵または敵の弾の衝突処理.
*/
void PlayerAndEnemyShotCollisionHandler(Entity::Entity& lhs, Entity::Entity& rhs)
{
  GameEngine& game = GameEngine::Instance();
  if (game.UserVariable(Global::varInvinsibleSeconds)) {
    return;
  }
  Entity::Entity& player = lhs.GroupId() == Global::EntityGroupId_Player ? lhs : rhs;
  Entity::Entity& enemy = lhs.GroupId() != Global::EntityGroupId_Player ? lhs : rhs;
  if (Entity::Entity* p = game.AddEntity(Global::EntityGroupId_Others, player.Position(), "Blast", "Res/Model/Toroid.dds", UpdateBlast())) {
    static const std::uniform_real_distribution<float> rotRange(0.0f, 359.0f);
    p->Rotation(glm::quat(glm::vec3(0, rotRange(game.Rand()), 0)));
    game.PlayAudio(0, CRI_SAMPLECUESHEET_BOMB);
  }
  if (enemy.GroupId() == Global::EntityGroupId_Enemy) {
    if (Entity::Entity* p = game.AddEntity(Global::EntityGroupId_Others, enemy.Position(), "Blast", "Res/Model/Toroid.dds", UpdateBlast())) {
      static const std::uniform_real_distribution<float> rotRange(0.0f, 359.0f);
      p->Rotation(glm::quat(glm::vec3(0, rotRange(game.Rand()), 0)));
      game.PlayAudio(1, CRI_SAMPLECUESHEET_BOMB);
    }
  }
  enemy.Destroy();

  double& playerStock = game.UserVariable(Global::varPlayerStock);
  playerStock -= 1;
  game.UserVariable(Global::varInvinsibleSeconds) = 5;
  player.Velocity(glm::vec3(0));
  game.UserVariable(Global::varAutoPilot) = 1;
  if (playerStock >= 0) {
    player.Position(glm::vec3(0, 0, -40));
  } else {
    player.Position(glm::vec3(0, 0, -400));
  }
}

const glm::vec3 cameraStart(5, -3, -10);
const glm::vec3 cameraEnd(0, 20, -8);
const glm::vec3 cameraTarget(0, 0, 12);
const double timerPeriod1 = 3;

MainGame::MainGame(Entity::Entity* p) : pSpaceSphere(p)
{
  GameEngine& game = GameEngine::Instance();
  game.UserVariable(Global::varScore) = 0;
  game.UserVariable(Global::varAutoPilot) = 2;
  game.UserVariable(Global::varInvinsibleSeconds) = 0;
  game.UserVariable(Global::varPlayerStock) = 2;
  game.UserVariable(Global::varEnemyLevel) = 0;
  game.CollisionHandler(Global::EntityGroupId_PlayerShot, Global::EntityGroupId_Enemy, &CollidePlayerShotAndEnemyHandler);
  game.CollisionHandler(Global::EntityGroupId_Player, Global::EntityGroupId_Enemy, &PlayerAndEnemyShotCollisionHandler);
  game.CollisionHandler(Global::EntityGroupId_Player, Global::EntityGroupId_EnemyShot, &PlayerAndEnemyShotCollisionHandler);

  game.AmbientLight(glm::vec4(0.05f, 0.1f, 0.2f, 1));
  game.Light(0, { glm::vec4(40, 100, 10, 1), glm::vec4(12000, 12000, 12000, 1) } );

  pPlayer = game.AddEntity(Global::EntityGroupId_Player, glm::vec3(0, 0, -20), "Aircraft", "Res/Model/Player.bmp", UpdatePlayer());
  glm::vec3 burnerPos = pPlayer->Position() + glm::vec3(-0.5f, 0, -1);
  for (auto& e : pPlayerBurner) {
    e = game.AddEntity(Global::EntityGroupId_Player, burnerPos, "NormalShot", "Res/Model/Player.bmp", UpdatePlayer());
    e->Color(glm::vec4(6, 6, 6, 0.75));
    burnerPos.x += 1.0f;
  }
  pPlayer->Collision(collisionDataList[Global::EntityGroupId_Player]);

  game.Camera({ cameraStart, pPlayer->Position(), glm::vec3(0, 0, 1) });
  game.PlayAudio(0, CRI_SAMPLECUESHEET_AFTERBURNER);
}

MainGame::~MainGame()
{
}

void MainGame::StartingDemo(double delta)
{
  GameEngine& game = GameEngine::Instance();

  timer += delta;
  const float ratio = static_cast<float>(timer * (1 / timerPeriod1));
  if (ratio <= 1) {
    const glm::vec3 pos = glm::mix(cameraStart, cameraEnd, glm::min(1.0f, ratio));
    const glm::vec3 target = glm::mix(pPlayer->Position(), cameraTarget, glm::clamp((ratio - 0.5f) * 2.0f, 0.0f, 1.0f));
    game.Camera({ pos, target, glm::vec3(0, 0, 1) });

    const glm::vec3 burnerScale(glm::min(1.0f, (1.0f - ratio) * 10.0f));
    for (auto& e : pPlayerBurner) {
      glm::vec3 pos = e->Position();
      pos.z = pPlayer->Position().z -1;
      e->Position(pos);
      e->Scale(burnerScale);
    }
  } else {
    for (auto& e : pPlayerBurner) {
      game.RemoveEntity(e);
      e = nullptr;
    }
    game.Camera({ cameraEnd, cameraTarget, glm::vec3(0, 0, 1) });
    game.UserVariable(Global::varAutoPilot) = 0;
    game.PlayAudio(2, CRI_SAMPLECUESHEET_BGM02);
    mode = Mode_Main;
  }
}

void MainGame::operator()(double delta)
{
  if (mode == Mode_StartingDemo) {
    StartingDemo(delta);
    return;
  }

  GameEngine& game = GameEngine::Instance();

  const float posZ = -8.28f;
  const float lookAtZ = 20.0f - 8.28f;
  static float degree = 0.0f;
  static double poppingTimer = 0.0f;

  game.Camera({ glm::vec4(0, 20, -8, 1), glm::vec3(0, 0, 12), glm::vec3(0, 0, 1) });

  if (game.UserVariable(Global::varPlayerStock) >= 0) {
    std::uniform_int_distribution<> distributerX(-12, 12);
    std::uniform_int_distribution<> distributerZ(40, 44);
    poppingTimer -= delta;
    if (poppingTimer <= 0) {
      int enemyLevel = static_cast<int>(game.UserVariable(Global::varEnemyLevel));
      const int enemyType = ((enemyLevel % 20) * 6 + 3) / 40;
      const std::uniform_real_distribution<> rndPoppingTime(2.0 - (enemyLevel % 20) / 10, 6.0 - (enemyLevel % 20) / 6);
      const std::uniform_int_distribution<> rndPoppingCount(1 + (enemyLevel % 20) / 8, 3 + (enemyLevel % 20)/ 3);
      for (int i = rndPoppingCount(game.Rand()); i > 0; --i) {
        const glm::vec3 pos(distributerX(game.Rand()), 0, distributerZ(game.Rand()));
        static const struct {
          const char* name;
          EnemyType type;
        } meshNameList[] = {
          { "Toroid", EnemyType::Toroid },
          { "Toroid.Acute", EnemyType::ToroidAcute },
          { "Cardioid", EnemyType::Cardioid },
        };
        if (Entity::Entity* p = game.AddEntity(Global::EntityGroupId_Enemy, pos, meshNameList[enemyType].name, "Res/Model/Toroid.dds", UpdateToroid(pPlayer, meshNameList[enemyType].type, enemyLevel))) {
          if (meshNameList[enemyType].type != EnemyType::Toroid) {
            p->Rotation(glm::vec3(0, glm::radians(180.0f), 0));
          }
          p->Velocity(glm::vec3(pos.x < 0 ? 4.0f : -4.0f, 0, -16));
          p->Collision(collisionDataList[Global::EntityGroupId_Enemy]);
        }
      }
      game.UserVariable(Global::varEnemyLevel) = std::min(100, enemyLevel + 1);
      poppingTimer = rndPoppingTime(game.Rand());
    }
  } else if (game.UserVariable(Global::varInvinsibleSeconds) <= 0) {
    pPlayer->Destroy();
    game.ClearCollisionHandlerList();
    game.StopAudio(2);
    game.UpdateFunc(GameOver(pSpaceSphere));
  }

  char str[16];
  snprintf(str, 16, "%08.0f", game.UserVariable(Global::varScore));
  game.FontPropotional(false);
  game.FontXAdvance(1.0f / 24.0f);
  game.FontScale(glm::vec2(1));
  game.FontColor(glm::vec4(1));
  game.FontThickness(0.5f);
  game.FontBorder(0.25f);
  game.FontSubColor({0.25f, 0.1f, 0.4f, 0.8f});
  game.AddString(glm::vec2(-0.2f, 1.0f), str);
  snprintf(str, 16, "%03.0f", game.Fps());
  game.AddString(glm::vec2(-0.95f, 1.0f), str);
  snprintf(str, 16, "level:%03.0f", game.UserVariable(Global::varEnemyLevel));
  game.AddString(glm::vec2(0.6f, 1.0f), str);
  snprintf(str, 16, "LEFT:%02.0f", std::max(0.0, game.UserVariable(Global::varPlayerStock)));
  game.AddString(glm::vec2(-0.975f, -0.9f), str);
}

} // namespace State