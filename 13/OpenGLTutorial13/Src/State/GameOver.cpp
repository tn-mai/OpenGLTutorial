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
  GameEngine& game = GameEngine::Instance();
  game.PlayAudio(2, CRI_SAMPLECUESHEET_GAMEOVER);
}

void GameOver::operator()(double delta)
{
  timer += delta;

  GameEngine& game = GameEngine::Instance();
  game.Camera({ glm::vec4(0, 20, -8, 1), glm::vec3(0, 0, 12), glm::vec3(0, 0, 1) });

  const float alpha = static_cast<float>(std::min(1.0, timer * 0.2));

  game.FontPropotional(true);
  game.FontThickness(0.25f);
  game.FontBorder(0.25f);
  game.FontColor({ 0.9f, 0.25f, 0.25f, 1.0f * alpha });
  game.FontSubColor({ 1.0f, 1.0f, 1.0f, 1.0f * alpha });
  game.FontScale(glm::vec2(2));
  game.AddString(glm::vec2(-0.5f, 0.125f), "game over");
  game.FontScale(glm::vec2(0.5f));

  game.FontScale(glm::vec2(1));
  game.FontColor(glm::vec4(1));
  game.FontThickness(0.5f);
  game.FontBorder(0.25f);
  game.FontSubColor({0.25f, 0.1f, 0.4f, 0.8f});
  char str[16];
  snprintf(str, 16, "level:");
  game.AddString(glm::vec2(-0.25f, -0.125f), str);
  snprintf(str, 16, "score:");
  game.AddString(glm::vec2(-0.35f, -0.25f), str);

  game.FontPropotional(false);
  game.FontXAdvance(1.0f / 24.0f);
  snprintf(str, 16, "%03.0f", game.UserVariable(Global::varEnemyLevel));
  game.AddString(glm::vec2(0.0f, -0.125f), str);
  snprintf(str, 16, "%08.0f", game.UserVariable(Global::varScore));
  game.AddString(glm::vec2(-0.05f, -0.25f), str);

  if (timer >= 2.0 && (game.GetGamePad(0).buttonDown & (GamePad::A | GamePad::B | GamePad::START))) {
    game.UpdateFunc(Title(pSpaceSphere));
    game.StopAudio(2);
    game.PlayAudio(1, CRI_SAMPLECUESHEET_START);
  }
}

} // namespace State
