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

static sf::Vector2f project3D(float x, float y, float z) {
  float centerX = 400.f;
  float horizonY = -250.f;
  float camY = 600.f;
  float scale = (y - horizonY) / (camY - horizonY);
  float newX = centerX + (x - centerX) * scale;
  return sf::Vector2f(newX, y - z * scale);
}

static float smoothStep01(float t) {
  t = std::clamp(t, 0.f, 1.f);
  return t * t * (3.f - 2.f * t);
}

static float getZ(float segment_float) {
    float w = std::fmod(segment_float, 500.f);
    if (w < 0) w += 500.f;
  if (w < 25.f || w > 75.f) return 0.f;  // Bridge between segment 25 and 75

  const float peak = 350.f;
  if (w <= 45.f) {
    float t = (w - 25.f) / 20.0f; // 0.0 to 1.0
    return smoothStep01(t) * peak;
  }
  if (w >= 55.f) {
    float t = (75.f - w) / 20.0f; // 1.0 to 0.0
    return smoothStep01(t) * peak;
  }
  return peak;
}

float World::getElevationAtScreenY(float screenY) const {
  float yPixelOffset = std::fmod(mScrollY, 100.f);
  int baseIndex = (int)(mScrollY / 100.f);
  float i_float = (screenY - yPixelOffset) / 100.f;
  float segment_float = baseIndex - i_float;
  
  float seg1 = std::floor(segment_float);
  float seg2 = seg1 + 1.f;
  float t = segment_float - seg1;
  
  return getZ(seg1) * (1.f - t) + getZ(seg2) * t;
}

