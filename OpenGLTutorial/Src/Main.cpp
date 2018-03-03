/**
* @file Main.cpp
*/
#include "GameEngine.h"
#include "GameState.h"
#include "../Res/Audio/SampleSound_acf.h"
#include "../Res/Audio/SampleCueSheet.h"
#include <random>
#include <glm/gtc/matrix_transform.hpp>

/**
* Uniform Block Objectを作成する.
*
* @param size Uniform Blockのサイズ.
* @param data Uniform Blockに転送するデータへのポインタ.
*
* @return 作成したUBO.
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

/// エントリーポイント.
int main()
{
  GameEngine& game = GameEngine::Instance();
  if (!game.Init(800, 600, "OpenGL Tutorial")) {
    return 1;
  }
  if (!game.InitAudio("Res/Audio/SampleSound.acf", "Res/Audio/SampleCueSheet.acb", "Res/Audio/SampleCueSheet.awb", CRI_SAMPLESOUND_ACF_DSPSETTING_DSPBUSSETTING_0, GameState::AudioPlayerId_Max)) {
    return 1;
  }

  game.LoadTextureFromFile("Res/Toroid.bmp");
  game.LoadTextureFromFile("Res/Player.bmp");
  game.LoadTextureFromFile("Res/SpaceSphere.bmp");

  game.LoadMeshFromFile("Res/Toroid.fbx");
  game.LoadMeshFromFile("Res/Player.fbx");
  game.LoadMeshFromFile("Res/Blast.fbx");
  game.LoadMeshFromFile("Res/SpaceSphere.fbx");
  game.LoadFontFromFile("Res/UniNeue.fnt");

  game.UpdateFunc(GameState::Title());
  game.Run();
  return 0;
}