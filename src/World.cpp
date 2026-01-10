#include "World.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>

World::World(sf::RenderWindow &window)
    : mWindow(window), mScrollSpeed(0.f), mScrollY(0.f), mPropSpawnTimer(0.f),
      mDayTime(0.f) {}

static sf::Vector2f project(float x, float y) {
  float centerX = 400.f;
  float horizonY = -250.f;
  float camY = 600.f;

  float scale = (y - horizonY) / (camY - horizonY);

  float newX = centerX + (x - centerX) * scale;
  return sf::Vector2f(newX, y);
}

void World::update(sf::Time dt) {
  mScrollY += mScrollSpeed * dt.asSeconds();
  if (mScrollY >= 2000.f) {
    mScrollY -= 2000.f;
  }

  // Day/Night Cycle (approx 60 seconds for full day)
  mDayTime += dt.asSeconds() * 0.016f;
  if (mDayTime > 1.0f)
    mDayTime -= 1.0f;

  updateProps(dt);
}

// ... spawnProp and updateProps remain same, verify if I need to re-output them
// if text matches ... Actually I am replacing the Constructor and update, but I
// can't rely on "updateProps" being the last line of target. Better to
// implement getSkyColor and modify draw.

sf::Color World::getSkyColor() const {
  // Adjusted Day/Night Cycle
  // 0.0 - 0.3: Bright Day (Blue)
  // 0.3 - 0.4: Afternoon (Transition to Orange)
  // 0.4 - 0.5: Sunset (Orange to Dark)
  // 0.5 - 0.8: Night (Dark)
  // 0.8 - 0.9: Sunrise (Dark to Purple)
  // 0.9 - 1.0: Morning (Purple to Blue)

  sf::Color deepBlue(0, 150, 255);
  sf::Color lightBlue(100, 200, 255);
  sf::Color orange(255, 100, 50);
  sf::Color dark(10, 10, 30);
  sf::Color purple(150, 50, 150);

  sf::Color c1, c2;
  float t = 0.f;

  if (mDayTime < 0.3f) { // Bright Day
    c1 = deepBlue;
    c2 = lightBlue;
    t = mDayTime / 0.3f;
  } else if (mDayTime < 0.4f) { // Afternoon
    c1 = lightBlue;
    c2 = orange;
    t = (mDayTime - 0.3f) / 0.1f;
  } else if (mDayTime < 0.5f) { // Sunset
    c1 = orange;
    c2 = dark;
    t = (mDayTime - 0.4f) / 0.1f;
  } else if (mDayTime < 0.8f) { // Night
    c1 = dark;
    c2 = dark;
    t = 0.f;
  } else if (mDayTime < 0.9f) { // Sunrise
    c1 = dark;
    c2 = purple;
    t = (mDayTime - 0.8f) / 0.1f;
  } else { // Morning
    c1 = purple;
    c2 = deepBlue;
    t = (mDayTime - 0.9f) / 0.1f;
  }

  // Interpolate
  sf::Uint8 r = (sf::Uint8)(c1.r + (c2.r - c1.r) * t);
  sf::Uint8 g = (sf::Uint8)(c1.g + (c2.g - c1.g) * t);
  sf::Uint8 b = (sf::Uint8)(c1.b + (c2.b - c1.b) * t);

  return sf::Color(r, g, b);
}

void World::spawnProp() {
  Prop p;
  int r = rand() % 100;
  if (r < 45)
    p.type = 0; // Tree
  else if (r < 90)
    p.type = 1; // Bush
  else
    p.type = 2; // Street Light

  p.active = true;
  p.pos.y = -200.f;

  bool left = (rand() % 2 == 0);

  if (p.type == 2) {
    if (left)
      p.pos.x = -200.f;
    else
      p.pos.x = 1000.f;
  } else {
    if (left) {
      p.pos.x = -1500.f + (rand() % 1000);
    } else {
      p.pos.x = 2300.f - (rand() % 1000);
    }
  }

  mProps.push_back(p);
}

void World::updateProps(sf::Time dt) {
  if (mScrollSpeed > 10.f) {
    float spawnChance = 0.05f * (mScrollSpeed / 200.f);
    if (spawnChance > 0.8f)
      spawnChance = 0.8f;

    mPropSpawnTimer += dt.asSeconds();
    float requiredTime = 1.0f / (1.0f + mScrollSpeed / 300.f);

    if (mPropSpawnTimer > requiredTime) {
      spawnProp();
      if (rand() % 2 == 0)
        spawnProp();
      mPropSpawnTimer = 0.f;
    }
  }

  float moveAmount = mScrollSpeed * dt.asSeconds();
  for (auto &p : mProps) {
    p.pos.y += moveAmount;
  }

  mProps.erase(std::remove_if(mProps.begin(), mProps.end(),
                              [](const Prop &p) { return p.pos.y > 800.f; }),
               mProps.end());
}

