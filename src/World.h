#pragma once
#include <SFML/Graphics.hpp>
#include <vector>

class World {
public:
  World(sf::RenderWindow &window);
  void update(sf::Time dt);
  void draw();

  void setScrollSpeed(float speed) { mScrollSpeed = speed; }

  struct Prop {
    sf::Vector2f pos;
    int type;
    bool active;
  };

private:
  void drawDashedLine(sf::Vector2f p1, sf::Vector2f p2, float thickness,
                      sf::Color color);
  void spawnProp();
  void updateProps(sf::Time dt);

private:
  sf::RenderWindow &mWindow;
  float mScrollSpeed;
  float mScrollY;

  std::vector<Prop> mProps;
  float mPropSpawnTimer;

  // Day/Night Cycle
  float mDayTime; // 0 to 1 (0=Noon, 0.25=Dusk, 0.5=Night, 0.75=Dawn)
  sf::Color getSkyColor() const;
};
