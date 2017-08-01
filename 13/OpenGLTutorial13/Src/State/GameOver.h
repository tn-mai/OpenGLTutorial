/**
* @file GameOver.h
*/
#ifndef STATE_GAMEOVER_H_INCLUDED
#define STATE_GAMEOVER_H_INCLUDED
#include "../Entity.h"

namespace State {

/**
* ゲームオーバー画面.
*/
class GameOver
{
public:
  explicit GameOver(Entity::Entity*);
  void operator()(double delta);

private:
  Entity::Entity* pSpaceSphere = nullptr;
  double timer = 0;
};

} // namespace State

#endif // STATE_GAMEOVER_H_INCLUDED