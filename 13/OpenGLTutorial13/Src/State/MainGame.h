/**
* @file MainGame.h
*/
#ifndef STATE_MAINGAME_H_INCLUDED
#define STATE_MAINGAME_H_INCLUDED
#include "../Entity.h"

namespace State {

class MainGame
{
public:
  explicit MainGame(Entity::Entity*);
  ~MainGame();
  void operator()(double);

private:
  void StartingDemo(double);
  void Main(double);

  Entity::Entity* pPlayer = nullptr;
  Entity::Entity* pPlayerBurner[2] = { nullptr };
  Entity::Entity* pSpaceSphere = nullptr;
  int enemyLevel = 0;

  enum Mode {
    Mode_StartingDemo,
    Mode_Main,
  };
  int mode = Mode_StartingDemo;
  double timer = 0;
};

} // namespace State

#endif // STATE_MAINGAME_H_INCLUDED