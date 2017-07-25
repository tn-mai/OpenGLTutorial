/**
* @file Title.h
*/
#ifndef STATE_TITLE_H_INCLUDED
#define STATE_TITLE_H_INCLUDED
#include "../Entity.h"

namespace State {

/**
* �^�C�g�����.
*/
class Title
{
public:
  explicit Title(Entity::Entity* p = nullptr);
  void operator()(double delta);
private:
  Entity::Entity* pSpaceSphere = nullptr;
};

} // namespace State

#endif // STATE_TITLE_H_INCLUDED