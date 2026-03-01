#include "AIController.h"
#include "../Entities/EnemyCar.h"
#include <algorithm>
#include <iostream>

AIController::AIController(EnemyCar *owner)
    : mOwner(owner), mDecisionTimer(0.f), mSwayTarget(0.f) {}

void AIController::update(
    sf::Time dt, const std::vector<std::shared_ptr<Entity>> &obstacles) {
  mDecisionTimer += dt.asSeconds();
  if (mDecisionTimer < 0.2f)
    return;
  mDecisionTimer = 0.f;

  float lanes[] = {250.f, 400.f, 550.f};
  int currentLaneIdx = 1;
  float myX = mOwner->getPosition().x;

  float minDiff = 10000.f;
  for (int i = 0; i < 3; ++i) {
    float diff = std::abs(myX - lanes[i]);
    if (diff < minDiff) {
      minDiff = diff;
      currentLaneIdx = i;
    }
  }

  float lookAheadDist = 300.f;

  std::vector<bool> blocked(3, false);
  std::vector<float> dists(3, lookAheadDist);

  for (int i = 0; i < 3; ++i) {
    float d = lookAheadDist;
    if (checkLane(lanes[i], obstacles, d)) {
      blocked[i] = true;
      dists[i] = d;
    }
  }

  if (blocked[currentLaneIdx]) {
    if (currentLaneIdx > 0 && !blocked[currentLaneIdx - 1]) {
      mOwner->setTargetX(lanes[currentLaneIdx - 1]);
    } else if (currentLaneIdx < 2 && !blocked[currentLaneIdx + 1]) {
      mOwner->setTargetX(lanes[currentLaneIdx + 1]);
    } else {
      float currentSpeed = mOwner->getSpeed();
      mOwner->setSpeed(currentSpeed * 0.9f);
    }
  } else {
    float currentSpeed = mOwner->getSpeed();
    float maxSpeed = (mOwner->getDirection() == EnemyCar::Oncoming) ? 200.f : 180.f;
    if (currentSpeed < maxSpeed) {
      mOwner->setSpeed(std::min(maxSpeed, currentSpeed + 10.f));
    }
  }
}

bool AIController::checkLane(
    float laneX, const std::vector<std::shared_ptr<Entity>> &obstacles,
    float &outDist) {
  sf::FloatRect ray;
  ray.left = laneX - 30.f;
  ray.width = 60.f;
  ray.top = mOwner->getPosition().y + 30.f;

  ray.height = 300.f;

  for (const auto &obs : obstacles) {
    if (obs.get() == mOwner)
      continue;

    if (ray.intersects(obs->getBounds())) {
      outDist = obs->getPosition().y - mOwner->getPosition().y;
      return true;
    }
  }
  return false;
}
