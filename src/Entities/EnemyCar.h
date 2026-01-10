#pragma once
#include "../ResourceManager.h"
#include "Entity.h"
#include <memory>
#include <vector>

class EnemyCar : public Entity {
public:
  enum Direction { Oncoming, SameWay };

  EnemyCar(float startX, float startY, float speed, Direction dir = Oncoming);

  virtual void update(sf::Time dt) override;
  virtual void draw(sf::RenderWindow &window) override;

  void setSpeed(float speed) { mSpeed = speed; }
  float getSpeed() const { return mSpeed; }
  Direction getDirection() const { return mDirection; }

  void setTargetX(float x) { mTargetX = x; }

  void setWorldSpeed(float s) { mWorldSpeed = s; }

  void updateAI(sf::Time dt,
                const std::vector<std::shared_ptr<Entity>> &obstacles);

private:
  float mSpeed;
  float mWorldSpeed = 400.f;
  float mTargetX;
  Direction mDirection;
  class AIController *mAI;
};
