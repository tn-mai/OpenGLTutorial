/**
* @file MainGameState.cpp
*/
#include "GameState.h"
#include "GameEngine.h"
#include "../Res/Audio/SampleCueSheet.h"
#include <algorithm>

namespace GameState {

/// �Փˌ`�󃊃X�g.
static const Entity::CollisionData collisionDataList[] = {
  { glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f) },
{ glm::vec3(-0.5f, -0.5f, -1.0f), glm::vec3(0.5f, 0.5f, 1.0f) },
{ glm::vec3(-1.0f, -1.0f, -1.0f), glm::vec3(1.0f, 1.0f, 1.0f) },
{ glm::vec3(-0.25f, -0.25f, -0.25f), glm::vec3(0.25f, 0.25f, 0.25f) },
};

/**
* �G�̉~�Ղ̏�Ԃ��X�V����.
*/
struct UpdateToroid
{
  void operator()(Entity::Entity& entity, double delta)
  {
    // �͈͊O�ɏo����폜����.
    const glm::vec3 pos = entity.Position();
    if (std::abs(pos.x) > 40.0f || std::abs(pos.z) > 40.0f) {
      GameEngine::Instance().RemoveEntity(&entity);
      return;
    }

    // �~�Ղ���]������.
    float rot = glm::angle(entity.Rotation());
    rot += glm::radians(15.0f) * static_cast<float>(delta);
    if (rot > glm::pi<float>() * 2.0f) {
      rot -= glm::pi<float>() * 2.0f;
    }
    entity.Rotation(glm::angleAxis(rot, glm::vec3(0, 1, 0)));
  }
};

/**
* ���@�̒e�̍X�V.
*/
struct UpdatePlayerShot
{
  void operator()(Entity::Entity& entity, double delta)
  {
    // �͈͊O�ɏo����폜����.
    const glm::vec3 pos = entity.Position();
    if (std::abs(pos.x) > 40 || pos.z < -4 || pos.z > 40) {
      entity.Destroy();
      return;
    }
  }
};

/**
* �����̍X�V.
*/
struct UpdateBlast
{
  void operator()(Entity::Entity& entity, double delta) {
    timer += delta;
    if (timer >= 0.5) {
      entity.Destroy();
      return;
    }
    const float variation = static_cast<float>(timer * 4); // �ω���.
    entity.Scale(glm::vec3(static_cast<float>(1 + variation))); // ���X�Ɋg�傷��.
                                                                // ���Ԍo�߂ŐF�Ɠ����x��ω�������.
    static const glm::vec4 color[] = {
      glm::vec4(1.0f, 1.0f, 0.75f, 1),
      glm::vec4(1.0f, 0.5f, 0.1f, 1),
      glm::vec4(0.25f, 0.1f, 0.1f, 0),
    };
    const glm::vec4 col0 = color[static_cast<int>(variation)];
    const glm::vec4 col1 = color[static_cast<int>(variation) + 1];
    const glm::vec4 newColor = glm::mix(col0, col1, std::fmod(variation, 1));
    entity.Color(newColor);
    // Y����]������.
    glm::vec3 euler = glm::eulerAngles(entity.Rotation());
    euler.y += glm::radians(60.0f) * static_cast<float>(delta);
    entity.Rotation(glm::quat(euler));
  }

  double timer = 0;
};

/**
* ���@�̍X�V.
*/
struct UpdatePlayer
{
  void operator()(Entity::Entity& entity, double delta)
  {
    GameEngine& game = GameEngine::Instance();
    const GamePad gamepad = game.GetGamePad();
    glm::vec3 vec;
    float rotZ = 0;
    if (gamepad.buttons & GamePad::DPAD_LEFT) {
      vec.x = 1;
      rotZ = -glm::radians(30.0f);
    } else if (gamepad.buttons & GamePad::DPAD_RIGHT) {
      vec.x = -1;
      rotZ = glm::radians(30.0f);
    }
    if (gamepad.buttons & GamePad::DPAD_UP) {
      vec.z = 1;
    } else if (gamepad.buttons & GamePad::DPAD_DOWN) {
      vec.z = -1;
    }
    if (vec.x || vec.z) {
      vec = glm::normalize(vec) * 15.0f;
    }
    entity.Velocity(vec);
    entity.Rotation(glm::quat(glm::vec3(0, 0, rotZ)));
    glm::vec3 pos = entity.Position();
    pos = glm::min(glm::vec3(11, 100, 20), glm::max(pos, glm::vec3(-11, -100, 1)));
    entity.Position(pos);

    if (gamepad.buttons & GamePad::A) {
      shotInterval -= delta;
      if (shotInterval <= 0) {
        glm::vec3 pos = entity.Position();
        pos.x -= 0.3f; // ���@�̒��S���獶��0.3���炵���ʒu��1�߂̔��˓_.
        for (int i = 0; i < 2; ++i) {
          if (Entity::Entity* p = game.AddEntity(EntityGroupId_PlayerShot, pos,
            "NormalShot", "Res/Player.bmp", UpdatePlayerShot())) {
            p->Velocity(glm::vec3(0, 0, 80));
            p->Collision(collisionDataList[EntityGroupId_PlayerShot]);
          }
          pos.x += 0.6f; // ���S����ɉE��0.3���炵���ʒu��2�߂̔��˓_.
        }
        shotInterval += 0.25;// �b��4�A��.
        game.PlayAudio(AudioPlayerId_Shot, CRI_SAMPLECUESHEET_PLAYERSHOT);
      }
    } else {
      shotInterval = 0;
    }
  }

private:
  double shotInterval = 0;
};

