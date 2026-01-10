#include <cmath>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>


// Minimal WAV header writing
void writeWav(const std::string &filename, const std::vector<short> &samples,
              int sampleRate) {
  std::ofstream file(filename, std::ios::binary);

  int dataSize = samples.size() * 2;
  int fileSize = 36 + dataSize;

  file.write("RIFF", 4);
  file.write(reinterpret_cast<const char *>(&fileSize), 4);
  file.write("WAVE", 4);
  file.write("fmt ", 4);

  int fmtChunkSize = 16;
  short audioFormat = 1; // PCM
  short numChannels = 1;
  int byteRate = sampleRate * 2;
  short blockAlign = 2;
  short bitsPerSample = 16;

  file.write(reinterpret_cast<const char *>(&fmtChunkSize), 4);
  file.write(reinterpret_cast<const char *>(&audioFormat), 2);
  file.write(reinterpret_cast<const char *>(&numChannels), 2);
  file.write(reinterpret_cast<const char *>(&sampleRate), 4);
  file.write(reinterpret_cast<const char *>(&byteRate), 4);
  file.write(reinterpret_cast<const char *>(&blockAlign), 2);
  file.write(reinterpret_cast<const char *>(&bitsPerSample), 2);

  file.write("data", 4);
  file.write(reinterpret_cast<const char *>(&dataSize), 4);
  file.write(reinterpret_cast<const char *>(samples.data()), dataSize);

  std::cout << "Generated " << filename << std::endl;
}

int main() {
  int sampleRate = 44100;

  // 1. Generate Engine Sound (Low frequency Sawtooth/Sine mix)
  std::vector<short> engineSamples;
  int engineDuration = sampleRate * 2; // 2 seconds loop
  for (int i = 0; i < engineDuration; ++i) {
    float t = (float)i / sampleRate;
    float freq = 100.0f; // Low rumble
    float val = std::sin(t * freq * 2.0f * 3.14159f);
    // Add some "roughness"
    val += 0.5f * std::sin(t * freq * 0.5f * 2.0f * 3.14159f);
    val += ((float)(rand() % 100) / 100.0f) * 0.2f; // Noise

    if (val > 1.0f)
      val = 1.0f;
    if (val < -1.0f)
      val = -1.0f;

    engineSamples.push_back(static_cast<short>(val * 10000));
  }
  writeWav("assets/audio/engine.wav", engineSamples, sampleRate);

  // 2. Generate Crash Sound (White Noise + Decay)
  std::vector<short> crashSamples;
  int crashDuration = sampleRate * 1; // 1 second
  for (int i = 0; i < crashDuration; ++i) {
    float t = (float)i / crashDuration; // 0 to 1
    float envelope = 1.0f - t;          // Linear decay

    float val = ((float)(rand() % 2000) / 1000.0f) - 1.0f; // -1 to 1 noise

    crashSamples.push_back(static_cast<short>(val * envelope * 20000));
  }
  writeWav("assets/audio/crash.wav", crashSamples, sampleRate);

  return 0;
}