void World::update(sf::Time dt) {
  mScrollY += mScrollSpeed * dt.asSeconds();
  if (mScrollY >= 10000.f) {
    mScrollY -= 10000.f;
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

void World::draw(const std::vector<ZItem>& items) {
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

  // Calculate light level for environment darkening
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

  // --- DRAW STARS (Visible at night/twilight) ---
  float starAlpha = 0.f;
  if (mDayTime > 0.45f && mDayTime < 0.9f) {
      // Fade in stars 0.45 -> 0.5, solid 0.5 -> 0.8, fade out 0.8 -> 0.9
      if (mDayTime < 0.5f) starAlpha = (mDayTime - 0.45f) * 20.f;
      else if (mDayTime > 0.8f) starAlpha = 1.f - (mDayTime - 0.8f) * 10.f;
      else starAlpha = 1.f;
  }
  if (starAlpha > 0.f) {
      sf::VertexArray stars(sf::Points);
      // Pseudo-random stars using fixed coordinates
      for (int i = 0; i < 150; ++i) {
          float sx = (float)((i * 137) % 800);
          float sy = (float)((i * 251) % 300); // Only upper half of screen
          float brightness = (float)((i * 73) % 255);
          stars.append(sf::Vertex(sf::Vector2f(sx, sy), sf::Color(255, 255, 255, (sf::Uint8)(brightness * starAlpha))));
      }
      mWindow.draw(stars);
  }

  // --- DRAW CLOUDS ---
  // A simple procedural scrolling cloud layer using simple semi-transparent circles
  float cxOffset = std::fmod(mDayTime * 300.f, 1500.f); 
  for (int i = 0; i < 7; ++i) {
      float cx = (float)((i * 371) % 1500) - cxOffset;
      if (cx < -200.f) cx += 1500.f; // wrap around
      float cy = 50.f + (float)((i * 83) % 150);
      float cSize = 40.f + (float)((i * 19) % 40);
      
      float alphaLight = 80.f * lightLevel;
      if (alphaLight < 10.f) alphaLight = 10.f;
      sf::Color cloudColor(255, 255, 255, (sf::Uint8)(int)alphaLight);
      
      sf::CircleShape puff(cSize);
      puff.setFillColor(cloudColor);
      
      puff.setPosition(cx, cy); mWindow.draw(puff);
      puff.setPosition(cx + cSize*0.8f, cy - cSize*0.3f); mWindow.draw(puff);
      puff.setPosition(cx + cSize*1.5f, cy + cSize*0.1f); mWindow.draw(puff);
  }

  // --- DRAW BIRDS ---
  // Small aesthetic "V" shaped birds flying across the sky occasionally
  float birdProgress = std::fmod(mDayTime * 15.f, 1.0f); // Fast cycle
  if (birdProgress < 0.3f && lightLevel > 0.4f) { // Only draw during day/evening for a short burst
      float bxStart = -50.f + birdProgress * 3500.f; // Sweep across
      float byStart = 100.f - std::sin(birdProgress * 3.1415f) * 50.f; // Arc up and down slightly
      
      sf::ConvexShape bird(3);
      bird.setFillColor(sf::Color(30, 30, 30, 200));
      for (int i = 0; i < 5; ++i) { // Flock of 5
          float bx = bxStart - i * 25.f + (float)((i*7)%15);
          float by = byStart + (i%2 == 0 ? i * 15.f : -i * 15.f);
          
          // Flapping animation
          float flap = std::sin(mDayTime * 200.f + i) * 6.f;
          
          bird.setPoint(0, sf::Vector2f(bx, by));
          bird.setPoint(1, sf::Vector2f(bx - 10.f, by - 5.f + flap));
          bird.setPoint(2, sf::Vector2f(bx - 5.f, by + 2.f));
          mWindow.draw(bird);
      }
  }

  // --- DRAW SUN / MOON ---
  // A day is 0.0 to 1.0. Sun is up 0.9 -> 0.0 -> 0.5. Moon is up 0.4 -> 0.9.
  float sunPosT = mDayTime;
  if (sunPosT > 0.9f) sunPosT -= 1.0f; // Shift so sun peaks at 0.15
  float sunAngle = (sunPosT + 0.1f) * 3.14159f / 0.6f; // Map -0.1 to 0.5 to 0 -> Pi
  if (sunAngle > 0 && sunAngle < 3.14159f) {
      float sx = 400.f - std::cos(sunAngle) * 300.f;
      float sy = 350.f - std::sin(sunAngle) * 300.f;
      sf::CircleShape sun(40.f);
      sun.setOrigin(40.f, 40.f);
      sun.setPosition(sx, sy);
      sun.setFillColor(sf::Color(255, 255, 200));
      mWindow.draw(sun);
      
      sf::CircleShape sunHalo(60.f);
      sunHalo.setOrigin(60.f, 60.f);
      sunHalo.setPosition(sx, sy);
      sunHalo.setFillColor(sf::Color(255, 255, 150, 60));
      mWindow.draw(sunHalo);
  }

  float moonAngle = (mDayTime - 0.4f) * 3.14159f / 0.5f; // Map 0.4 to 0.9 to 0 -> Pi
  if (moonAngle > 0 && moonAngle < 3.14159f) {
      float mx = 400.f - std::cos(moonAngle) * 300.f;
      float my = 350.f - std::sin(moonAngle) * 300.f;
      sf::CircleShape moon(30.f);
      moon.setOrigin(30.f, 30.f);
      moon.setPosition(mx, my);
      moon.setFillColor(sf::Color(220, 220, 255));
      mWindow.draw(moon);
      
      // Moon craters (simple overlapping circles)
      sf::CircleShape crater(8.f);
      crater.setFillColor(sf::Color(180, 180, 220));
      crater.setPosition(mx - 10.f, my - 5.f); mWindow.draw(crater);
      crater.setRadius(5.f); crater.setPosition(mx + 10.f, my + 10.f); mWindow.draw(crater);
  }

  // --- DRAW PARALLAX MOUNTAINS ---
  // Scroll X and Y could affect mountains slightly
  float mountainParallaxX = 0.f; 
  sf::ConvexShape mountain(3);
  int numMountains = 5;
  for (int i = 0; i < numMountains; ++i) {
      // Far mountains
      float mx = (i * 200.f) - 100.f + mountainParallaxX;
      float mW = 350.f;
      float mH = 150.f + (float)((i * 37) % 50);
      sf::Color mCol = sf::Color(std::min(255, currentSky.r + 20), std::min(255, currentSky.g + 30), std::min(255, currentSky.b + 40));
      mountain.setFillColor(mCol);
      mountain.setPoint(0, sf::Vector2f(mx - mW/2, 350.f)); // Horizon is around 350 visually for backdrop
      mountain.setPoint(1, sf::Vector2f(mx, 350.f - mH));
      mountain.setPoint(2, sf::Vector2f(mx + mW/2, 350.f));
      mWindow.draw(mountain);
  }
  for (int i = 0; i < numMountains + 1; ++i) {
      // Near mountains
      float mx = (i * 150.f) - 50.f + mountainParallaxX * 1.5f;
      float mW = 250.f;
      float mH = 100.f + (float)((i * 91) % 60);
      sf::Color mCol = sf::Color(std::max(0, currentSky.r - 20), std::max(0, currentSky.g - 10), std::max(0, currentSky.b - 10));
      mountain.setFillColor(mCol);
      mountain.setPoint(0, sf::Vector2f(mx - mW/2, 350.f));
      mountain.setPoint(1, sf::Vector2f(mx, 350.f - mH));
      mountain.setPoint(2, sf::Vector2f(mx + mW/2, 350.f));
      mWindow.draw(mountain);
  }

  sf::RectangleShape grass(sf::Vector2f(800.f, 600.f));
  // Keep base grass starting under the mountains
  grass.setPosition(0, 350.f);
  grass.setSize(sf::Vector2f(800.f, 250.f));

  sf::Color baseGrass = sf::Color(0, 100, 0);
  grass.setFillColor(sf::Color(baseGrass.r * lightLevel,
                               baseGrass.g * lightLevel,
                               baseGrass.b * lightLevel));
  mWindow.draw(grass);

  float segmentHeight = 100.f;
  int numSegments = 30;

  float yPixelOffset = std::fmod(mScrollY, segmentHeight);

  int baseIndex = (int)(mScrollY / segmentHeight);

  auto applyLight = [lightLevel](sf::Color c) {
    return sf::Color((sf::Uint8)(c.r * lightLevel),
                     (sf::Uint8)(c.g * lightLevel),
                     (sf::Uint8)(c.b * lightLevel), c.a);
  };

  bool bridgeVisible = false;
  bool hasBridgeRange = false;
  float bridgeMinY = 0.f;
  float bridgeMaxY = 0.f;
  for (int i = -1; i < numSegments; ++i) {
    float y1 = i * segmentHeight + yPixelOffset;
    float y2 = y1 + segmentHeight;

    if (y2 < 0)
      continue;

    float z1 = getElevationAtScreenY(y1);
    float z2 = getElevationAtScreenY(y2);

    sf::Vector2f tl = project3D(100.f, y1, z1);
    sf::Vector2f tr = project3D(700.f, y1, z1);
    sf::Vector2f br = project3D(700.f, y2, z2);
    sf::Vector2f bl = project3D(100.f, y2, z2);

    int segmentIndex = baseIndex - i;
    int wrappedIndex = ((segmentIndex % 500) + 500) % 500;
    bool isBridge = (wrappedIndex >= 25 && wrappedIndex <= 75);

    bool isDark = (std::abs(segmentIndex) % 2 == 0);
    bool drawLaneLines = isDark || isBridge;

    sf::Color roadColor =
        isDark ? sf::Color(100, 100, 100) : sf::Color(90, 90, 90);
    sf::Color grassColor = isDark ? sf::Color(0, 110, 0) : sf::Color(0, 100, 0);

    if (isBridge) {
      grassColor = sf::Color(85, 85, 90); // Concrete deck sides
      roadColor = sf::Color(75, 75, 80); // Darker asphalt for visibility
    }

    float segmentLight = isBridge ? std::max(lightLevel, 0.55f) : lightLevel;

    // Apply lighting to road/grass segments
    roadColor.r *= segmentLight;
    roadColor.g *= segmentLight;
    roadColor.b *= segmentLight;
    grassColor.r *= segmentLight;
    grassColor.g *= segmentLight;
    grassColor.b *= segmentLight;
    
    float leftGrassX = isBridge ? 20.f : -2500.f;
    float rightGrassX = isBridge ? 780.f : 3300.f;

    if (isBridge) {
        // Draw the lake at z=0 
        sf::ConvexShape lake(4);
        sf::Color lakeColor = isDark ? sf::Color(20, 80, 180) : sf::Color(30, 90, 200);
        
        // Add subtle procedural water ripples
        int ripple = (int)(std::sin((y1 + mScrollY * 0.5f) * 0.05f) * 10.f);
        lakeColor.r = (sf::Uint8)std::clamp(lakeColor.r + ripple, 0, 255);
        lakeColor.g = (sf::Uint8)std::clamp(lakeColor.g + ripple, 0, 255);
        lakeColor.b = (sf::Uint8)std::clamp(lakeColor.b + ripple, 0, 255);

        lake.setFillColor(applyLight(lakeColor));
        lake.setPoint(0, project3D(-2500.f, y1, 0.f));
        lake.setPoint(1, project3D(3300.f, y1, 0.f));
        lake.setPoint(2, project3D(3300.f, y2, 0.f));
        lake.setPoint(3, project3D(-2500.f, y2, 0.f));
        mWindow.draw(lake);

        // --- DYNAMIC SUN/MOON SPECULAR WATER REFLECTIONS ---
        float sunPosT = mDayTime;
        if (sunPosT > 0.9f) sunPosT -= 1.0f;
        float sunAngle = (sunPosT + 0.1f) * 3.14159f / 0.6f;
        float moonAngle = (mDayTime - 0.4f) * 3.14159f / 0.5f;
        
        bool hasRefl = false;
        float sx = 400.f;
        sf::Color reflColor;
        
        if (sunAngle > 0 && sunAngle < 3.14159f) {
            sx = 400.f - std::cos(sunAngle) * 300.f;
            reflColor = sf::Color(255, 230, 150, 100);
            hasRefl = true;
        } else if (moonAngle > 0 && moonAngle < 3.14159f) {
            sx = 400.f - std::cos(moonAngle) * 300.f;
            reflColor = sf::Color(200, 200, 255, 70);
            hasRefl = true;
        }

        if (hasRefl) {
            // Draw shimmering horizontal light streaks on the water matching the sun/moon X-position!
            // The segment scaling naturally makes them perspective-correct towards the horizon
            if (isDark) { // Alternating segments to create the 'shimmering lines' effect
                float shimmerTime = mDayTime * 300.f;
                // Width fluctuates slightly
                float rw = 120.f + std::sin(wrappedIndex*0.5f + shimmerTime) * 30.f; 
                // X position drifts very slightly around the source light
                float driftX = sx + std::cos(wrappedIndex*0.3f + shimmerTime*1.5f) * 20.f;
                
                sf::ConvexShape reflection(4);
                reflection.setFillColor(reflColor);
                reflection.setPoint(0, project3D(driftX - rw, y1, 0.f));
                reflection.setPoint(1, project3D(driftX + rw, y1, 0.f));
                reflection.setPoint(2, project3D(driftX + rw, y2, 0.f));
                reflection.setPoint(3, project3D(driftX - rw, y2, 0.f));
                mWindow.draw(reflection);
            }
        }

        // Draw some lotus flowers on the lake
        for (int l = 0; l < 3; ++l) {
            int seed = wrappedIndex * 13 + l * 17;
            if (seed % 3 != 0) continue; 
            float flowerX = -1000.f + (float)(seed % 2800);
            
            float fy = y1 + segmentHeight * ((seed % 10) / 10.f);
            float fscale = (fy - -250.f) / (600.f - -250.f); 
            if (fscale < 0.05f) fscale = 0.05f;

            sf::Vector2f fPos = project3D(flowerX, fy, 0.f);
            
            // Draw Pad
            sf::CircleShape pad(30.f);
            pad.setFillColor(applyLight(sf::Color(20, 120, 50)));
            pad.setScale(fscale, fscale * 0.4f);
            pad.setOrigin(30.f, 30.f);
            pad.setPosition(fPos);
            mWindow.draw(pad);

            // Draw Flower
            sf::CircleShape flower(12.f);
            flower.setFillColor(applyLight(sf::Color(255, 105, 180)));
            flower.setScale(fscale, fscale * 0.6f);
            flower.setOrigin(12.f, 12.f);
            flower.setPosition(fPos.x, fPos.y - 2.f * fscale);
            mWindow.draw(flower);
        }
    }

    sf::ConvexShape grassL(4);
    grassL.setFillColor(grassColor);
    grassL.setPoint(0, project3D(leftGrassX, y1, z1));
    grassL.setPoint(1, project3D(100.f, y1, z1));
    grassL.setPoint(2, project3D(100.f, y2, z2));
    grassL.setPoint(3, project3D(leftGrassX, y2, z2));
    mWindow.draw(grassL);

    sf::ConvexShape grassR(4);
    grassR.setFillColor(grassColor);
    grassR.setPoint(0, project3D(700.f, y1, z1));
    grassR.setPoint(1, project3D(rightGrassX, y1, z1));
    grassR.setPoint(2, project3D(rightGrassX, y2, z2));
    grassR.setPoint(3, project3D(700.f, y2, z2));
    mWindow.draw(grassR);

    sf::ConvexShape road(4);
    road.setFillColor(roadColor);
    road.setPoint(0, tl);
    road.setPoint(1, tr);
    road.setPoint(2, br);
    road.setPoint(3, bl);
    mWindow.draw(road);

    sf::Color borderColor = isDark ? sf::Color::Red : sf::Color::White;
    if (isBridge) borderColor = sf::Color(130, 130, 135); // Brighter shoulder for bridge

    // Darken borders slightly less so they pop?
    borderColor.r *= segmentLight;
    borderColor.g *= segmentLight;
    borderColor.b *= segmentLight;

    sf::ConvexShape borderL(4);
    borderL.setFillColor(borderColor);
    borderL.setPoint(0, project3D(90.f, y1, z1));
    borderL.setPoint(1, project3D(110.f, y1, z1));
    borderL.setPoint(2, project3D(110.f, y2, z2));
    borderL.setPoint(3, project3D(90.f, y2, z2));
    mWindow.draw(borderL);

    sf::ConvexShape borderR(4);
    borderR.setFillColor(borderColor);
    borderR.setPoint(0, project3D(690.f, y1, z1));
    borderR.setPoint(1, project3D(710.f, y1, z1));
    borderR.setPoint(2, project3D(710.f, y2, z2));
    borderR.setPoint(3, project3D(690.f, y2, z2));
    mWindow.draw(borderR);
    
    if (isBridge) {
      bridgeVisible = true;
      if (!hasBridgeRange) {
        bridgeMinY = y1;
        bridgeMaxY = y2;
        hasBridgeRange = true;
      } else {
        if (y1 < bridgeMinY)
          bridgeMinY = y1;
        if (y2 > bridgeMaxY)
          bridgeMaxY = y2;
      }
        // Draw Metallic Guardrails elevated higher than the road surface
      float railH1 = z1 + 60.f; 
      float railH2 = z2 + 60.f;

        sf::Color railColor = applyLight(isDark ? sf::Color(190, 190, 200)
                                                : sf::Color(210, 210, 220));
        sf::Color railWallColor = applyLight(isDark ? sf::Color(130, 130, 140)
                                                    : sf::Color(150, 150, 160));
        
        // Left Guardrail
        sf::ConvexShape railL(4);
        railL.setFillColor(railColor);
        railL.setPoint(0, project3D(90.f, y1, railH1));
        railL.setPoint(1, project3D(110.f, y1, railH1));
        railL.setPoint(2, project3D(110.f, y2, railH2));
        railL.setPoint(3, project3D(90.f, y2, railH2));
        mWindow.draw(railL);
        
        // Right Guardrail
        sf::ConvexShape railR(4);
        railR.setFillColor(railColor);
        railR.setPoint(0, project3D(690.f, y1, railH1));
        railR.setPoint(1, project3D(710.f, y1, railH1));
        railR.setPoint(2, project3D(710.f, y2, railH2));
        railR.setPoint(3, project3D(690.f, y2, railH2));
        mWindow.draw(railR);
        
        // Solid wall bridging the guardrail down to the road edge
        sf::ConvexShape railWallL(4);
        railWallL.setFillColor(railWallColor);
        railWallL.setPoint(0, project3D(110.f, y1, railH1));
        railWallL.setPoint(1, project3D(110.f, y1, z1));
        railWallL.setPoint(2, project3D(110.f, y2, z2));
        railWallL.setPoint(3, project3D(110.f, y2, railH2));
        mWindow.draw(railWallL);
        
        sf::ConvexShape railWallR(4);
        railWallR.setFillColor(railWallColor);
        railWallR.setPoint(0, project3D(690.f, y1, railH1));
        railWallR.setPoint(1, project3D(690.f, y1, z1));
        railWallR.setPoint(2, project3D(690.f, y2, z2));
        railWallR.setPoint(3, project3D(690.f, y2, railH2));
        mWindow.draw(railWallR);

        // Add Bridge Pillars anchoring down to the lake
        if (wrappedIndex % 10 == 0) {
            sf::Color pillarColor = applyLight(isDark ? sf::Color(50, 50, 55) : sf::Color(60, 60, 65));
            // Left Pillar
            sf::ConvexShape pillarL(4);
            pillarL.setFillColor(pillarColor);
            pillarL.setPoint(0, project3D(leftGrassX + 40.f, y1, z1));
            pillarL.setPoint(1, project3D(100.f, y1, z1));
            pillarL.setPoint(2, project3D(100.f, y1, 0.f)); // Ground
            pillarL.setPoint(3, project3D(leftGrassX + 40.f, y1, 0.f));
            mWindow.draw(pillarL);

            // Right Pillar
            sf::ConvexShape pillarR(4);
            pillarR.setFillColor(pillarColor);
            pillarR.setPoint(0, project3D(700.f, y1, z1));
            pillarR.setPoint(1, project3D(rightGrassX - 40.f, y1, z1));
            pillarR.setPoint(2, project3D(rightGrassX - 40.f, y1, 0.f)); // Ground
            pillarR.setPoint(3, project3D(700.f, y1, 0.f));
            mWindow.draw(pillarR);
        }

        // Add Glowing Bridge Lights every few segments
        if (wrappedIndex % 5 == 0) {
            float lightHeight = railH1 + 100.f; // Pole height
            sf::Color poleColor = applyLight(sf::Color(40, 40, 45));
            
            // Left Pole
            sf::ConvexShape poleL(4);
            poleL.setFillColor(poleColor);
            poleL.setPoint(0, project3D(95.f, y1, lightHeight));
            poleL.setPoint(1, project3D(105.f, y1, lightHeight));
            poleL.setPoint(2, project3D(105.f, y1, railH1));
            poleL.setPoint(3, project3D(95.f, y1, railH1));
            mWindow.draw(poleL);

            // Right Pole
            sf::ConvexShape poleR(4);
            poleR.setFillColor(poleColor);
            poleR.setPoint(0, project3D(695.f, y1, lightHeight));
            poleR.setPoint(1, project3D(705.f, y1, lightHeight));
            poleR.setPoint(2, project3D(705.f, y1, railH1));
            poleR.setPoint(3, project3D(695.f, y1, railH1));
            mWindow.draw(poleR);

            // Glowing bulbs
            float scaleL = (y1 - -250.f) / (600.f - -250.f);
            if (scaleL < 0.05f) scaleL = 0.05f;

            sf::Vector2f glowPosL = project3D(100.f, y1, lightHeight);
            sf::Vector2f glowPosR = project3D(700.f, y1, lightHeight);

            // Turn on lights at night (lightLevel < 0.6)
            sf::Color bulbColor = (lightLevel < 0.6f) ? sf::Color(255, 255, 150) : applyLight(sf::Color(200, 200, 200));
            
            sf::CircleShape bulbL(15.f);
            bulbL.setOrigin(15.f, 15.f);
            bulbL.setScale(scaleL, scaleL);
            bulbL.setPosition(glowPosL);
            bulbL.setFillColor(bulbColor);
            mWindow.draw(bulbL);

            sf::CircleShape bulbR(15.f);
            bulbR.setOrigin(15.f, 15.f);
            bulbR.setScale(scaleL, scaleL);
            bulbR.setPosition(glowPosR);
            bulbR.setFillColor(bulbColor);
            mWindow.draw(bulbR);

            if (lightLevel < 0.6f) { // Outer glow at night
                sf::CircleShape haloL(40.f);
                haloL.setOrigin(40.f, 40.f);
                haloL.setScale(scaleL, scaleL);
                haloL.setPosition(glowPosL);
                haloL.setFillColor(sf::Color(255, 200, 50, 60));
                mWindow.draw(haloL);
                
                sf::CircleShape haloR(40.f);
                haloR.setOrigin(40.f, 40.f);
                haloR.setScale(scaleL, scaleL);
                haloR.setPosition(glowPosR);
                haloR.setFillColor(sf::Color(255, 200, 50, 60));
                mWindow.draw(haloR);
            }
        }
    }
    
    // Draw bridge side connection walls so you see height!
    if (z1 > 0 || z2 > 0) {
        sf::ConvexShape wallL(4);
        wallL.setFillColor(applyLight(sf::Color(75, 75, 80)));
        wallL.setPoint(0, project3D(leftGrassX, y1, z1)); 
        wallL.setPoint(1, project3D(leftGrassX, y2, z2));
        wallL.setPoint(2, project(leftGrassX, y2)); // Base water level
        wallL.setPoint(3, project(leftGrassX, y1)); // Base water level
        mWindow.draw(wallL);

        sf::ConvexShape wallR(4);
        wallR.setFillColor(applyLight(sf::Color(60, 60, 65))); // Shaded bit darker
        wallR.setPoint(0, project3D(rightGrassX, y1, z1));
        wallR.setPoint(1, project3D(rightGrassX, y2, z2));
        wallR.setPoint(2, project(rightGrassX, y2)); // Base water level
        wallR.setPoint(3, project(rightGrassX, y1)); // Base water level
        mWindow.draw(wallR);
    }

    if (drawLaneLines) {
      sf::Color laneColor = sf::Color(255, 255, 255, 200);
      laneColor.r *= lightLevel;
      laneColor.g *= lightLevel;
      laneColor.b *= lightLevel;

      sf::ConvexShape lane1(4);
      lane1.setFillColor(laneColor);
      lane1.setPoint(0, project3D(248.f, y1 + 20.f, getElevationAtScreenY(y1 + 20.f)));
      lane1.setPoint(1, project3D(252.f, y1 + 20.f, getElevationAtScreenY(y1 + 20.f)));
      lane1.setPoint(2, project3D(252.f, y2 - 20.f, getElevationAtScreenY(y2 - 20.f)));
      lane1.setPoint(3, project3D(248.f, y2 - 20.f, getElevationAtScreenY(y2 - 20.f)));
      mWindow.draw(lane1);

      sf::ConvexShape lane3(4);
      lane3.setFillColor(laneColor);
      lane3.setPoint(0, project3D(548.f, y1 + 20.f, getElevationAtScreenY(y1 + 20.f)));
      lane3.setPoint(1, project3D(552.f, y1 + 20.f, getElevationAtScreenY(y1 + 20.f)));
      lane3.setPoint(2, project3D(552.f, y2 - 20.f, getElevationAtScreenY(y2 - 20.f)));
      lane3.setPoint(3, project3D(548.f, y2 - 20.f, getElevationAtScreenY(y2 - 20.f)));
      mWindow.draw(lane3);
    }

    sf::ConvexShape center(4);
    center.setFillColor(sf::Color(255, 204, 0));
    center.setPoint(0, project3D(396.f, y1, z1));
    center.setPoint(1, project3D(404.f, y1, z1));
    center.setPoint(2, project3D(404.f, y2, z2));
    center.setPoint(3, project3D(396.f, y2, z2));
    mWindow.draw(center);
  }

  std::sort(mProps.begin(), mProps.end(),
            [](const Prop &a, const Prop &b) { return a.pos.y < b.pos.y; });

  for (const auto &p : mProps) {
    if (p.pos.y < 0 || p.pos.y > 600)
      continue;
      
    int propAbsolute = ((int)(mScrollY - p.pos.y) % 10000 + 10000) % 10000;
    if (propAbsolute >= 4000 && propAbsolute <= 6000)
      continue; // Don't draw over water

    float z = getElevationAtScreenY(p.pos.y);
    sf::Vector2f screenPos = project3D(p.pos.x, p.pos.y, z);
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
