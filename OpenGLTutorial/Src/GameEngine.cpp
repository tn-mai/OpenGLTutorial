/**
* @file GameEngine.cpp
*/
#include "GameEngine.h"
#include "GLFWEW.h"
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

/**
* ゲームエンジンのインスタンスを取得する.
*
* @return ゲームエンジンのインスタンス.
*/
GameEngine& GameEngine::Instance()
{
  static GameEngine instance;
  return instance;
}

/**
* ゲームエンジンを初期化する.
*
* @param w     ウィンドウの描画範囲の幅(ピクセル).
* @param h     ウィンドウの描画範囲の高さ(ピクセル).
* @param title ウィンドウタイトル(UTF-8の0終端文字列).
*
* @retval true  初期化成功.
* @retval false 初期化失敗.
*
* Run関数を呼び出す前に、一度だけ呼び出しておく必要がある.
* 一度初期化に成功すると、以後の呼び出しではなにもせずにtrueを返す.
*/
bool GameEngine::Init(int w, int h, const char* title)
{
  if (isInitialized) {
    return true;
  }
  if (!GLFWEW::Window::Instance().Init(w, h, title)) {
    return false;
  }

  //<--- ここに初期化処理を追加する --->

  isInitialized = true;
  return true;
}

/**
* ゲームを実行する.
*/
void GameEngine::Run()
{
  const double delta = 1.0 / 60.0;
  GLFWEW::Window& window = GLFWEW::Window::Instance();
  while (!window.ShouldClose()) {
    Update(delta);
    Render();
    window.SwapBuffers();
  }
}

/**
* 状態更新関数を設定する.
*
* @param func 設定する更新関数.
*/
void GameEngine::UpdateFunc(const UpdateFuncType& func)
{
  updateFunc = func;
}

/**
* 状態更新関数を取得する.
*
* @return 設定されている更新関数.
*/
const GameEngine::UpdateFuncType& GameEngine::UpdateFunc() const
{
  return updateFunc;
}

/**
* デストラクタ.
*/
GameEngine::~GameEngine()
{
  //<--- ここに終了処理を追加する --->
}

/**
* ゲームの状態を更新する.
*
* @param delta 前回の更新からの経過時間(秒).
*/
void GameEngine::Update(double delta)
{
  if (updateFunc) {
    updateFunc(delta);
  }
  //<--- ここに更新処理を追加する --->
}

/**
* ゲームの状態を描画する.
*/
void GameEngine::Render() const
{
  //<--- ここに描画処理を追加します --->
}
