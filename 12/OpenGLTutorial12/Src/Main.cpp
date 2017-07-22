/**
* @file main.cpp
*/
#include "GameEngine.h"
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <algorithm>
#include "../Res/Audio/SampleCueSheet.h"

static const char varScore[] = "score";
static const char varAutoPilot[] = "auto_pilot";
static const char varInvinsibleSeconds[] = "invinsible_seconds";
static const char varPlayerStock[] = "player_stock";

struct TitleState;
struct GameOverState
{
  void operator()(double delta);
  Entity::Entity* pSpaceSphere = nullptr;
  double timer = 0;
};

/// エンティティの衝突グループID.
enum EntityGroupId {
  EntityGroupId_Background,
  EntityGroupId_Player,
  EntityGroupId_PlayerShot,
  EntityGroupId_Enemy,
  EntityGroupId_EnemyShot,
  EntityGroupId_Others,
};

/// 衝突データリスト.
static const Entity::CollisionData collisionDataList[] = {
  { glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f) },
  { glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f) },
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

/**
* 敵の更新.
*/
struct UpdateToroid {
  explicit UpdateToroid(const Entity::Entity* t) : target(t)
  {
    GameEngine& game = GameEngine::Instance();
  }

  void operator()(Entity::Entity& entity, double delta)
  {
    GameEngine& game = GameEngine::Instance();
    glm::vec3 pos = entity.Position();
    if (pos.z < -2.0f || pos.x < -40.0f || pos.x > 40.0f) {
      game.RemoveEntity(&entity);
      return;
    } else if (isEscape || (pos.z < 35.0f && std::abs(pos.x - target->Position().x) <= 3.0f)) {
      isEscape = true;
      glm::vec3 v = entity.Velocity();
      if (accelX) {
        v.x += accelX;
        entity.Velocity(v);
      } else {
        accelX = v.x * -0.04f;
        const float V0 = 16.0f;
        if (Entity::Entity* p = game.AddEntity(EntityGroupId_EnemyShot, pos, "Spario", "Res/Model/Toroid.bmp", UpdateEnemyShot)) {
          glm::vec3 targetPos = CalcCatchUpPosition(entity.Position(), V0, target->Position(), target->Velocity());
          targetPos.x += static_cast<float>(std::normal_distribution<>(0, 1.5f)(game.Rand()));
          targetPos.z += static_cast<float>(std::normal_distribution<>(0, 1.5f)(game.Rand()));
          targetPos = glm::min(glm::vec3(11, 100, 20), glm::max(targetPos, glm::vec3(-11, -100, 1)));
          p->Velocity(glm::normalize(targetPos - entity.Position()) * V0);
          p->Color(glm::vec4(6, 6, 6, 1));
          p->Collision(collisionDataList[EntityGroupId_EnemyShot]);
        }
      }
      entity.Velocity(v);
      glm::quat q = glm::rotate(glm::quat(), -accelX * 0.2f, glm::vec3(0, 0, 1));
      entity.Rotation(q * entity.Rotation());
    } else {
      float rot = glm::angle(entity.Rotation());
      rot += glm::radians(120.0f) * static_cast<float>(delta);
      if (rot > glm::pi<float>() * 2.0f) {
        rot -= glm::pi<float>() * 2.0f;
      }
      entity.Rotation(glm::angleAxis(rot, glm::vec3(0, 1, 0)));
    }
  }
  const Entity::Entity* target;
  bool isEscape = false;
  float accelX = 0;
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
    double& invinsibleSeconds = game.UserVariable(varInvinsibleSeconds);
    if (invinsibleSeconds > 0) {
      invinsibleSeconds -= delta;
      if (invinsibleSeconds <= 0) {
        invinsibleSeconds = 0;
        entity.Color(glm::vec4(1));
      } else {
        entity.Color(glm::vec4(1, 1, 1, 0.5f));
      }
    }
    double& autoPilot = game.UserVariable(varAutoPilot);
    if (autoPilot) {
      glm::vec3 pos = entity.Position();
      pos.z += static_cast<float>(20 * delta);
      if (pos.z >= 2) {
        autoPilot = 0;
        pos.z = 2;
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
        shotInterval = 0.2;
        glm::vec3 pos = entity.Position();
        pos.x -= 0.3f;
        for (int i = 0; i < 2; ++i) {
          if (Entity::Entity* p = game.AddEntity(EntityGroupId_PlayerShot, pos, "NormalShot", "Res/Model/Player.bmp", UpdatePlayerShot)) {
            p->Velocity(glm::vec3(0, 0, 80));
            p->Color(glm::vec4(3));
            p->Collision(collisionDataList[EntityGroupId_PlayerShot]);
          }
          pos.x += 0.6f;
        }
        game.PlayAudio(0, CRI_SAMPLECUESHEET_PLAYERSHOT);
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
  if (Entity::Entity* p = game.AddEntity(EntityGroupId_Others, rhs.Position(), "Blast", "Res/Model/Toroid.bmp", UpdateBlast())) {
    static const std::uniform_real_distribution<float> rotRange(0.0f, 359.0f);
    p->Rotation(glm::quat(glm::vec3(0, rotRange(game.Rand()), 0)));
    game.UserVariable(varScore) += 100;
    game.PlayAudio(1, CRI_SAMPLECUESHEET_BOMB);
  }
  lhs.Destroy();
  rhs.Destroy();
}

/**
* 自機と敵または敵の弾の衝突処理.
*/
void PlayerAndEnemyShotCollisionHandler(Entity::Entity& lhs, Entity::Entity& rhs)
{
  GameEngine& game = GameEngine::Instance();
  if (game.UserVariable(varInvinsibleSeconds)) {
    return;
  }
  Entity::Entity& player = lhs.GroupId() == EntityGroupId_Player ? lhs : rhs;
  Entity::Entity& enemy = lhs.GroupId() != EntityGroupId_Player ? lhs : rhs;
  if (Entity::Entity* p = game.AddEntity(EntityGroupId_Others, player.Position(), "Blast", "Res/Model/Toroid.bmp", UpdateBlast())) {
    static const std::uniform_real_distribution<float> rotRange(0.0f, 359.0f);
    p->Rotation(glm::quat(glm::vec3(0, rotRange(game.Rand()), 0)));
    game.PlayAudio(0, CRI_SAMPLECUESHEET_BOMB);
  }
  if (enemy.GroupId() == EntityGroupId_Enemy) {
    if (Entity::Entity* p = game.AddEntity(EntityGroupId_Others, enemy.Position(), "Blast", "Res/Model/Toroid.bmp", UpdateBlast())) {
      static const std::uniform_real_distribution<float> rotRange(0.0f, 359.0f);
      p->Rotation(glm::quat(glm::vec3(0, rotRange(game.Rand()), 0)));
      game.PlayAudio(1, CRI_SAMPLECUESHEET_BOMB);
    }
  }
  enemy.Destroy();

  double& playerStock = game.UserVariable(varPlayerStock);
  if (playerStock > 0) {
    playerStock -= 1;
    player.Position(glm::vec3(0, 0, -40));
    player.Velocity(glm::vec3(0));
    game.UserVariable(varAutoPilot) = 1;
    game.UserVariable(varInvinsibleSeconds) = 5;
  } else {
    game.UpdateFunc(GameOverState());
  }
}

/**
* ゲーム状態の更新.
*/
struct Update
{
  Update()
  {
    GameEngine& game = GameEngine::Instance();
    game.UserVariable(varScore) = 0;
    game.UserVariable(varAutoPilot) = 1;
    game.UserVariable(varInvinsibleSeconds) = 0;
    game.UserVariable(varPlayerStock) = 2;
  }
  ~Update()
  {
  }

  void operator()(double delta)
  {
    GameEngine& game = GameEngine::Instance();

    if (!pPlayer) {
      pPlayer = game.AddEntity(EntityGroupId_Player, glm::vec3(0, 0, -10), "Aircraft", "Res/Model/Player.bmp", UpdatePlayer());
      pPlayer->Collision(collisionDataList[EntityGroupId_Player]);
    }
    if (!pSpaceSphere) {
      pSpaceSphere =  game.AddEntity(EntityGroupId_Background, glm::vec3(0, 0, 0), "SpaceSphere", "Res/Model/SpaceSphere.bmp", nullptr, false);
    }

    const float posZ = -8.28f;
    const float lookAtZ = 20.0f - 8.28f;
    static float degree = 0.0f;
    static double poppingTimer = 0.0f;

    game.Camera({ glm::vec4(0, 20, -8, 1), glm::vec3(0, 0, 12), glm::vec3(0, 0, 1) });

    game.AmbientLight(glm::vec4(0.05f, 0.1f, 0.2f, 1));
    game.Light(0, { glm::vec4(40, 100, 10, 1), glm::vec4(12000, 12000, 12000, 1) } );

    glm::vec3 rotSpace = glm::eulerAngles(pSpaceSphere->Rotation());
    rotSpace.x += static_cast<float>(glm::radians(2.5) * delta);
    pSpaceSphere->Rotation(rotSpace);

    std::uniform_int_distribution<> distributerX(-12, 12);
    std::uniform_int_distribution<> distributerZ(40, 44);
    poppingTimer -= delta;
    if (poppingTimer <= 0) {
      const std::uniform_real_distribution<> rndPoppingTime(2.0, 6.0);
      const std::uniform_int_distribution<> rndPoppingCount(1, 5);
      for (int i = rndPoppingCount(game.Rand()); i > 0; --i) {
        const glm::vec3 pos(distributerX(game.Rand()), 0, distributerZ(game.Rand()));
        if (Entity::Entity* p = game.AddEntity(EntityGroupId_Enemy, pos, "Toroid", "Res/Model/Toroid.bmp", UpdateToroid(pPlayer))) {
          p->Velocity(glm::vec3(pos.x < 0 ? 4.0f : -4.0f, 0, -16));
          p->Collision(collisionDataList[EntityGroupId_Enemy]);
        }
      }
      poppingTimer = rndPoppingTime(game.Rand());
    }

    char str[16];
    snprintf(str, 16, "%08.0f", game.UserVariable(varScore));
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
  }

  Entity::Entity* pPlayer = nullptr;
  Entity::Entity* pSpaceSphere = nullptr;
};

/**
* タイトル画面.
*/
struct TitleState
{
  void operator()(double delta)
  {
    GameEngine& game = GameEngine::Instance();
    game.Camera({ glm::vec4(0, 20, -8, 1), glm::vec3(0, 0, 12), glm::vec3(0, 0, 1) });

    if (!pSpaceSphere) {
      pSpaceSphere =  game.AddEntity(EntityGroupId_Others, glm::vec3(0, 0, 0), "SpaceSphere", "Res/Model/SpaceSphere.bmp", nullptr, false);
    }
    glm::vec3 rotSpace = glm::eulerAngles(pSpaceSphere->Rotation());
    rotSpace.x += static_cast<float>(glm::radians(2.5) * delta);
    pSpaceSphere->Rotation(rotSpace);

    game.FontPropotional(true);
    game.FontThickness(0.25f);
    game.FontBorder(0.25f);
    game.FontColor({ 0.9f, 0.95f, 1.0f, 1.0f });
    game.FontSubColor({ 0.1f, 0.5f, 0.8f, 0.8f });
    game.FontScale(glm::vec2(2));
    game.AddString(glm::vec2(-0.25f, 0.125f), "star fighter");
    game.FontScale(glm::vec2(0.5f));
    game.FontBorder(0.0f);
    game.AddString(glm::vec2(0.1f, -0.05f), "the ultimate in manned-fighter");
    if (game.GetGamePad(0).buttonDown & (GamePad::A | GamePad::B | GamePad::START)) {
      game.RemoveEntity(pSpaceSphere);
      game.UpdateFunc(Update());
      game.PlayAudio(1, CRI_SAMPLECUESHEET_START);
    }
  }
  Entity::Entity* pSpaceSphere = nullptr;
};

/**
* ゲームオーバー画面.
*/
void GameOverState::operator()(double delta)
{
  timer += delta;

  GameEngine& game = GameEngine::Instance();
  game.Camera({ glm::vec4(0, 20, -8, 1), glm::vec3(0, 0, 12), glm::vec3(0, 0, 1) });

  if (!pSpaceSphere) {
    pSpaceSphere = game.AddEntity(EntityGroupId_Others, glm::vec3(0, 0, 0), "SpaceSphere", "Res/Model/SpaceSphere.bmp", nullptr, false);
  }
  glm::vec3 rotSpace = glm::eulerAngles(pSpaceSphere->Rotation());
  rotSpace.x += static_cast<float>(glm::radians(2.5) * delta);
  pSpaceSphere->Rotation(rotSpace);

  const float alpha = static_cast<float>(std::min(1.0, timer * 0.2));

  game.FontPropotional(true);
  game.FontThickness(0.25f);
  game.FontBorder(0.25f);
  game.FontColor({ 0.9f, 0.25f, 0.25f, 1.0f * alpha });
  game.FontSubColor({ 1.0f, 1.0f, 1.0f, 1.0f * alpha });
  game.FontScale(glm::vec2(2));
  game.AddString(glm::vec2(-0.5f, 0.125f), "game over");
  game.FontScale(glm::vec2(0.5f));
  if (timer >= 2.0 && (game.GetGamePad(0).buttonDown & (GamePad::A | GamePad::B | GamePad::START))) {
    game.RemoveEntity(pSpaceSphere);
    game.UpdateFunc(TitleState());
    game.PlayAudio(1, CRI_SAMPLECUESHEET_START);
  }
}


/// エントリーポイント.
int main()
{
  GameEngine& game = GameEngine::Instance();
  if (!game.Init(800, 600, "OpenGL Tutorial")) {
    return 1;
  }

  game.LoadTextureFromFile("Res/Model/Toroid.bmp");
  game.LoadTextureFromFile("Res/Model/Player.bmp");
  game.LoadTextureFromFile("Res/Model/SpaceSphere.bmp", GL_REPEAT);
  game.LoadMeshFromFile("Res/Model/Player.fbx");
  game.LoadMeshFromFile("Res/Model/Toroid.fbx");
  game.LoadMeshFromFile("Res/Model/Blast.fbx");
  game.LoadMeshFromFile("Res/Model/SpaceSphere.fbx");
  game.LoadFontFromFile("Res/Font.fnt");

  game.CollisionHandler(EntityGroupId_PlayerShot, EntityGroupId_Enemy, &CollidePlayerShotAndEnemyHandler);
  game.CollisionHandler(EntityGroupId_Player, EntityGroupId_Enemy, &PlayerAndEnemyShotCollisionHandler);
  game.CollisionHandler(EntityGroupId_Player, EntityGroupId_EnemyShot, &PlayerAndEnemyShotCollisionHandler);
  game.UpdateFunc(TitleState());
  game.Run();

  return 0;
}