void World::draw() {
  sf::Color currentSky = getSkyColor();

  sf::VertexArray skyQuad(sf::Quads, 4);
  skyQuad[0].position = sf::Vector2f(0.f, 0.f);
  skyQuad[1].position = sf::Vector2f(800.f, 0.f);
  skyQuad[2].position = sf::Vector2f(800.f, 600.f);
  skyQuad[3].position = sf::Vector2f(0.f, 600.f);

  skyQuad[0].color = currentSky;
  skyQuad[1].color = currentSky;
  // Gradient for horizon
  skyQuad[2].color = sf::Color(
      255, 200, 150); // Setting Horizon to fixed warm color looks odd if night?
  // Let's make horizon a lighter version of sky
  skyQuad[2].color = sf::Color(std::min(255, currentSky.r + 50),
                               std::min(255, currentSky.g + 50),
                               std::min(255, currentSky.b + 50));
  skyQuad[3].color = skyQuad[2].color;
  mWindow.draw(skyQuad);

  sf::RectangleShape grass(sf::Vector2f(800.f, 600.f));
  grass.setPosition(0, 0);
  // Darken grass at night
  float lightLevel = 1.0f;
  if (mDayTime > 0.25f && mDayTime < 0.75f)
    lightLevel = 0.4f; // Darker at night

  // Actually interpolate light level
  lightLevel =
      1.0f -
      0.7f * std::sin(mDayTime *
                      3.14159f); // simple sine wave? Noon=0=1.0, Night=0.5=0.3
  if (lightLevel < 0.2f)
    lightLevel = 0.2f;

  sf::Color baseGrass = sf::Color(0, 100, 0);
  grass.setFillColor(sf::Color(baseGrass.r * lightLevel,
                               baseGrass.g * lightLevel,
                               baseGrass.b * lightLevel));
  mWindow.draw(grass);

  float segmentHeight = 100.f;
  int numSegments = 600 / (int)segmentHeight + 2;

  float yPixelOffset = std::fmod(mScrollY, segmentHeight);

  int baseIndex = (int)(mScrollY / segmentHeight);

  for (int i = -1; i < numSegments; ++i) {
    float y1 = i * segmentHeight + yPixelOffset;
    float y2 = y1 + segmentHeight;

    if (y1 >= 600)
      continue;
    if (y2 < 0)
      continue;

    sf::Vector2f tl = project(100.f, y1);
    sf::Vector2f tr = project(700.f, y1);
    sf::Vector2f br = project(700.f, y2);
    sf::Vector2f bl = project(100.f, y2);

    int segmentIndex = baseIndex - i;
    bool isDark = (std::abs(segmentIndex) % 2 == 0);

    sf::Color roadColor =
        isDark ? sf::Color(100, 100, 100) : sf::Color(90, 90, 90);
    sf::Color grassColor = isDark ? sf::Color(0, 110, 0) : sf::Color(0, 100, 0);

    // Apply lighting to road/grass segments
    roadColor.r *= lightLevel;
    roadColor.g *= lightLevel;
    roadColor.b *= lightLevel;
    grassColor.r *= lightLevel;
    grassColor.g *= lightLevel;
    grassColor.b *= lightLevel;

    sf::ConvexShape grassL(4);
    grassL.setFillColor(grassColor);
    grassL.setPoint(0, sf::Vector2f(0.f, y1));
    grassL.setPoint(1, sf::Vector2f(400.f, y1));
    grassL.setPoint(2, sf::Vector2f(400.f, y2));
    grassL.setPoint(3, sf::Vector2f(0.f, y2));
    mWindow.draw(grassL);

    sf::ConvexShape grassR(4);
    grassR.setFillColor(grassColor);
    grassR.setPoint(0, sf::Vector2f(400.f, y1));
    grassR.setPoint(1, sf::Vector2f(800.f, y1));
    grassR.setPoint(2, sf::Vector2f(800.f, y2));
    grassR.setPoint(3, sf::Vector2f(400.f, y2));
    mWindow.draw(grassR);

    sf::ConvexShape road(4);
    road.setFillColor(roadColor);
    road.setPoint(0, tl);
    road.setPoint(1, tr);
    road.setPoint(2, br);
    road.setPoint(3, bl);
    mWindow.draw(road);

    sf::Color borderColor = isDark ? sf::Color::Red : sf::Color::White;
    // Darken borders slightly less so they pop?
    borderColor.r *= lightLevel;
    borderColor.g *= lightLevel;
    borderColor.b *= lightLevel;

    sf::ConvexShape borderL(4);
    borderL.setFillColor(borderColor);
    borderL.setPoint(0, project(90.f, y1));
    borderL.setPoint(1, project(110.f, y1));
    borderL.setPoint(2, project(110.f, y2));
    borderL.setPoint(3, project(90.f, y2));
    mWindow.draw(borderL);

    sf::ConvexShape borderR(4);
    borderR.setFillColor(borderColor);
    borderR.setPoint(0, project(690.f, y1));
    borderR.setPoint(1, project(710.f, y1));
    borderR.setPoint(2, project(710.f, y2));
    borderR.setPoint(3, project(690.f, y2));
    mWindow.draw(borderR);

    if (isDark) {
      sf::Color laneColor = sf::Color(255, 255, 255, 200);
      laneColor.r *= lightLevel;
      laneColor.g *= lightLevel;
      laneColor.b *= lightLevel;

      sf::ConvexShape lane1(4);
      lane1.setFillColor(laneColor);
      lane1.setPoint(0, project(248.f, y1 + 20.f));
      lane1.setPoint(1, project(252.f, y1 + 20.f));
      lane1.setPoint(2, project(252.f, y2 - 20.f));
      lane1.setPoint(3, project(248.f, y2 - 20.f));
      mWindow.draw(lane1);

      sf::ConvexShape lane3(4);
      lane3.setFillColor(laneColor);
      lane3.setPoint(0, project(548.f, y1 + 20.f));
      lane3.setPoint(1, project(552.f, y1 + 20.f));
      lane3.setPoint(2, project(552.f, y2 - 20.f));
      lane3.setPoint(3, project(548.f, y2 - 20.f));
      mWindow.draw(lane3);
    }

    sf::ConvexShape center(4);
    center.setFillColor(sf::Color(255, 204, 0));
    center.setPoint(0, project(396.f, y1));
    center.setPoint(1, project(404.f, y1));
    center.setPoint(2, project(404.f, y2));
    center.setPoint(3, project(396.f, y2));
    mWindow.draw(center);
  }

  std::sort(mProps.begin(), mProps.end(),
            [](const Prop &a, const Prop &b) { return a.pos.y < b.pos.y; });

  for (const auto &p : mProps) {
    if (p.pos.y < 0 || p.pos.y > 600)
      continue;

    sf::Vector2f screenPos = project(p.pos.x, p.pos.y);
    float scale = (p.pos.y + 250.f) / 850.f;
    scale = std::max(0.1f, scale * 2.0f);

    if (p.type == 0) {
      sf::ConvexShape tree(3);
      tree.setFillColor(sf::Color(0, 60, 0));
      // Darken tree at night
      tree.setFillColor(sf::Color(0, 60 * lightLevel, 0));

      tree.setPoint(0, sf::Vector2f(0, -60));
      tree.setPoint(1, sf::Vector2f(20, 0));
      tree.setPoint(2, sf::Vector2f(-20, 0));

      tree.setPosition(screenPos);
      tree.setScale(scale, scale);
      mWindow.draw(tree);

      sf::RectangleShape trunk(sf::Vector2f(10.f, 15.f));
      trunk.setFillColor(sf::Color(50, 30, 0));
      trunk.setFillColor(sf::Color(50 * lightLevel, 30 * lightLevel, 0));
      trunk.setOrigin(5.f, 0.f);
      trunk.setPosition(screenPos);
      trunk.setScale(scale, scale);
      mWindow.draw(trunk);
    } else if (p.type == 1) {
      sf::CircleShape bush(20);
      bush.setFillColor(sf::Color(30, 80, 20));
      bush.setFillColor(
          sf::Color(30 * lightLevel, 80 * lightLevel, 20 * lightLevel));

      bush.setOrigin(20, 20);
      bush.setPosition(screenPos);
      bush.setScale(scale, scale * 0.7f);
      mWindow.draw(bush);
    } else if (p.type == 2) {
      // Street Light rendering
      sf::RectangleShape pole(sf::Vector2f(10.f, 200.f));
      pole.setFillColor(
          sf::Color(50 * lightLevel, 50 * lightLevel, 50 * lightLevel));
      pole.setOrigin(5.f, 200.f);
      pole.setPosition(screenPos);
      pole.setScale(scale, scale);
      mWindow.draw(pole);

      // Arm
      sf::RectangleShape arm(sf::Vector2f(60.f, 6.f));
      arm.setFillColor(pole.getFillColor());

      float armYOffset = 200.f;
      bool isLeft = (p.pos.x < 400.f);

      if (isLeft) {
        arm.setOrigin(0.f, 3.f); // Point Right
      } else {
        arm.setOrigin(60.f, 3.f); // Point Left
      }

      arm.setPosition(screenPos.x, screenPos.y - (armYOffset * scale));
      arm.setScale(scale, scale);
      mWindow.draw(arm);

      // Light
      if (lightLevel < 0.6f) { // Night Mode
        sf::Vector2f lightPos = arm.getPosition();
        if (isLeft)
          lightPos.x += 50.f * scale;
        else
          lightPos.x -= 50.f * scale;

        lightPos.y += 5.f * scale;

        // Glow
        sf::CircleShape glow(30.f);
        glow.setOrigin(30.f, 30.f);
        glow.setFillColor(sf::Color(255, 255, 200, 100)); // Soft Yellow
        glow.setPosition(lightPos);
        glow.setScale(scale, scale);
        mWindow.draw(glow);

        // Core
        sf::CircleShape core(8.f);
        core.setOrigin(8.f, 8.f);
        core.setFillColor(sf::Color::White);
        core.setPosition(lightPos);
        core.setScale(scale, scale);
        mWindow.draw(core);
      }
    }
  }
}

void World::drawDashedLine(sf::Vector2f p1, sf::Vector2f p2, float thickness,
                           sf::Color color) {}
