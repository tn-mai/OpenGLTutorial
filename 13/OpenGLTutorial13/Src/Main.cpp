/**
* @file main.cpp
*/
#include "GameEngine.h"
#include "State/Title.h"
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

  game.LoadTextureFromFile("Res/Model/DummyNormal.dds");
  game.LoadTextureFromFile("Res/Model/Toroid.dds");
  game.LoadTextureFromFile("Res/Model/Toroid.Normal.dds");
  game.LoadTextureFromFile("Res/Model/SpaceShip.Diffuse.dds");
  game.LoadTextureFromFile("Res/Model/SpaceShip.Normal.dds");
  game.LoadTextureFromFile("Res/Model/Player.bmp");
  game.LoadTextureFromFile("Res/Model/SpaceSphere.bmp");
  game.LoadMeshFromFile("Res/Model/Player.fbx");
  game.LoadMeshFromFile("Res/Model/Toroid.fbx");
  game.LoadMeshFromFile("Res/Model/SpaceShip.fbx");
  game.LoadMeshFromFile("Res/Model/Cardioid.fbx");
  game.LoadMeshFromFile("Res/Model/Blast.fbx");
  game.LoadMeshFromFile("Res/Model/SpaceSphere.fbx");
  game.LoadFontFromFile("Res/Font.fnt");

  game.UpdateFunc(State::Title());
  game.Run();

  return 0;
}