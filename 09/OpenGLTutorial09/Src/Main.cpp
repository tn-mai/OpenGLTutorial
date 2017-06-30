/**
* @file main.cpp
*/
#include "GLFWEW.h"
#include "GameEngine.h"
#include <glm/gtc/matrix_transform.hpp>
#include <string.h>

/**
* 敵弾の更新.
*/
void UpdateEnemyShot(Entity::Entity& entity, void* ubo, double delta, const glm::mat4& matView, const glm::mat4& matProj)
{
  const glm::vec3 pos = entity.Position();
  if (pos.x < -40.0f || pos.x > 40.0f || pos.z < -2.0f || pos.z > 40.0f) {
    GameEngine& game = GameEngine::Instance();
    game.RemoveEntity(&entity);
    return;
  }
  DefaultUpdateVertexData(entity, ubo, delta, matView, matProj);
}

/**
* 敵の更新.
*/
struct UpdateToroid {
  explicit UpdateToroid(const Entity::Entity* t) : target(t)
  {
    GameEngine& game = GameEngine::Instance();
  }

  void operator()(Entity::Entity& entity, void* ubo, double delta, const glm::mat4& matView, const glm::mat4& matProj)
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
        if (Entity::Entity* p = game.AddEntity(pos, "Spario", "Res/Model/Toroid.bmp", UpdateEnemyShot)) {
          glm::vec3 targetPos = target->Position();
          targetPos.x += std::uniform_real_distribution<float>(-2, 2)(game.Rand());
          targetPos.z += std::uniform_real_distribution<float>(-2, 2)(game.Rand());
          glm::vec3 vec = glm::normalize(targetPos - entity.Position()) * 4.0f;
          p->Velocity(vec);
        }
      }
      entity.Velocity(v);
      glm::quat q = glm::rotate(glm::quat(), -accelX * 2.0f, glm::vec3(0, 0, 1));
      entity.Rotation(q * entity.Rotation());
    } else {
      float rot = glm::angle(entity.Rotation());
      rot += glm::radians(35.0f) * static_cast<float>(delta);
      if (rot > glm::pi<float>() * 2.0f) {
        rot -= glm::pi<float>() * 2.0f;
      }
      entity.Rotation(glm::angleAxis(rot, glm::vec3(0, 1, 0)));
    }
    DefaultUpdateVertexData(entity, ubo, delta, matView, matProj);
  }
  const Entity::Entity* target;
  bool isEscape = false;
  float accelX = 0;
};

/**
* 自機の弾の更新.
*/
void UpdatePlayerShot(Entity::Entity& entity, void* ubo, double delta, const glm::mat4& matView, const glm::mat4& matProj)
{
  glm::vec3 pos = entity.Position();
  if (std::abs(pos.x) > 40 || pos.z < -4 || pos.z > 40.0f) {
    GameEngine& game = GameEngine::Instance();
    game.RemoveEntity(&entity);
    return;
  }
  DefaultUpdateVertexData(entity, ubo, delta, matView, matProj);
}

/**
* 自機の更新
*/
struct UpdatePlayer {
  void operator()(Entity::Entity& entity, void* ubo, double delta, const glm::mat4& matView, const glm::mat4& matProj)
  {
    const GamePad gamepad = GLFWEW::Window::Instance().GetGamePad(0);
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
      vec = glm::normalize(vec) * static_cast<float>(2 * delta);
      const glm::vec3 pos3 = entity.Position();
      glm::vec2 pos2 = glm::vec2(pos3.x, pos3.z) + vec;
      pos2 = glm::min(glm::vec2(11, 20), glm::max(pos2, glm::vec2(-11, 1)));
      entity.Position(glm::vec3(pos2.x, pos3.y, pos2.y));
    }
    entity.Rotation(glm::quat(glm::vec3(0, glm::radians(180.0f), rotZ)));

