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
    entity.Parent()->RemoveEntity(&entity);
    return;
  }
  TransformationData data;
  data.matM = entity.TRSMatrix();
  data.matMVP = matProj * matView * data.matM;
  memcpy(ubo, &data, sizeof(data));
}

/**
* 敵の更新.
*/
struct UpdateToroid {
  UpdateToroid()
  {
    static const std::uniform_real_distribution<float> rndOffset(-5.0f, 2.0f);
    Game& game = Game::Instance();
    reversePoint = 20.0f + rndOffset(game.rand);
  }

  void operator()(Entity::Entity& entity, void* ubo, double delta, const glm::mat4& matView, const glm::mat4& matProj)
  {
    Game& game = Game::Instance();
    glm::vec3 pos = entity.Position();
    if (pos.z < -2.0f || pos.x < -40.0f || pos.x > 40.0f) {
      game.entityBuffer->RemoveEntity(&entity);
      return;
    } else if (pos.z < reversePoint) {
      glm::vec3 v = entity.Velocity();
      if (accelX) {
        v.x += accelX;
        entity.Velocity(v);
      } else {
        accelX = v.x * -0.05f;
        if (Entity::Entity* p = game.entityBuffer->AddEntity(pos, game.sampleMesh[1], entity.Texture(), entity.ShaderProgram(), UpdateEnemyShot)) {
          glm::vec3 target = entity.Position();
          target.x += std::uniform_real_distribution<float>(-2, 2)(game.rand);
          target.y += std::uniform_real_distribution<float>(-2, 2)(game.rand);
          glm::vec3 vec = glm::normalize(glm::vec3() - target) * 2.0f;
          p->Velocity(vec);
        }
      }
      entity.Velocity(v);
      glm::quat q = glm::rotate(glm::quat(), -accelX * 4.0f, glm::vec3(0, 0, 1));
      entity.Rotation(q * entity.Rotation());
    } else {
      float rot = glm::angle(entity.Rotation());
      rot += glm::radians(10.0f) * static_cast<float>(delta);
      if (rot > glm::pi<float>() * 2.0f) {
        rot = 0.0f;
      }
      entity.Rotation(glm::angleAxis(rot, glm::vec3(0, 1, 0)));
    }
    TransformationData data;
    data.matM = entity.TRSMatrix();
    data.matMVP = matProj * matView * data.matM;
    memcpy(ubo, &data, sizeof(data));
  }

  float reversePoint;
  float accelX = 0;
};

/**
*
*/
void Update(double delta)
{
  Game& game = Game::Instance();

  const float posZ = -8.28f;
  const float lookAtZ = 20.0f - 8.28f;
  static float degree = 0.0f;
  static double poppingTimer = 0.0f;
  //degree += 0.05f;
  if (degree >= 360.0f) { degree -= 360.0f; }
  game.viewPos = glm::rotate(glm::mat4(), glm::radians(degree), glm::vec3(0, 1, 0)) * glm::vec4(0, 20, posZ, 1);
  game.viewTarget = glm::vec3(0, 0, lookAtZ);
  game.viewUp = glm::vec3(0, 0, 1);

  game.lightData.ambientColor = glm::vec4(0.05f, 0.1f, 0.2f, 1);
  game.lightData.light[0].color = glm::vec4(18, 18, 18, 1);
  game.lightData.light[0].position = glm::vec4(2, 2, 2, 1);
  game.lightData.light[1].color = glm::vec4(0.125f, 0.125f, 0.05f, 1);
  game.lightData.light[1].position = glm::vec4(-0.2f, 0, 0.6f, 1);
  game.lightData.light[2].position = glm::vec4(15, 50, 10, 1);
  float lightDistance = glm::length(glm::vec3(game.lightData.light[2].position));
  lightDistance *= lightDistance;
  game.lightData.light[2].color = glm::vec4(lightDistance, lightDistance, lightDistance, 1);

  std::uniform_int_distribution<> distributerX(-15, 15);
  std::uniform_int_distribution<> distributerZ(40, 44);
  poppingTimer -= delta;
  if (poppingTimer <= 0) {
    const std::uniform_real_distribution<> rndPoppingTime(8.0, 16.0);
    const std::uniform_int_distribution<> rndPoppingCount(1, 5);
    for (int i = rndPoppingCount(game.rand); i > 0; --i) {
      const glm::vec3 pos(distributerX(game.rand), 0, distributerZ(game.rand));
      if (Entity::Entity* p = game.entityBuffer->AddEntity(
        pos, game.sampleMesh[0], game.texSample, game.progTutorial, UpdateToroid())
      ) {
        p->Velocity(glm::vec3(pos.x < 0 ? 0.1f : -0.1f, 0, -1));
      }
    }
    poppingTimer = rndPoppingTime(game.rand);
  }
}

/// エントリーポイント.
int main()
{
  GLFWEW::Window& window = GLFWEW::Window::Instance();
  if (!window.Init(800, 600, "OpenGL Tutorial")) {
    return 1;
  }
  Game& game = Game::Instance();
  if (!game.Init()) {
    return 1;
  }
  game.SetUpdateFunc(&Update);

  const double delta = 1.0 / 60.0;
  while (!window.ShouldClose()) {
    game.Update(delta);
    game.Render();
    window.SwapBuffers();
  }

  return 0;
}