/**
* ���@�̒e�ƓG�̏Փˏ���.
*/
void PlayerShotAndEnemyCollisionHandler(Entity::Entity& lhs, Entity::Entity& rhs)
{
  GameEngine& game = GameEngine::Instance();
  if (Entity::Entity* p = game.AddEntity(EntityGroupId_Others, rhs.Position(),
    "Blast", "Res/Toroid.bmp", UpdateBlast())) {
    const std::uniform_real_distribution<float> rotRange(0.0f, glm::pi<float>() * 2);
    p->Rotation(glm::quat(glm::vec3(0, rotRange(game.Rand()), 0)));
    game.Variable("score") += 100;
  }
  game.PlayAudio(AudioPlayerId_Bomb, CRI_SAMPLECUESHEET_BOMB);
  lhs.Destroy();
  rhs.Destroy();
}

/**
* ���C���Q�[����ʂ̃R���X�g���N�^.
*/
MainGame::MainGame(Entity::Entity* p) : pSpaceSphere(p)
{
  GameEngine& game = GameEngine::Instance();
  game.CollisionHandler(EntityGroupId_PlayerShot, EntityGroupId_Enemy, &PlayerShotAndEnemyCollisionHandler);
  GameEngine::Instance().Variable("score") = 0;
}

/**
* ���C���Q�[����ʂ̍X�V.
*/
void MainGame::operator()(double delta)
{
  GameEngine& game = GameEngine::Instance();

  if (!isInitialized) {
    isInitialized = true;
    game.Camera({ glm::vec4(0, 20, -8, 1),{ 0, 0, 12 },{ 0, 0, 1 } });
    game.AmbientLight({ 0.05f, 0.1f, 0.2f, 1 });
    game.Light(0, { { 40, 100, 10, 1 },{ 12000, 12000, 12000, 1 } });
    pPlayer = game.AddEntity(EntityGroupId_Player, glm::vec3(0, 0, 2), "Aircraft", "Res/Player.bmp", UpdatePlayer());
    pPlayer->Collision(collisionDataList[EntityGroupId_Player]);

    game.PlayAudio(AudioPlayerId_BGM, CRI_SAMPLECUESHEET_BGM02);
  }

  std::uniform_int_distribution<> posXRange(-15, 15);
  std::uniform_int_distribution<> posZRange(38, 40);
  interval -= delta;
  if (interval <= 0) {
    const std::uniform_int_distribution<> rndAddingCount(1, 5);
    for (int i = rndAddingCount(game.Rand()); i > 0; --i) {
      const glm::vec3 pos(posXRange(game.Rand()), 0, posZRange(game.Rand()));
      if (Entity::Entity* p = game.AddEntity(
        EntityGroupId_Enemy, pos, "Toroid", "Res/Toroid.bmp", UpdateToroid())) {
        p->Velocity({ pos.x < 0 ? 3.0f : -3.0f, 0, -12.0f });
        p->Collision(collisionDataList[EntityGroupId_Enemy]);
      }
    }
    std::normal_distribution<> intervalRange(2.0, 0.5);
    interval += glm::clamp(intervalRange(game.Rand()), 0.5, 3.0);
  }
  char str[16];
  snprintf(str, 16, "%08.0f", game.Variable("score"));
  game.FontScale(glm::vec2(1));
  game.FontColor(glm::vec4(1));
  game.AddString(glm::vec2(320, 8), str);
}

} // namespace GameState