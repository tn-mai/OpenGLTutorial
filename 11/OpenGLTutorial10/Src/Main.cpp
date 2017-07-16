/**
* @file main.cpp
*/
#include "GameEngine.h"
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <algorithm>

/// エンティティの衝突グループID.
enum EntityGroupId {
  EntityGroupId_Player,
  EntityGroupId_PlayerShot,
  EntityGroupId_Enemy,
  EntityGroupId_EnemyShot,
  EntityGroupId_Others,
};

/// 衝突データリスト.
static const Entity::CollisionData collisionDataList[] = {
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
        accelX = v.x * -0.025f;
        if (Entity::Entity* p = game.AddEntity(EntityGroupId_EnemyShot, pos, "Spario", "Res/Model/Toroid.bmp", UpdateEnemyShot)) {
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
          const glm::vec3 P0 = entity.Position();
          const glm::vec3 P1 = target->Position();
          const glm::vec3 V1 = target->Velocity();
          const float V0 = 4.0f;
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
          targetPos.x += static_cast<float>(std::normal_distribution<>(0, 1.5f)(game.Rand()));
          targetPos.z += static_cast<float>(std::normal_distribution<>(0, 1.5f)(game.Rand()));
          targetPos = glm::min(glm::vec3(11, 100, 20), glm::max(targetPos, glm::vec3(-11, -100, 1)));
          p->Velocity(glm::normalize(targetPos - P0) * 4.0f);
          p->Color(glm::vec4(6, 6, 6, 1));
          p->Collision(collisionDataList[EntityGroupId_EnemyShot]);
        }
      }
      entity.Velocity(v);
      glm::quat q = glm::rotate(glm::quat(), -accelX * 1.0f, glm::vec3(0, 0, 1));
      entity.Rotation(q * entity.Rotation());
    } else {
      float rot = glm::angle(entity.Rotation());
      rot += glm::radians(35.0f) * static_cast<float>(delta);
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
struct UpdatePlayer {
  void operator()(Entity::Entity& entity, double delta)
  {
    GameEngine& game = GameEngine::Instance();
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
      vec = glm::normalize(vec) * 2.0f;
    }
    entity.Velocity(glm::vec3(vec.x, 0, vec.y));
    entity.Rotation(glm::quat(glm::vec3(0, 0, rotZ)));

    glm::vec3 pos3 = entity.Position();
    pos3 = glm::min(glm::vec3(11, 100, 20), glm::max(pos3, glm::vec3(-11, -100, 1)));
    entity.Position(pos3);

    if (gamepad.buttons & GamePad::A) {
      shotInterval -= delta;
      if (shotInterval <= 0) {
        shotInterval = 1.0;
        glm::vec3 pos = entity.Position();
        pos.x -= 0.3f;
        for (int i = 0; i < 2; ++i) {
          if (Entity::Entity* p = game.AddEntity(EntityGroupId_PlayerShot, pos, "NormalShot", "Res/Model/Player.bmp", UpdatePlayerShot)) {
            p->Velocity(glm::vec3(0, 0, 16));
            p->Color(glm::vec4(3));
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
    if (timer >= 2) {
      GameEngine::Instance().RemoveEntity(&entity);
      return;
    }
    entity.Scale(glm::vec3(static_cast<float>(1 + timer)));
    static const glm::vec4 color[] = {
      glm::vec4(1.0f, 1.0f, 0.75f, 1) * 2.0f,
      glm::vec4(1.0f, 0.5f, 0.1f, 1) * 2.0f,
      glm::vec4(0.25f, 0.1f, 0.1f, 0) * 2.0f,
      glm::vec4(0.25f, 0.1f, 0.1f, 0) * 2.0f,
    };
    const double tmp = timer * 1;
    const float fract = static_cast<float>(std::fmod(tmp, 1));
    const glm::vec4 col0 = color[static_cast<int>(tmp)];
    const glm::vec4 col1 = color[static_cast<int>(tmp) + 1];
    const glm::vec4 newColor = col0 * glm::vec4(1 - fract) + col1 * glm::vec4(fract);
    entity.Color(newColor);
    glm::vec3 euler = glm::eulerAngles(entity.Rotation());
    euler.y += glm::radians(30.0f) * static_cast<float>(delta);
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
  }
  lhs.Destroy();
  rhs.Destroy();
}

/**
* ゲーム状態の更新.
*/
struct Update {
  void operator()(double delta)
  {
    GameEngine& game = GameEngine::Instance();

    if (!pPlayer) {
      pPlayer = game.AddEntity(EntityGroupId_Player, glm::vec3(0, 0, 2), "Aircraft", "Res/Model/Player.bmp", UpdatePlayer());
      pPlayer->Collision(collisionDataList[EntityGroupId_Player]);
    }
    if (!pSpaceSphere) {
      pSpaceSphere =  game.AddEntity(EntityGroupId_Others, glm::vec3(0, 0, 0), "SpaceSphere", "Res/Model/SpaceSphere.bmp", nullptr, false);
    }

    const float posZ = -8.28f;
    const float lookAtZ = 20.0f - 8.28f;
    static float degree = 0.0f;
    static double poppingTimer = 0.0f;

    game.Camera({ glm::vec4(0, 20, -8, 1), glm::vec3(0, 0, 12), glm::vec3(0, 0, 1) });

    game.AmbientLight(glm::vec4(0.05f, 0.1f, 0.2f, 1));
    game.Light(0, { glm::vec4(40, 100, 10, 1), glm::vec4(12000, 12000, 12000, 1) } );

    glm::vec3 rotSpace = glm::eulerAngles(pSpaceSphere->Rotation());
    rotSpace.x += static_cast<float>(glm::radians(0.25) * delta);
    pSpaceSphere->Rotation(rotSpace);
    game.FontScale(glm::vec2(4));
    //game.FontBorder(0.125f);
    //game.FontThickness(1);
    game.FontSubColor({ 255, 100, 20, 255 });
    game.AddString(glm::vec2(-0.45f, 0.1f), "demo play");

    std::uniform_int_distribution<> distributerX(-12, 12);
    std::uniform_int_distribution<> distributerZ(40, 44);
    poppingTimer -= delta;
    if (poppingTimer <= 0) {
      const std::uniform_real_distribution<> rndPoppingTime(8.0, 16.0);
      const std::uniform_int_distribution<> rndPoppingCount(1, 5);
      for (int i = rndPoppingCount(game.Rand()); i > 0; --i) {
        const glm::vec3 pos(distributerX(game.Rand()), 0, distributerZ(game.Rand()));
        if (Entity::Entity* p = game.AddEntity(EntityGroupId_Enemy, pos, "Toroid", "Res/Model/Toroid.bmp", UpdateToroid(pPlayer))) {
          p->Velocity(glm::vec3(pos.x < 0 ? 1.0f : -1.0f, 0, -4));
          p->Collision(collisionDataList[EntityGroupId_Enemy]);
        }
      }
      poppingTimer = rndPoppingTime(game.Rand());
    }
  }

  Entity::Entity* pPlayer = nullptr;
  Entity::Entity* pSpaceSphere = nullptr;
};

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
  game.LoadFontFromFile("Res/BaronNeue.fnt");

  game.CollisionHandler(EntityGroupId_PlayerShot, EntityGroupId_Enemy, &CollidePlayerShotAndEnemyHandler);
  game.UpdateFunc(Update());
  game.Run();

  return 0;
}