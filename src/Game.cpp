#include "Game.h"
#include "ResourceManager.h"
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>

static sf::Vector2f projectToScreen(sf::Vector2f worldPos) {
  float centerX = 400.f;
  float horizonY = -250.f;
  float camY = 600.f;

  float scale = (worldPos.y - horizonY) / (camY - horizonY);
  float screenX = centerX + (worldPos.x - centerX) * scale;
  return sf::Vector2f(screenX, worldPos.y);
}


Game::Game()
    : mWindow(sf::VideoMode(800, 600), "AI Racing Game - C++ SFML"),
      mState(Menu), mScore(0.f), mDistance(0.f), mCarsPassed(0), mLevel(1), mHighScore(0),
      mHighScoreName("None"), mIsNewHighScore(false), mScreenShakeTimer(0.f) {
  mWindow.setFramerateLimit(60);
  mGameView = mWindow.getDefaultView();
  std::srand(static_cast<unsigned>(std::time(nullptr)));

  mWorld = std::make_shared<World>(mWindow);
  mPlayer = std::make_shared<PlayerCar>();

  mSpawnInterval = sf::seconds(2.f);
  mSpawnTimer = sf::Time::Zero;

  loadHighScore();

  ResourceManager &res = ResourceManager::getInstance();
  res.loadFont("MainFont", "assets/fonts/arial.ttf");

  res.loadSoundBuffer("Engine", "assets/audio/engine.wav");
  res.loadSoundBuffer("Crash", "assets/audio/crash.wav");

  try {
    mEngineSound.setBuffer(res.getSoundBuffer("Engine"));
    mEngineSound.setLoop(true);
    mEngineSound.setVolume(50.f);

    mCrashSound.setBuffer(res.getSoundBuffer("Crash"));
  } catch (...) {
    std::cout << "Audio files missing. Sound disabled." << std::endl;
  }

  mFont = res.getFont("MainFont");

  mScoreText.setFont(mFont);
  mScoreText.setCharacterSize(22);
  mScoreText.setFillColor(sf::Color::White);
  mScoreText.setPosition(20.f, 15.f);
  mScoreText.setStyle(sf::Text::Bold);

  mSpeedText.setFont(mFont);
  mSpeedText.setCharacterSize(22);
  mSpeedText.setFillColor(sf::Color(255, 215, 0));
  mSpeedText.setPosition(20.f, 45.f);
  mSpeedText.setStyle(sf::Text::Italic | sf::Text::Bold);

  mHighScoreText.setFont(mFont);
  mHighScoreText.setCharacterSize(22);
  mHighScoreText.setFillColor(sf::Color::Cyan);
  mHighScoreText.setPosition(20.f, 75.f);
  mHighScoreText.setStyle(sf::Text::Bold);

  mLevelText.setFont(mFont);
  mLevelText.setCharacterSize(36);
  mLevelText.setFillColor(sf::Color(255, 165, 0));
  mLevelText.setStyle(sf::Text::Bold);
  mLevelText.setOutlineColor(sf::Color::Black);
  mLevelText.setOutlineThickness(2.f);

  mMenuText.setFont(mFont);
  mMenuText.setCharacterSize(40);
  mMenuText.setFillColor(sf::Color::White);
  mMenuText.setOutlineColor(sf::Color::Black);
  mMenuText.setOutlineThickness(2.f);
  mMenuText.setString("STREET RACER\n\nPress ENTER to Start\nWASD to Drive");

  sf::FloatRect textRect = mMenuText.getLocalBounds();
  mMenuText.setOrigin(textRect.left + textRect.width / 2.0f,
                      textRect.top + textRect.height / 2.0f);
  mMenuText.setPosition(sf::Vector2f(800 / 2.0f, 600 / 2.0f));

  mGameOverText.setFont(mFont);
  mGameOverText.setCharacterSize(40);
  mGameOverText.setFillColor(sf::Color::Red);
  mGameOverText.setOutlineColor(sf::Color::White);
  mGameOverText.setOutlineThickness(2.f);

  mInputNameText.setFont(mFont);
  mInputNameText.setCharacterSize(30);
  mInputNameText.setFillColor(sf::Color::Green);
  mInputNameText.setPosition(200.f, 400.f);
}

Game::~Game() {}

