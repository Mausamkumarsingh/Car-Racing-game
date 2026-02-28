#pragma once
#include <SFML/Graphics.hpp>

class Entity {
public:
  virtual ~Entity() = default;

  virtual void update(sf::Time dt) = 0;
  virtual void draw(sf::RenderWindow &window, float z = 0.f) = 0;

  sf::Vector2f getPosition() const { return mSprite.getPosition(); }
  void setPosition(float x, float y) { mSprite.setPosition(x, y); }
  sf::FloatRect getBounds() const { return mSprite.getGlobalBounds(); }

protected:
  sf::Sprite mSprite;
  sf::Vector2f mVelocity;
};
