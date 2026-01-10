#include "ResourceManager.h"
#include <iostream>

void ResourceManager::loadTexture(const std::string &name,
                                  const std::string &filename) {
  sf::Texture texture;
  if (texture.loadFromFile(filename)) {
    mTextures[name] = texture;
  } else {
    std::cerr << "Failed to load texture: " << filename << std::endl;
  }
}

sf::Texture &ResourceManager::getTexture(const std::string &name) {
  if (mTextures.find(name) == mTextures.end()) {
    static sf::Texture empty;
    return empty;
  }
  return mTextures[name];
}

void ResourceManager::loadFont(const std::string &name,
                               const std::string &filename) {
  sf::Font font;
  if (font.loadFromFile(filename)) {
    mFonts[name] = font;
  } else {
    std::cerr << "Failed to load font: " << filename << std::endl;
  }
}

sf::Font &ResourceManager::getFont(const std::string &name) {
  if (mFonts.find(name) == mFonts.end()) {
    static sf::Font empty;
    return empty;
  }
  return mFonts.at(name);
}

void ResourceManager::loadSoundBuffer(const std::string &name,
                                      const std::string &filename) {
  sf::SoundBuffer buffer;
  if (buffer.loadFromFile(filename)) {
    mSoundBuffers[name] = buffer;
  } else {
    std::cerr << "Failed to load sound buffer: " << filename << std::endl;
  }
}

sf::SoundBuffer &ResourceManager::getSoundBuffer(const std::string &name) {
  if (mSoundBuffers.find(name) == mSoundBuffers.end()) {
    static sf::SoundBuffer empty;
    return empty;
  }
  return mSoundBuffers[name];
}