void Game::run() {
  sf::Clock clock;
  sf::Time timeSinceLastUpdate = sf::Time::Zero;

  while (mWindow.isOpen()) {
    sf::Time elapsedTime = clock.restart();
    timeSinceLastUpdate += elapsedTime;
    while (timeSinceLastUpdate > TimePerFrame) {
      timeSinceLastUpdate -= TimePerFrame;
      processEvents();
      update(TimePerFrame);
    }
    render();
  }
}

void Game::processEvents() {
  sf::Event event;
  while (mWindow.pollEvent(event)) {
    if (event.type == sf::Event::Closed)
      mWindow.close();

    if (event.type == sf::Event::TextEntered) {
      if (mState == InputName) {
        if (event.text.unicode == '\b') {
          if (!mCurrentInputName.empty())
            mCurrentInputName.pop_back();
        } else if (event.text.unicode == '\r') {

          mHighScoreName = mCurrentInputName;
          if (mHighScoreName.empty())
            mHighScoreName = "Player";
          saveHighScore();
          mState = GameOver;
        } else if (event.text.unicode < 128 &&
                   mCurrentInputName.length() < 10) {
          mCurrentInputName += static_cast<char>(event.text.unicode);
        }
      }
    }

    if (event.type == sf::Event::KeyPressed) {
      if (mState == Menu && event.key.code == sf::Keyboard::Enter) {
        mState = Playing;
      }
      if (mState == GameOver && event.key.code == sf::Keyboard::R) {

        mState = Playing;
        mEnemies.clear();
        mPlayer->setPosition(400.f, 500.f);
        mScore = 0.f;
        mDistance = 0.f;
        mCarsPassed = 0;
        mLevel = 1;
        mIsNewHighScore = false;

        if (mEngineSound.getStatus() != sf::Sound::Playing)
          mEngineSound.play();
      }
    }
  }
}

void Game::spawnParticles(sf::Vector2f pos, int count, sf::Color color) {
  for (int i = 0; i < count; ++i) {
    Particle p;
    p.pos = pos;
    float angle = (std::rand() % 360) * 3.14159f / 180.f;
    float speed = 100.f + (std::rand() % 200);
    p.vel = sf::Vector2f(std::cos(angle) * speed, std::sin(angle) * speed);
    p.life = 0.5f + ((std::rand() % 100) / 200.f);
    p.color = color;
    mParticles.push_back(p);
  }
}

void Game::updateParticles(sf::Time dt) {
  for (auto it = mParticles.begin(); it != mParticles.end();) {
    it->pos += it->vel * dt.asSeconds();
    it->life -= dt.asSeconds();
    if (it->life <= 0) {
      it = mParticles.erase(it);
    } else {
      ++it;
    }
  }
}

