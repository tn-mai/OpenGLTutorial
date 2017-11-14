/**
* @file main.cpp
*/
#include "GameEngine.h"
#include "GameState.h"
#include "../Res/Audio/SampleSound_acf.h"

/// エントリーポイント.
int main()
{
  GameEngine& game = GameEngine::Instance();
  if (!game.Init(800, 600, "OpenGL Tutorial")) {
    return 1;
  }
  if (!game.InitAudio("Res/Audio/SampleSound.acf", "Res/Audio/SampleCueSheet.acb", "Res/Audio/SampleCueSheet.awb", CRI_SAMPLESOUND_ACF_DSPSETTING_DSPBUSSETTING_0)) {
    return 1;
  }

  game.LoadFontFromFile("Res/Font.fnt");
  game.LoadTextureFromFile("Res/Model/Dummy.Normal.bmp");

  game.PushLevel();
  game.UpdateFunc(GameState::Title());
  game.Run();

  return 0;
}