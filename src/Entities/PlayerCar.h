#pragma once
#include "../ResourceManager.h"
#include "Entity.h"


class PlayerCar : public Entity {
public:
  PlayerCar();

  virtual void update(sf::Time dt) override;
  virtual void draw(sf::RenderWindow &window, float z = 0.f) override;

  void handleInput();

private:
  float mSpeed;
  float mMaxSpeed;
};
