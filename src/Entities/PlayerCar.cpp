#include "PlayerCar.h"

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
    mSprite.move(movement * mMaxSpeed * dt.asSeconds());
  }

  sf::Vector2f pos = getPosition();
  if (pos.x < 150)
    setPosition(150, pos.y);
  if (pos.x > 650)
    setPosition(650, pos.y);
}

static sf::Vector2f project(float x, float y, float &outScale) {
  float centerX = 400.f;
  float horizonY = -250.f;
  float camY = 600.f;

  float scale = (y - horizonY) / (camY - horizonY);
  outScale = scale;

  float newX = centerX + (x - centerX) * scale;
  return sf::Vector2f(newX, y);
}

void PlayerCar::draw(sf::RenderWindow &window) {
  sf::Transform originalTransform = mSprite.getTransform();
  sf::Vector2f originalPos = mSprite.getPosition();
  sf::Vector2f originalScale = mSprite.getScale();

  float scaleFactor = 1.0f;
  sf::Vector2f viewPos = project(getPosition().x, getPosition().y, scaleFactor);

  mSprite.setPosition(viewPos);
  mSprite.setScale(originalScale.x * scaleFactor,
                   originalScale.y * scaleFactor);

  window.draw(mSprite);

  mSprite.setPosition(originalPos);
  mSprite.setScale(originalScale);
}

void PlayerCar::handleInput() {}
