#include "EnemyCar.h"
#include "../AI/AIController.h"
#include <cmath>

EnemyCar::EnemyCar(float startX, float startY, float speed, Direction dir)
    : mSpeed(speed), mTargetX(startX), mDirection(dir) {
  mAI = new AIController(this);

  if (mDirection == Oncoming) {
    ResourceManager::getInstance().loadTexture("EnemyOncoming",
                                               "assets/graphics/pngegg.png");
    mSprite.setTexture(
        ResourceManager::getInstance().getTexture("EnemyOncoming"));
  } else {
    ResourceManager::getInstance().loadTexture("EnemySameWay",
                                               "assets/graphics/pngegg.png");
    mSprite.setTexture(
        ResourceManager::getInstance().getTexture("EnemySameWay"));
  }

  float scale = 80.f / mSprite.getLocalBounds().width;
  mSprite.setScale(scale, scale);

  sf::FloatRect bounds = mSprite.getLocalBounds();
  mSprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);

  if (mDirection == Oncoming) {
    mSprite.setRotation(90.f);
  } else {
    mSprite.setRotation(-90.f);
  }

  setPosition(startX, startY);
}

void EnemyCar::update(sf::Time dt) {

  float relativeSpeed = 0.f;

  if (mDirection == Oncoming) {
    relativeSpeed = mWorldSpeed + mSpeed;
  } else {
    relativeSpeed = mWorldSpeed - mSpeed;
  }

  mSprite.move(0.f, relativeSpeed * dt.asSeconds());

  float currentX = getPosition().x;
  if (std::abs(mTargetX - currentX) > 1.f) {
    float dir = (mTargetX > currentX) ? 1.f : -1.f;
    float strafeSpeed = 100.f;
    mSprite.move(dir * strafeSpeed * dt.asSeconds(), 0.f);

    if (std::abs(mTargetX - getPosition().x) < 2.f) {
      setPosition(mTargetX, getPosition().y);
    }
  }
}

void EnemyCar::updateAI(sf::Time dt,
                        const std::vector<std::shared_ptr<Entity>> &obstacles) {
  if (mAI) {
    mAI->update(dt, obstacles);
  }
}

static sf::Vector2f project(float x, float y, float z, float &outScale) {
  float centerX = 400.f;
  float horizonY = -250.f;
  float camY = 600.f;

  float scale = (y - horizonY) / (camY - horizonY);
  outScale = scale;

  float newX = centerX + (x - centerX) * scale;
  return sf::Vector2f(newX, y - z * scale);
}

void EnemyCar::draw(sf::RenderWindow &window, float z) {
  sf::Transform originalTransform = mSprite.getTransform();
  sf::Vector2f originalPos = mSprite.getPosition();
  sf::Vector2f originalScale = mSprite.getScale();

  float scaleFactor = 1.0f;
  sf::Vector2f viewPos = project(getPosition().x, getPosition().y, z, scaleFactor);

  mSprite.setPosition(viewPos);

  mSprite.setScale(originalScale.x * scaleFactor,
                   originalScale.y * scaleFactor);

  window.draw(mSprite);

  mSprite.setPosition(originalPos);
  mSprite.setScale(originalScale);
}
