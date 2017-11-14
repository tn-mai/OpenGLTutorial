/**
* @file Stage01.cpp
*/
#include "GameState.h"
#include "GameEngine.h"
#include "../Res/Audio/SampleCueSheet.h"
#include <algorithm>

namespace GameState {

/// 衝突形状リスト.
static const Entity::CollisionData collisionDataList[] = {
  {},
  { glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f) },
  { glm::vec3(-0.5f, -0.5f, -1.0f), glm::vec3(0.5f, 0.5f, 1.0f) },
  { glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f) },
  { glm::vec3(-0.25f, -0.25f, -0.25f), glm::vec3(0.25f, 0.25f, 0.25f) },
};

/**
* 敵の更新.
*/
struct UpdateToroid
{
  void operator()(Entity::Entity& entity, double delta)
  {
    // 範囲外に出たら削除する.
    const glm::vec3 pos = entity.Position();
    if (std::abs(pos.x) > 40.0f || std::abs(pos.z) > 40.0f) {
      GameEngine::Instance().RemoveEntity(&entity);
      return;
    }
    // 円盤を回転させる.
    float rot = glm::angle(entity.Rotation());
    rot += glm::radians(45.0f) * static_cast<float>(delta);
    if (rot > glm::pi<float>() * 2.0f) {
      rot -= glm::pi<float>() * 2.0f;
    }
    entity.Rotation(glm::angleAxis(rot, glm::vec3(0, 1, 0)));
  }
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
        game.PlayAudio(0, CRI_SAMPLECUESHEET_PLAYERSHOT);
        glm::vec3 pos = entity.Position();
        pos.x -= 0.3f;
        for (int i = 0; i < 2; ++i) {
          if (Entity::Entity* p = game.AddEntity(EntityGroupId_PlayerShot, pos, "NormalShot", "Res/Model/Player.bmp", UpdatePlayerShot)) {
            p->Velocity(glm::vec3(0, 0, 80));
            p->Color(glm::vec4(3, 3, 3, 1));
            p->Collision(collisionDataList[EntityGroupId_PlayerShot]);
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
    p->Color(glm::vec4(1.0f, 1.0f, 0.75f, 1) * 2.0f);
  }
  game.UserVariable(varScore) += 100;
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
  playerStock -= 1;
  game.UserVariable(varInvinsibleSeconds) = 5;
  player.Velocity(glm::vec3(0));
  game.UserVariable(varAutoPilot) = 1;
  if (playerStock >= 0) {
    player.Position(glm::vec3(0, 0, -40));
  } else {
    player.Position(glm::vec3(0, 0, -400));
  }
}

/**
* 背景球を更新する.
*/
static void UpdateSpaceSphere(Entity::Entity& entity, double delta)
{
  glm::vec3 rotSpace = glm::eulerAngles(entity.Rotation());
  rotSpace.x += static_cast<float>(glm::radians(2.5) * delta);
  entity.Rotation(rotSpace);
}

void UpdateLandscape(Entity::Entity& entity, double delta)
{
  entity.Position(entity.Position() + glm::vec3(0, 0, -2.0f * delta));
}
/**
*　コンストラクタ.
*/
MainGame::MainGame()
{
  GameEngine& game = GameEngine::Instance();
  game.CollisionHandler(EntityGroupId_PlayerShot, EntityGroupId_Enemy, &CollidePlayerShotAndEnemyHandler);
  game.CollisionHandler(EntityGroupId_Player, EntityGroupId_Enemy, &PlayerAndEnemyShotCollisionHandler);
  game.CollisionHandler(EntityGroupId_Player, EntityGroupId_EnemyShot, &PlayerAndEnemyShotCollisionHandler);
}

/**
* メインゲーム画面の更新.
*/
void MainGame::operator()(double delta)
{
  GameEngine& game = GameEngine::Instance();

  static const float stageTime = 120;
  static const float stageTitleTime = stageTime - 3;

  if (stageTimer < 0) {
    ++stageNo;
    stageTimer = stageTime;

    game.Camera({ glm::vec4(0, 20, -8, 1), glm::vec3(0, 0, 12), glm::vec3(0, 0, 1) });
    game.AmbientLight(glm::vec4(0.05f, 0.1f, 0.2f, 1));
    game.Light(0, { glm::vec4(40, 100, 10, 1), glm::vec4(12000, 12000, 12000, 1) } );
    game.KeyValue(0.24f);

    game.RemoveAllEntity();
    game.ClearLevel();

    game.LoadTextureFromFile("Res/Model/Player.bmp");
    game.LoadMeshFromFile("Res/Model/Player.fbx");

    game.LoadMeshFromFile("Res/Model/Toroid.fbx");
    game.LoadMeshFromFile("Res/Model/Blast.fbx");
    game.LoadTextureFromFile("Res/Model/Toroid.bmp");
    game.LoadTextureFromFile("Res/Model/Toroid.Normal.bmp");

    switch (stageNo) {
    case 1: {
      game.KeyValue(0.24f);
      game.LoadMeshFromFile("Res/Model/BG01.fbx");
      game.LoadTextureFromFile("Res/Model/BG01.Diffuse.bmp");
      game.LoadTextureFromFile("Res/Model/BG01.Normal.bmp");
      game.LoadTextureFromFile("Res/Model/Block.Base.Diffuse.bmp");
      game.LoadTextureFromFile("Res/Model/Block.Base.Normal.bmp");
      game.LoadTextureFromFile("Res/Model/Block.End.Diffuse.bmp");
      game.LoadTextureFromFile("Res/Model/Block.End.Normal.bmp");
      game.AddEntity(EntityGroupId_Others, glm::vec3(0, -60, 800), "Landscape01", "Res/Model/BG01.Diffuse.bmp", "Res/Model/BG01.Normal.bmp", &UpdateLandscape);
      for (int i = 0; i < 3; ++i) {
        auto p0 = game.AddEntity(EntityGroupId_Others, glm::vec3(3, -10, 30 + 50 * i), "Block.Base", "Res/Model/Block.Base.Diffuse.bmp", "Res/Model/Block.Base.Normal.bmp", UpdateLandscape);
        p0->Rotation(glm::vec3(0, 0, glm::radians(10.0f)));
        auto p1 = game.AddEntity(EntityGroupId_Others, glm::vec3(-3, -10, 30 + 50 * i), "Block.Base", "Res/Model/Block.Base.Diffuse.bmp", "Res/Model/Block.Base.Normal.bmp", UpdateLandscape);
        p1->Rotation(glm::vec3(0, glm::radians(180.0f), glm::radians(-10.0f)));
      }
      auto p0 = game.AddEntity(EntityGroupId_Others, glm::vec3(3, -10, 30 + 50 * 2), "Block.End", "Res/Model/Block.Base.Diffuse.bmp", "Res/Model/Block.Base.Normal.bmp", UpdateLandscape);
      p0->Rotation(glm::vec3(0, glm::radians(180.0f), glm::radians(190.0f)));
      auto p1 = game.AddEntity(EntityGroupId_Others, glm::vec3(-3, -10, 30 + 50 * 2), "Block.End", "Res/Model/Block.End.Diffuse.bmp", "Res/Model/Block.End.Normal.bmp", UpdateLandscape);
      p1->Rotation(glm::vec3(0, glm::radians(180.0f), glm::radians(-10.0f)));
      break;
    }
    default:
    case 2: {
      game.KeyValue(0.02f);
      game.LoadMeshFromFile("Res/Model/SpaceSphere.fbx");
      game.LoadTextureFromFile("Res/Model/SpaceSphere.bmp");
      game.AddEntity(EntityGroupId_Others, glm::vec3(0, 0, 0), "SpaceSphere", "Res/Model/SpaceSphere.bmp", &UpdateSpaceSphere, "NonLighting");
      break;
    }
    }

    auto pPlayer = game.AddEntity(EntityGroupId_Player, glm::vec3(0, 0, 2), "Aircraft", "Res/Model/Player.bmp", UpdatePlayer());
    pPlayer->Collision(collisionDataList[EntityGroupId_Player]);
  }
  stageTimer -= delta;
  if (stageTimer > stageTitleTime) {
    char str[32];
    snprintf(str, sizeof(str), "stage %02d", stageNo);
    const float timer = static_cast<float>(stageTimer) - stageTitleTime;
    float alpha = glm::clamp(timer, 0.0f, 1.0f);
    game.FontScale(glm::vec2(2));
    game.FontColor(glm::vec4(1, 1, 1, alpha));
    game.FontSubColor(glm::vec4(0.1f, 0.5f, 0.8f, 0.8f * alpha));
    game.AddString(glm::vec2(-0.25f, 0.125f), str);
  }

  std::uniform_int_distribution<> distributerX(-12, 12);
  std::uniform_int_distribution<> distributerZ(40, 44);
  interval -= delta;
  if (interval <= 0) {
    const std::uniform_real_distribution<> rndInterval(0.1, 1.0);
    const std::uniform_int_distribution<> rndAddingCount(1, 5);
    for (int i = rndAddingCount(game.Rand()); i > 0; --i) {
      const glm::vec3 pos(distributerX(game.Rand()), 0, distributerZ(game.Rand()));
      if (Entity::Entity* p = game.AddEntity(EntityGroupId_Enemy, pos, "Toroid", "Res/Model/Toroid.bmp", "Res/Model/Toroid.Normal.bmp", UpdateToroid())) {
        p->Velocity(glm::vec3(pos.x < 0 ? 3.0f : -3.0f, 0, -12.0f));
        p->Collision(collisionDataList[EntityGroupId_Enemy]);
      }
    }
    interval = rndInterval(game.Rand());
  }
}

} // namespace GameState