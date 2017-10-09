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
  Entity::Entity* pPlayer = nullptr;
  Entity::Entity* pSpaceSphere = nullptr;
  int enemyLevel = 0;
};

} // namespace State

#endif // STATE_MAINGAME_H_INCLUDED