    GameEngine& game = GameEngine::Instance();
    if (gamepad.buttons & GamePad::A) {
      shotInterval -= delta;
      if (shotInterval <= 0) {
        shotInterval = 1.0;
        glm::vec3 pos = entity.Position();
        pos.x -= 0.3f;
        for (int i = 0; i < 2; ++i) {
          if (Entity::Entity* p = game.AddEntity(pos, "NormalShot", "Res/Model/Player.bmp", UpdatePlayerShot)) {
            p->Velocity(glm::vec3(0, 0, 16));
            p->Scale(glm::vec3(0.25f, 0.25f, 0.25f));
            p->Rotation(glm::angleAxis(3.14f, glm::vec3(0, 1, 0)));
            p->Id(2);
          }
          pos.x += 0.6f;
        }
      }
    } else {
      shotInterval = 0;
    }

    DefaultUpdateVertexData(entity, ubo, delta, matView, matProj);
  }
  double shotInterval = 0;
};

/**
* 爆発の更新.
*/
struct UpdateBlast {
  void operator()(Entity::Entity& entity, void* ubo, double delta, const glm::mat4& matView, const glm::mat4& matProj) {
    timer += delta;
    if (timer >= 2) {
      GameEngine::Instance().RemoveEntity(&entity);
      return;
    }
    entity.Scale(glm::vec3(1 + timer));
    static const glm::vec4 color[] = {
      glm::vec4(1.0f, 1.0f, 0.75f, 1),
      glm::vec4(1.0f, 0.5f, 0.1f, 1),
      glm::vec4(0.25f, 0.1f, 0.1f, 0),
      glm::vec4(0.25f, 0.1f, 0.1f, 1),
    };
    const double tmp = timer * 1;
    const float fract = std::fmod(tmp, 1);
    const glm::vec4 col0 = color[static_cast<int>(tmp)];
    const glm::vec4 col1 = color[static_cast<int>(tmp) + 1];
    const glm::vec4 newColor = col0 * glm::vec4(1 - fract) + col1 * glm::vec4(fract);
    entity.Color(newColor);
    glm::vec3 euler = glm::eulerAngles(entity.Rotation());
    euler.y += glm::radians(30.0f) * static_cast<float>(delta);
    entity.Rotation(glm::quat(euler));

    DefaultUpdateVertexData(entity, ubo, delta, matView, matProj);
  }
  double timer = 0;
};

