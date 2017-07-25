/**
* @file GameOver.cpp
*/
#include "GameOver.h"
#include "Title.h"
#include "Global.h"
#include "../GameEngine.h"
#include "../../Res/Audio/SampleCueSheet.h"
#include <algorithm>

namespace State {

GameOver::GameOver(Entity::Entity* p) : pSpaceSphere(p)
{
}

void GameOver::operator()(double delta)
{
  timer += delta;

  GameEngine& game = GameEngine::Instance();
  game.Camera({ glm::vec4(0, 20, -8, 1), glm::vec3(0, 0, 12), glm::vec3(0, 0, 1) });

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
    game.UpdateFunc(Title(pSpaceSphere));
    game.PlayAudio(1, CRI_SAMPLECUESHEET_START);
  }
}

} // namespace State
