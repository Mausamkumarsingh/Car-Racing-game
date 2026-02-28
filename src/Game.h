#include "Entities/EnemyCar.h"
#include "Entities/PlayerCar.h"
#include "World.h"
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <memory>
#include <vector>

class Game {
public:
  Game();
  ~Game();

  void run();

private:
  void processEvents();
  void update(sf::Time deltaTime);
  void render();

  void handleInput();

private:
  sf::RenderWindow mWindow;
  const sf::Time TimePerFrame = sf::seconds(1.f / 60.f);

  std::shared_ptr<World> mWorld;
  std::shared_ptr<PlayerCar> mPlayer;
  std::vector<std::shared_ptr<EnemyCar>> mEnemies;

  sf::Time mSpawnTimer;
  sf::Time mSpawnInterval;

  enum State { Menu, Playing, GameOver, InputName };
  State mState;

  int mHighScore;
  std::string mHighScoreName;
  std::string mCurrentInputName;
  bool mIsNewHighScore;

  void loadHighScore();
  void saveHighScore();

  sf::Font mFont;
  sf::Text mScoreText;
  sf::Text mSpeedText;
  sf::Text mHighScoreText;
  sf::Text mLevelText;

  sf::Text mMenuText;
  sf::Text mGameOverText;
  sf::Text mInputNameText;

  sf::Sound mEngineSound;
  sf::Sound mCrashSound;

  float mScore;
  float mDistance;
  int mCarsPassed;
  int mLevel;

  float mScreenShakeTimer;
  sf::View mGameView;

  struct Particle {
    sf::Vector2f pos;
    sf::Vector2f vel;
    float life;
    sf::Color color;
  };
  std::vector<Particle> mParticles;
  void spawnParticles(sf::Vector2f pos, int count, sf::Color color);
  void updateParticles(sf::Time dt);
};
