#pragma once
#include <SFML/Audio.hpp>
#include <SFML/Graphics.hpp>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>

class ResourceManager {
public:
  static ResourceManager &getInstance() {
    static ResourceManager instance;
    return instance;
  }

  void loadTexture(const std::string &name, const std::string &filename);
  sf::Texture &getTexture(const std::string &name);

  void loadFont(const std::string &name, const std::string &filename);
  sf::Font &getFont(const std::string &name);

  void loadSoundBuffer(const std::string &name, const std::string &filename);
  sf::SoundBuffer &getSoundBuffer(const std::string &name);

  ResourceManager(const ResourceManager &) = delete;
  void operator=(const ResourceManager &) = delete;

private:
  ResourceManager() = default;

  std::map<std::string, sf::Texture> mTextures;
  std::map<std::string, sf::Font> mFonts;
  std::map<std::string, sf::SoundBuffer> mSoundBuffers;
};
