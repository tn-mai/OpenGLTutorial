/**
* @file TitleState.cpp
*/
#include "GameState.h"

#include "GameEngine.h"
#include "../Res/Audio/SampleCueSheet.h"

namespace GameState {

/**
* 背景球を更新する.
*/
void UpdateSpaceSphere(Entity::Entity& entity, double delta)
{
  glm::vec3 rotSpace = glm::eulerAngles(entity.Rotation());
  rotSpace.x += static_cast<float>(glm::radians(2.5) * delta);
  entity.Rotation(rotSpace);
}

/**
* タイトル画面を更新する.
*/
void Title::operator()(double delta)
{
  GameEngine& game = GameEngine::Instance();
  game.Camera({ glm::vec4(0, 20, -8, 1), glm::vec3(0, 0, 12), glm::vec3(0, 0, 1) });

  if (initial) {
    initial = false;

    game.KeyValue(0.02f);
    game.AmbientLight(glm::vec4(0.05f, 0.1f, 0.2f, 1));
    game.Light(0, { glm::vec4(40, 100, 10, 1), glm::vec4(12000, 12000, 12000, 1) } );

    game.RemoveAllEntity();
    game.ClearLevel();
    game.LoadMeshFromFile("Res/Model/SpaceSphere.fbx");
    game.LoadTextureFromFile("Res/Model/SpaceSphere.bmp");
    game.AddEntity(EntityGroupId_Others, glm::vec3(0, 0, 0), "SpaceSphere", "Res/Model/SpaceSphere.bmp", &UpdateSpaceSphere, "NonLighting");
  }

  const float offset = timer == 0 ? 0 : (2.0f - timer) * (2.0f - timer) * 2.0f;
  game.FontPropotional(true);
  game.FontThickness(0.25f);
  game.FontBorder(0.25f);
  game.FontColor({ 0.9f, 0.95f, 1.0f, 1.0f });
  game.FontSubColor({ 0.1f, 0.5f, 0.8f, 0.8f });
  game.FontScale(glm::vec2(2));
  game.AddString(glm::vec2(-0.25f + offset, 0.125f), "star fighter");
  game.FontScale(glm::vec2(0.5f));
  game.FontBorder(0.0f);
  game.AddString(glm::vec2(0.1f + offset, -0.05f), "the ultimate in manned-fighter");
  if (timer > 0) {
    timer -= static_cast<float>(delta);
    if (timer <= 0) {
      game.UpdateFunc(GameState::MainGame());
    }
  } else if (game.GetGamePad(0).buttonDown & (GamePad::A | GamePad::B | GamePad::START)) {
    game.PlayAudio(1, CRI_SAMPLECUESHEET_START);
    timer = 2;
  }
}

} // namespace State