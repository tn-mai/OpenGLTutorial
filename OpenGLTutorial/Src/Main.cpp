/**
* @file Main.cpp
*/
#include "GameEngine.h"
#include <random>
#include <glm/gtc/matrix_transform.hpp>

/**
* �G�̉~�Ղ̏�Ԃ��X�V����.
*/
struct UpdateToroid
{
  void operator()(Entity::Entity& entity, void* ubo, double delta,
    const glm::mat4& matView, const glm::mat4& matProj)
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

    // ���_�V�F�[�_�[�̃p�����[�^��UBO�ɃR�s�[����.
    InterfaceBlock::VertexData data;
    data.matModel = entity.CalcModelMatrix();
    data.matNormal = glm::mat4_cast(entity.Rotation());
    data.matMVP = matProj * matView * data.matModel;
    data.color = entity.Color();
    memcpy(ubo, &data, sizeof(InterfaceBlock::VertexData));
  }
};

/**
* ���@�̍X�V.
*/
struct UpdatePlayer
{
  void operator()(Entity::Entity& entity, void* ubo, double delta,
    const glm::mat4& matView, const glm::mat4& matProj)
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

    InterfaceBlock::VertexData data;
    data.matModel = entity.CalcModelMatrix();
    data.matNormal = glm::mat4_cast(entity.Rotation());
    data.matMVP = matProj * matView * data.matModel;
    data.color = entity.Color();
    memcpy(ubo, &data, sizeof(InterfaceBlock::VertexData));
  }
};

/**
* �Q�[����Ԃ̍X�V.
*/
class Update
{
public:
  void operator()(double delta)
  {
    GameEngine& game = GameEngine::Instance();

    if (!isInitialized) {
      isInitialized = true;
      game.Camera({ glm::vec4(0, 20, -8, 1), {0, 0, 12}, {0, 0, 1} });
      game.AmbientLight({ 0.05f, 0.1f, 0.2f, 1 });
      game.Light(0, { {40, 100, 10, 1}, {12000, 12000, 12000, 1} });
      pPlayer = game.AddEntity(glm::vec3(0, 0, 2), "Aircraft", "Res/Player.bmp", UpdatePlayer());
    }

    std::uniform_int_distribution<> posXRange(-15, 15);
    std::uniform_int_distribution<> posZRange(38, 40);
    interval -= delta;
    if (interval <= 0) {
      const std::uniform_int_distribution<> rndAddingCount(1, 5);
      for (int i = rndAddingCount(game.Rand()); i > 0; --i) {
        const glm::vec3 pos(posXRange(game.Rand()), 0, posZRange(game.Rand()));
        if (Entity::Entity* p = game.AddEntity(
          pos, "Toroid", "Res/Toroid.bmp", UpdateToroid())) {
          p->Velocity({pos.x < 0 ? 3.0f : -3.0f, 0, -12.0f});
        }
      }
      std::normal_distribution<> intervalRange(2.0, 0.5);
      interval += glm::clamp(intervalRange(game.Rand()), 0.5, 3.0);
    }
  }

private:
  bool isInitialized = false;
  double interval = 0;
  Entity::Entity* pPlayer = nullptr;
};

/**
* Uniform Block Object���쐬����.
*
* @param size Uniform Block�̃T�C�Y.
* @param data Uniform Block�ɓ]������f�[�^�ւ̃|�C���^.
*
* @return �쐬����UBO.
*/
GLuint CreateUBO(GLsizeiptr size, const GLvoid* data = nullptr)
{
  GLuint ubo;
  glGenBuffers(1, &ubo);
  glBindBuffer(GL_UNIFORM_BUFFER, ubo);
  glBufferData(GL_UNIFORM_BUFFER, size, data, GL_STREAM_DRAW);
  glBindBuffer(GL_UNIFORM_BUFFER, 0);
  return ubo;
}

/// �G���g���[�|�C���g.
int main()
{
  GameEngine& game = GameEngine::Instance();
  if (!game.Init(800, 600, "OpenGL Tutorial")) {
    return 1;
  }
  game.LoadTextureFromFile("Res/Toroid.bmp");
  game.LoadTextureFromFile("Res/Player.bmp");
  game.LoadMeshFromFile("Res/Toroid.fbx");
  game.LoadMeshFromFile("Res/Player.fbx");
  game.UpdateFunc(Update());
  game.Run();
  return 0;
}