/**
* ゲーム状態の更新.
*/
struct Update {
  Update(Entity::Entity* p0, Entity::Entity* p1) : pPlayer(p0), pSpaceSphere(p1) {}
  void operator()(double delta)
  {
    GameEngine& game = GameEngine::Instance();

    const float posZ = -8.28f;
    const float lookAtZ = 20.0f - 8.28f;
    static float degree = 0.0f;
    static double poppingTimer = 0.0f;

    game.SetView(
      glm::vec4(0, 20, posZ, 1),
      glm::vec3(0, 0, lookAtZ),
      glm::vec3(0, 0, 1)
    );

    game.AmbientLight(glm::vec4(0.05f, 0.1f, 0.2f, 1));
    game.Light(0, { glm::vec4(40, 100, 10, 1), glm::vec4(12000, 12000, 12000, 1) } );

    glm::vec3 rotSpace = glm::eulerAngles(pSpaceSphere->Rotation());
    rotSpace.x += static_cast<float>(glm::radians(0.25) * delta);
    pSpaceSphere->Rotation(rotSpace);

    std::uniform_int_distribution<> distributerX(-12, 12);
    std::uniform_int_distribution<> distributerZ(40, 44);
    poppingTimer -= delta;
    if (poppingTimer <= 0) {
      const std::uniform_real_distribution<> rndPoppingTime(8.0, 16.0);
      const std::uniform_int_distribution<> rndPoppingCount(1, 5);
      for (int i = rndPoppingCount(game.Rand()); i > 0; --i) {
        const glm::vec3 pos(distributerX(game.Rand()), 0, distributerZ(game.Rand()));
        if (Entity::Entity* p = game.AddEntity(pos, "Toroid", "Res/Model/Toroid.bmp", UpdateToroid(pPlayer))
          ) {
          p->Id(1);
          p->Velocity(glm::vec3(pos.x < 0 ? 1.0f : -1.0f, 0, -4));
        }
      }
      poppingTimer = rndPoppingTime(game.Rand());
    }

    std::vector<Entity::Entity*> enemyList;
    std::vector<Entity::Entity*> playerShotList;
    Entity::Buffer::Iterator end = game.EndEntity();
    for (Entity::Buffer::Iterator itr = game.BeginEntity(); itr != end; ++itr) {
      switch (itr->Id()) {
      case 1: enemyList.push_back(&*itr); break;
      case 2: playerShotList.push_back(&*itr); break;
      }
    }
    int firstEnemy = 0;
    for (auto shot : playerShotList) {
      const glm::vec3 ltShot = shot->Position() - glm::vec3(0.5f, 0.5f, 1.0f);
      const glm::vec3 rbShot = shot->Position() + glm::vec3(0.5f, 0.5f, 1.0f);
      const auto end = enemyList.end();
      for (auto enemy = enemyList.begin() + firstEnemy; enemy != end; ++enemy) {
        const glm::vec3 lt = (*enemy)->Position() - glm::vec3(1.0f, 1.0f, 1.0f);
        const glm::vec3 rb = (*enemy)->Position() + glm::vec3(1.0f, 1.0f, 1.0f);
        if (lt.x >= rbShot.x || rb.x <= ltShot.x || lt.z >= rbShot.z || rb.z <= rbShot.z) {
          continue;
        }
        Entity::Entity* p = game.AddEntity((*enemy)->Position(), "Blast", "Res/Model/Toroid.bmp", UpdateBlast());
        static const std::uniform_real_distribution<float> rotRange(0.0f, 359.0f);
        p->Rotation(glm::quat(glm::vec3(0, rotRange(game.Rand()), 0)));
        game.RemoveEntity(*enemy);
        game.RemoveEntity(shot);
        *enemy = enemyList[firstEnemy];
        ++firstEnemy;
        break;
      }
    }
  }
  Entity::Entity* pPlayer = nullptr;
  Entity::Entity* pSpaceSphere = nullptr;
};

/// エントリーポイント.
int main()
{
  GLFWEW::Window& window = GLFWEW::Window::Instance();
  if (!window.Init(800, 600, "OpenGL Tutorial")) {
    return 1;
  }
  GameEngine& game = GameEngine::Instance();
  if (!game.Init()) {
    return 1;
  }

  game.LoadTextureFromFile("Res/Model/Toroid.bmp");
  game.LoadTextureFromFile("Res/Model/Player.bmp");
  game.LoadTextureFromFile("Res/Model/SpaceSphere.bmp", GL_REPEAT);
  game.LoadMeshFromFile("Res/Model/Player.fbx");
  game.LoadMeshFromFile("Res/Model/Toroid.fbx");
  game.LoadMeshFromFile("Res/Model/Blast.fbx");
  game.LoadMeshFromFile("Res/Model/SpaceSphere.fbx");

  Entity::Entity* p0 = game.AddEntity(glm::vec3(0, 0, 2), "Aircraft", "Res/Model/Player.bmp", UpdatePlayer());
  p0->Rotation(glm::rotate(glm::quat(), glm::radians(180.0f), glm::vec3(0, 1, 0)));
  p0->Scale(glm::vec3(0.25f));

  Entity::Entity* p1 =  game.AddEntity(glm::vec3(0, 0, 0), "SpaceSphere", "Res/Model/SpaceSphere.bmp", DefaultUpdateVertexData, false);
//  p1->Scale(glm::vec3(0.5f));

  game.SetUpdateFunc(Update(p0, p1));

  const double delta = 1.0 / 60.0;
  while (!window.ShouldClose()) {
    window.UpdateGamePad();
    game.Update(delta);
    game.Render();
    window.SwapBuffers();
  }

  return 0;
}