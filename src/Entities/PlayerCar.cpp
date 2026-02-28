#include "PlayerCar.h"
#include <cmath>

PlayerCar::PlayerCar() : mSpeed(0.f), mMaxSpeed(500.f) {
  ResourceManager::getInstance().loadTexture("PlayerCar",
                                             "assets/graphics/pngegg_1.png");
  mSprite.setTexture(ResourceManager::getInstance().getTexture("PlayerCar"));

  float scale = 80.f / mSprite.getLocalBounds().width;
  mSprite.setScale(scale, scale);

  sf::FloatRect bounds = mSprite.getLocalBounds();
  mSprite.setOrigin(bounds.width / 2.f, bounds.height / 2.f);

  mSprite.setRotation(90.f);

  setPosition(400.f, 500.f);
}

void PlayerCar::update(sf::Time dt) {
  sf::Vector2f movement(0.f, 0.f);
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) ||
      sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
    movement.x -= 1.f;
  }
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) ||
      sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
    movement.x += 1.f;
  }
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) ||
      sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
    movement.y -= 1.f;
  }
  if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) ||
      sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
    movement.y += 1.f;
  }

  if (movement.x != 0 || movement.y != 0) {
    float length = std::sqrt(movement.x * movement.x + movement.y * movement.y);
    movement.x /= length;
    movement.y /= length;
    mSprite.move(movement * mMaxSpeed * dt.asSeconds());
  }

  // Tilt the car visually when turning
  float targetRotation = 90.f; // Base rotation facing up (since image faces right)
  if (movement.x < 0) targetRotation = 80.f; // Steer left (counter-clockwise)
  else if (movement.x > 0) targetRotation = 100.f; // Steer right (clockwise)
  
  float currentRot = mSprite.getRotation();
  float diff = targetRotation - currentRot;
  if (diff > 180.f) diff -= 360.f;
  if (diff < -180.f) diff += 360.f;
  mSprite.rotate(diff * 15.f * dt.asSeconds());

  sf::Vector2f pos = getPosition();
  if (pos.x < 175) pos.x = 175;
  if (pos.x > 625) pos.x = 625;
  if (pos.y < 400) pos.y = 400; // Keep on visible screen road
  if (pos.y > 550) pos.y = 550; // Don't let player drive out bottom
  setPosition(pos.x, pos.y);
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

void PlayerCar::draw(sf::RenderWindow &window, float z) {
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

void PlayerCar::handleInput() {}
