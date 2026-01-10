#pragma once
#include "../Entities/Entity.h"
#include <SFML/Graphics.hpp>
#include <memory>
#include <vector>

class EnemyCar;

class AIController {
public:
  explicit AIController(EnemyCar *owner);

  void update(sf::Time dt,
              const std::vector<std::shared_ptr<Entity>> &obstacles);

private:
  void decideMovement(const std::vector<bool> &blockedLanes,
                      const std::vector<float> &distances);
  bool checkLane(float laneX,
                 const std::vector<std::shared_ptr<Entity>> &obstacles,
                 float &outDist);

private:
  EnemyCar *mOwner;
  float mDecisionTimer;
  float mSwayTarget;
};
