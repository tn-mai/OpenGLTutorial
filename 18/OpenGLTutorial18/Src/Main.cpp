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

  game.LoadTextureFromFile("Res/Model/Toroid.bmp");
  game.LoadTextureFromFile("Res/Model/Toroid.Normal.bmp");
  game.LoadTextureFromFile("Res/Model/Dummy.Normal.bmp");
  game.LoadTextureFromFile("Res/Model/Player.bmp");
  game.LoadTextureFromFile("Res/Model/SpaceSphere.bmp");
  game.LoadMeshFromFile("Res/Model/Player.fbx");
  game.LoadMeshFromFile("Res/Model/Toroid.fbx");
  game.LoadMeshFromFile("Res/Model/Blast.fbx");
  game.LoadMeshFromFile("Res/Model/SpaceSphere.fbx");
  game.LoadFontFromFile("Res/Font.fnt");

  game.UpdateFunc(GameState::Title());
  game.Run();

  return 0;
}