void Game::update(sf::Time deltaTime) {
  if (mState != Playing)
    return;

  mGameView.setCenter(400.f, 300.f);

  updateParticles(deltaTime);
  mWorld->update(deltaTime);
  mPlayer->update(deltaTime);

  // Difficulty & Speed
  float difficultyScalar = 1.0f + (mCarsPassed / 2) * 0.1f;
  float currentWorldSpeed = 200.f * difficultyScalar;
  mWorld->setScrollSpeed(currentWorldSpeed);
  for (auto &enemy : mEnemies) {
    enemy->setWorldSpeed(currentWorldSpeed);
  }

  // Camera Tilt and Zoom disabled to prevent screen background fluctuation
  mGameView.setRotation(0.f);
  mGameView.setSize(800.f, 600.f);

  // Screen Shake (Applied LAST)
  if (mScreenShakeTimer > 0.f) {
    mScreenShakeTimer -= deltaTime.asSeconds();
    float progress = 0.5f - mScreenShakeTimer;
    float shakeIntensity = 10.f * (mScreenShakeTimer / 0.5f);

    // Smooth Sine Wave Shake
    float offsetX = std::sin(progress * 50.f) * shakeIntensity;
    float offsetY = std::cos(progress * 43.f) * shakeIntensity;

    mGameView.move(offsetX, offsetY);
  }

  mWindow.setView(mGameView);

  // 3. Dynamic Engine Pitch
  float pitch = 0.5f + (currentWorldSpeed / 800.f);
  if (pitch > 2.0f)
    pitch = 2.0f;
  mEngineSound.setPitch(pitch);

  mDistance += currentWorldSpeed * 0.1f * deltaTime.asSeconds();
  mScore = mDistance * 10.f;
  if ((int)mScore > mHighScore) {
    mHighScore = (int)mScore;
    mIsNewHighScore = true;
  }

  if (currentWorldSpeed > 100.f) {
    for (int t = 0; t < 2; t++) {
      float tireOffset = (t == 0) ? -20.f : 20.f;

      for (int i = 0; i < 3; ++i) {
        Particle p;
        float jitterX = (std::rand() % 10) - 5.f;
        float jitterY = (std::rand() % 10) - 5.f;

        sf::Vector2f worldPos =
            mPlayer->getPosition() +
            sf::Vector2f(tireOffset + jitterX, 35.f + jitterY);

        p.pos = projectToScreen(worldPos);

        float velX = (std::rand() % 40) - 20.f;
        p.vel =
            sf::Vector2f(velX, currentWorldSpeed + 200.f + (std::rand() % 100));

        p.life = 0.3f;
        int g = std::rand() % 200;
        p.color = sf::Color(255, g, 0, 150);
        mParticles.push_back(p);
      }
    }
  }

  std::stringstream ss;
  ss << "Score: " << (int)mScore;
  mScoreText.setString(ss.str());

  std::stringstream ssSpeed;
  int displaySpeed = 60 + (mCarsPassed * 5);
  ssSpeed << "Speed: " << displaySpeed << " km/h";
  mSpeedText.setString(ssSpeed.str());

  std::stringstream ssHS;
  ssHS << "High Score: " << mHighScore << " (" << mHighScoreName << ")";
  if (mIsNewHighScore)
    ssHS << " NEW!";
  mHighScoreText.setString(ssHS.str());

  mLevel = 1 + (mCarsPassed / 10);
  std::stringstream ssLevel;
  ssLevel << "LEVEL " << mLevel;
  mLevelText.setString(ssLevel.str());

  mSpawnTimer += deltaTime;

  float adjustedInterval = std::max(0.5f, 2.0f - (mCarsPassed * 0.05f));

  if (mSpawnTimer.asSeconds() > adjustedInterval) {
    float lanes[] = {175.f, 325.f, 475.f, 625.f};

    int attempts = 0;
    bool spawned = false;

    while (attempts < 4 && !spawned) {
      int laneIndex = rand() % 4;
      float laneX = lanes[laneIndex];

      bool overlaps = false;
      for (const auto &existing : mEnemies) {
        if (std::abs(existing->getPosition().x - laneX) < 50.f) {
          if (existing->getPosition().y > -400.f && existing->getPosition().y < 200.f) {
            overlaps = true;
            break;
          }
        }
      }

      if (!overlaps) {
        EnemyCar::Direction dir =
            (laneIndex < 2) ? EnemyCar::SameWay : EnemyCar::Oncoming;

        float baseSpeed = 160.f + (rand() % 140);

        if (dir == EnemyCar::SameWay)
          baseSpeed = 120.f + (rand() % 100);
        float speed = baseSpeed * difficultyScalar;

        auto enemy = std::make_shared<EnemyCar>(laneX, -150.f, speed, dir);
        mEnemies.push_back(enemy);

        mSpawnTimer = sf::Time::Zero;
        spawned = true;
      }
      attempts++;
    }
  }

  std::vector<std::shared_ptr<Entity>> allEntities;
  allEntities.push_back(mPlayer);
  for (const auto &e : mEnemies)
    allEntities.push_back(e);

  for (auto it = mEnemies.begin(); it != mEnemies.end();) {

    (*it)->update(deltaTime);

    if ((*it)->getBounds().intersects(mPlayer->getBounds())) {
      if (mCrashSound.getStatus() != sf::Sound::Playing)
        mCrashSound.play();
      mEngineSound.stop();

      mScreenShakeTimer = 0.5f;
      spawnParticles(projectToScreen(mPlayer->getPosition()), 50,
                     sf::Color(255, 100, 0));

      mIsNewHighScore = true;
      mState = InputName;
      mCurrentInputName = "";
    }

    if ((*it)->getPosition().y > 800) {
      if ((*it)->getDirection() == EnemyCar::SameWay) {
        mScore += 10;
        mCarsPassed++;
        spawnParticles(projectToScreen(mPlayer->getPosition()), 10,
                       sf::Color::Yellow);
      } else {
        mCarsPassed++;
        mScore += 100;
      }

      it = mEnemies.erase(it);
    } else if ((*it)->getPosition().y < -600.f) {
      it = mEnemies.erase(it);
    } else {
      ++it;
    }
  }
}

