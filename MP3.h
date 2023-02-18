#ifndef MP3_H__
#define MP3_H__
#include <Audio.h>
#include <SD.h>
#include <FS.h>
// I2S Connections
#define I2S_DOUT 26
#define I2S_LRC 25
#define I2S_BCLK 22


#define mp3Serial Serial2

enum FileType {
  INVALID,
  NORMAL,
  REDIRECT,
  TERMINATE,
  DONT_EXIST
};

class AudioFile {
  String name;
  uint32_t duration;
  uint32_t start;
  int action;
  bool playing;
public:
  bool readParams(String filename) {
    if (SD.exists(filename)) {
      this->name = filename;
      File f = SD.open(filename, FILE_READ);
      if (!f) {
        return false;
      }
      this->calculateDuration(f.size());
      f.close();
      return true;
    }
    return false;
  }
  void play(int action) {
    this->playing = true;
    this->action = action;
    this->start = millis();
  }

  void stop() {
    this->action = FileType::NORMAL;
  }
  void showParams() {
    Serial.println(String("Name: ") + this->name);
    Serial.println(String("Duration: ") + this->duration);
  }
  void calculateDuration(uint32_t fileSize) {
    float sampleRate = 24000;
    float numChannels = 2;
    float bitsPerSample = 16;
    float duration = fileSize / (sampleRate * numChannels * bitsPerSample / 8);
    this->duration = duration * 1000;
  }
  int completed() {
    if (!this->playing) {
      return FileType::INVALID;
    }
    if (millis() - this->start > this->duration) {
      this->playing = false;
      return this->action;
    }
    return FileType::INVALID;
  }
};
namespace MP3 {

  const int chipSelect = 5;

  Audio audio;
  AudioFile currentFile;

  void play();
  void update();
  void stop();

  bool __initialized = false;
  void initializeAudio();


  bool begin() {
    // initializeAudio();
    __initialized = SD.begin(chipSelect);
    return __initialized;
  }

  void initializeAudio() {
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(audio.maxVolume());
  }



  bool initialized() {
    return __initialized;
  }
//213T
//1221R

  int getFileInfo(String filename) {
    if (SD.exists(String("/") + filename + ".wav")) {
      return FileType::NORMAL;
    }
    if (SD.exists(String("/") + filename + "T.wav")) {
      return FileType::TERMINATE;
    }
    if (SD.exists(String("/") + filename + "R.wav")) {
      return FileType::REDIRECT;
    }
    return FileType::DONT_EXIST;
  }


  bool play(String filename) {
    if (!MP3::initialized()) {
      if (MP3::begin()) {
        Serial.println("MP3 initialized");
      }
    }
    MP3::stop();
    
    Serial.println(filename);
    int action = MP3::getFileInfo(filename); 
    switch(action) {
      case FileType::DONT_EXIST: return false;
      case FileType::NORMAL: break;
      case FileType::REDIRECT: filename += 'R'; break;
      case FileType::TERMINATE: filename += 'T'; break;
    }
    filename = String("/") + filename + ".wav";

    if (currentFile.readParams(filename)) {
      currentFile.showParams();
    }
    bool played = audio.connecttoSD(filename.c_str());
    currentFile.play(action);
    if (played) {
      Serial.println("File palyed successfully");
    } else {
      Serial.println("Unable to play file");
    }
    return played;
  }

  void update() {
    audio.loop();
  }

  void stop() {
    audio.stopSong();
    currentFile.stop();
  }
};
#endif