void Game::render() {
  mWindow.clear(sf::Color(30, 30, 30));

  if (mState == Menu) {
    mWorld->draw();
    sf::RectangleShape overlay(sf::Vector2f(800, 600));
    overlay.setFillColor(sf::Color(0, 0, 0, 200));
    mWindow.draw(overlay);
    mWindow.draw(mMenuText);
  } else {
    mWorld->draw();

    for (const auto &p : mParticles) {
      float z = mWorld->getElevationAtScreenY(p.pos.y);
      // Let's create an external project function since particles are local here?
      // Wait, particles pos is ALREADY projected in spawn/update.
      // But we can just offset it manually by z * scale?
      // Actually particles are currently drawn with p.pos.
      float size = std::max(1.f, p.life * 15.f);
      sf::RectangleShape rect(sf::Vector2f(size, size));
      rect.setOrigin(size / 2.f, size / 2.f);
      
      // Calculate scale to offset the particle. 
      // y is screen space, scale = (y - (-250.f)) / (600.f - (-250.f))
      float scale = (p.pos.y + 250.f) / 850.f;
      sf::Vector2f finalPos = p.pos;
      finalPos.y -= z * scale;
      rect.setPosition(finalPos);

      sf::Color c = p.color;
      c.a = static_cast<sf::Uint8>(std::min(255.f, p.life * 500.f));
      rect.setFillColor(c);

      mWindow.draw(rect);
    }

    for (const auto &enemy : mEnemies) {
      float z = mWorld->getElevationAtScreenY(enemy->getPosition().y);
      enemy->draw(mWindow, z);
    }
    float playerZ = mWorld->getElevationAtScreenY(mPlayer->getPosition().y);
    mPlayer->draw(mWindow, playerZ);
    {
      // We need to draw this in "Screen Coordinates", so we reset view
      // temporarily
      sf::View currentView = mWindow.getView();
      mWindow.setView(mWindow.getDefaultView());

      sf::VertexArray vignette(sf::Quads, 4);
      vignette[0].position = sf::Vector2f(0.f, 0.f);
      vignette[1].position = sf::Vector2f(800.f, 0.f);
      vignette[2].position = sf::Vector2f(800.f, 600.f);
      vignette[3].position = sf::Vector2f(0.f, 600.f);

      // Center transparent, corners dark? Vertex colors interpolate linearly.
      // A single quad won't give a radial dark corner.
      // It will just be a linear gradient.
      // Attempt: 4 Triangles meeting at center.
      // Center: (400, 300) Alpha 0.
      // Corners: Alpha 200.

      sf::VertexArray v(sf::TrianglesFan, 6);
      v[0].position = sf::Vector2f(400.f, 300.f); // Center
      v[0].color = sf::Color(0, 0, 0, 0);

      v[1].position = sf::Vector2f(0.f, 0.f);
      v[1].color = sf::Color(0, 0, 0, 80);

      v[2].position = sf::Vector2f(800.f, 0.f);
      v[2].color = sf::Color(0, 0, 0, 80);

      v[3].position = sf::Vector2f(800.f, 600.f);
      v[3].color = sf::Color(0, 0, 0, 80);

      v[4].position = sf::Vector2f(0.f, 600.f);
      v[4].color = sf::Color(0, 0, 0, 80);

      v[5].position = sf::Vector2f(0.f, 0.f); // Close loop
      v[5].color = sf::Color(0, 0, 0, 80);

      mWindow.draw(v);

      mWindow.setView(currentView);
    }
    // Switch to Default View for HUD
    sf::View worldView = mWindow.getView();
    mWindow.setView(mWindow.getDefaultView());

    // 1. Score Panel (Top Left, Compact)
    sf::RectangleShape scorePanel(sf::Vector2f(260.f, 80.f));
    scorePanel.setFillColor(sf::Color(20, 20, 20, 150));
    scorePanel.setOutlineColor(sf::Color(255, 255, 255, 50));
    scorePanel.setOutlineThickness(1.f);
    scorePanel.setPosition(10.f, 10.f);
    mWindow.draw(scorePanel);

    mScoreText.setPosition(20.f, 15.f);
    mScoreText.setCharacterSize(18);
    mHighScoreText.setPosition(20.f, 45.f);
    mHighScoreText.setCharacterSize(16);
    mHighScoreText.setFillColor(sf::Color(100, 255, 255));

    mLevelText.setPosition(800.f - mLevelText.getLocalBounds().width - 20.f, 20.f);

    mWindow.draw(mScoreText);
    mWindow.draw(mHighScoreText);
    mWindow.draw(mLevelText);

    // Common Dial Properties
    float dialRadius = 60.f;
    sf::Color dialColor(10, 10, 10, 200);
    sf::Color tickColor(200, 200, 200);

    // 2. RPM Gauge (Bottom Left)
    {
      sf::Vector2f rpmPos(100.f, 520.f);

      sf::CircleShape rpmDial(dialRadius);
      rpmDial.setOrigin(dialRadius, dialRadius);
      rpmDial.setPosition(rpmPos);
      rpmDial.setFillColor(dialColor);
      rpmDial.setOutlineColor(sf::Color::White);
      rpmDial.setOutlineThickness(2.f);
      mWindow.draw(rpmDial);

      // Calculate RPM (Simulated Gear Shift)
      int displaySpeed = 60 + (mCarsPassed * 5); // Base logic
      // Gear logic: Shift every 60 km/h?
      // 0-60: Gear 1. 60-120: Gear 2.
      float normalizedInGear = std::fmod((float)displaySpeed, 60.f) / 60.f;
      // prevent perfect 0 at 60, 120 etc.
      if (displaySpeed > 0 && (displaySpeed % 60 == 0))
        normalizedInGear = 1.0f;

      // RPM Range: 1000 (Idle) to 7000 (Redline)
      float currentRPM = 1000.f + normalizedInGear * 6000.f;

      // Needle
      float minRPM = 0.f;
      float maxRPM = 8000.f;
      float rpmRatio = (currentRPM - minRPM) / (maxRPM - minRPM);
      float angle = 135.f + rpmRatio * 270.f;

      // Draw Ticks (RPM)
      for (int i = 0; i <= 8; ++i) {
        float t = i / 8.f;
        float a = (135.f + t * 270.f) * 3.14159f / 180.f;
        float rIn = dialRadius - 10.f;
        float rOut = dialRadius - 2.f;
        sf::Vertex line[] = {
            sf::Vertex(rpmPos +
                           sf::Vector2f(std::cos(a) * rIn, std::sin(a) * rIn),
                       tickColor),
            sf::Vertex(rpmPos +
                           sf::Vector2f(std::cos(a) * rOut, std::sin(a) * rOut),
                       tickColor)};
        mWindow.draw(line, 2, sf::Lines);
      }

      sf::RectangleShape needle(sf::Vector2f(dialRadius - 10.f, 3.f));
      needle.setFillColor(sf::Color::Red);
      needle.setOrigin(0.f, 1.5f);
      needle.setPosition(rpmPos);
      needle.setRotation(angle);
      mWindow.draw(needle);

      // Text
      sf::Text rpmText;
      rpmText.setFont(mFont);
      rpmText.setString("RPM");
      rpmText.setCharacterSize(14);
      rpmText.setFillColor(sf::Color::White);
      sf::FloatRect tr = rpmText.getLocalBounds();
      rpmText.setOrigin(tr.width / 2.f, tr.height / 2.f);
      rpmText.setPosition(rpmPos.x, rpmPos.y + 35.f);
      mWindow.draw(rpmText);

      // Digital RPM? Nah, just label.
    }

    // 3. Speed Gauge (Bottom Right)
    {
      sf::Vector2f speedPos(700.f, 520.f);

      sf::CircleShape speedDial(dialRadius);
      speedDial.setOrigin(dialRadius, dialRadius);
      speedDial.setPosition(speedPos);
      speedDial.setFillColor(dialColor);
      speedDial.setOutlineColor(sf::Color::White);
      speedDial.setOutlineThickness(2.f);
      mWindow.draw(speedDial);

      int displaySpeed = 60 + (mCarsPassed * 5);
      float minSpeed = 0.f;
      float maxSpeed = 240.f;
      float speedRatio = (displaySpeed - minSpeed) / (maxSpeed - minSpeed);
      if (speedRatio > 1.f)
        speedRatio = 1.f;

      float angle = 135.f + speedRatio * 270.f;

      // Draw Ticks (Speed)
      for (int i = 0; i <= 12; ++i) { // Every 20 unit
        float t = i / 12.f;
        float a = (135.f + t * 270.f) * 3.14159f / 180.f;
        float rIn = dialRadius - 10.f;
        float rOut = dialRadius - 2.f;

        // Make every 2nd tick longer
        if (i % 2 == 0)
          rIn -= 5.f;

        sf::Vertex line[] = {
            sf::Vertex(speedPos +
                           sf::Vector2f(std::cos(a) * rIn, std::sin(a) * rIn),
                       tickColor),
            sf::Vertex(speedPos +
                           sf::Vector2f(std::cos(a) * rOut, std::sin(a) * rOut),
                       tickColor)};
        mWindow.draw(line, 2, sf::Lines);
      }

      sf::RectangleShape needle(sf::Vector2f(dialRadius - 10.f, 3.f));
      needle.setFillColor(sf::Color::Red);
      needle.setOrigin(0.f, 1.5f);
      needle.setPosition(speedPos);
      needle.setRotation(angle);
      mWindow.draw(needle);

      // Hub
      sf::CircleShape hub(6.f);
      hub.setOrigin(6.f, 6.f);
      hub.setPosition(speedPos);
      hub.setFillColor(sf::Color::White);
      mWindow.draw(hub);

      // Digital Speed
      mSpeedText.setString(std::to_string(displaySpeed) + "\nkm/h");
      mSpeedText.setCharacterSize(18);
      mSpeedText.setFillColor(sf::Color::Yellow);
      sf::FloatRect sr = mSpeedText.getLocalBounds();
      mSpeedText.setOrigin(sr.width / 2.f, sr.height / 2.f);
      mSpeedText.setPosition(speedPos.x, speedPos.y + 40.f);
      mWindow.draw(mSpeedText);
    }

    // Restore World View (though unlikely needed as we loop)
    mWindow.setView(worldView);

    if (mState == InputName) {
      sf::RectangleShape overlay(sf::Vector2f(800, 600));
      overlay.setFillColor(sf::Color(0, 0, 0, 200));
      mWindow.draw(overlay);

      std::stringstream ss;
      ss << "NEW HIGH SCORE: " << (int)mScore << "\n\nEnter Your Name:\n"
         << mCurrentInputName << "_";
      mInputNameText.setString(ss.str());

      sf::FloatRect textRect = mInputNameText.getLocalBounds();
      mInputNameText.setOrigin(textRect.left + textRect.width / 2.0f,
                               textRect.top + textRect.height / 2.0f);
      mInputNameText.setPosition(400.f, 300.f);
      mWindow.draw(mInputNameText);
    } else if (mState == GameOver) {
      sf::RectangleShape overlay(sf::Vector2f(800, 600));
      overlay.setFillColor(sf::Color(50, 0, 0, 180));
      mWindow.draw(overlay);

      sf::RectangleShape box(sf::Vector2f(500.f, 250.f));
      box.setFillColor(sf::Color::Black);
      box.setOutlineColor(sf::Color::Red);
      box.setOutlineThickness(4.f);
      box.setOrigin(250.f, 125.f);
      box.setPosition(400.f, 300.f);

      std::stringstream goSS;
      goSS << "CRASHED!\n";
      if (mIsNewHighScore)
        goSS << "NEW HIGH SCORE SET!\n";
      goSS << "Score: " << (int)mScore << "\n";
      goSS << "Best: " << mHighScore << " (" << mHighScoreName << ")\n";
      goSS << "Press R to Restart";
      mGameOverText.setString(goSS.str());

      sf::FloatRect goRect = mGameOverText.getLocalBounds();
      mGameOverText.setOrigin(goRect.left + goRect.width / 2.0f,
                              goRect.top + goRect.height / 2.0f);
      mGameOverText.setPosition(400.f, 300.f);

      mWindow.draw(box);
      mWindow.draw(mGameOverText);
    }
  }

  mWindow.display();
}

void Game::handleInput() {}

void Game::loadHighScore() {
  std::ifstream file("highscore.txt");
  if (file.is_open()) {
    if (!(file >> mHighScore))
      mHighScore = 0;
    std::getline(file, mHighScoreName);
    std::getline(file, mHighScoreName);
    if (mHighScoreName.empty())
      mHighScoreName = "Player";
    file.close();
  }
}

void Game::saveHighScore() {
  std::ofstream file("highscore.txt");
  if (file.is_open()) {
    file << mHighScore << "\n" << mHighScoreName;
    file.close();
  }
}
