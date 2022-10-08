#ifndef AudioEngine_h
#define AudioEngine_h

#include "Arduino.h"
#include <SPI.h>
#include <SD.h>
#include <Audio.h>

#include "AudioFileInfo.h"
#include "WavHeaderReader.h"
#include "SDPlayPCM.h"


#define MAX_SPEED 4.489

class AudioEngine {

private:
  bool              _isStopped;
  SDPlayPCM         _player;
  AudioOutputAnalog _dac;
  AudioConnection   _connection;
  WavHeaderReader   _wavHeaderReader;
  AudioFileInfo     _fileInfo;

public:

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
  AudioEngine() : _connection(_player, 0, _dac, 0) {
    // nop
  }

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
  void initializeAudioEngine () {
    AudioMemory(25);
    _player.loopPlayback(true);
    _isStopped = true;
  }

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
  bool isCurrentlyPlaying (String filePath) {
    if (_isStopped) return false;
    return _fileInfo.name.compareTo(filePath) == 0;
  }

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
  void playFile (String filePath) {
    AudioNoInterrupts();
    if (!_isStopped) {
      _isStopped = true;
      _player.stop();
    }

    // Reset play state
    _fileInfo.name          = filePath;
    _fileInfo.startPlayFrom = 0;

    // Read WAV data
    File wavFile = SD.open(filePath.c_str());
    if (!_wavHeaderReader.readWavHeader(&wavFile, &_fileInfo)) {
      _fileInfo.reset();
    };
    wavFile.close();

    // Start playback
    if (_fileInfo.size > 0) {
      _player.playFrom(&_fileInfo);
      _isStopped = false;
    }
    AudioInterrupts();
  }

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
  void stopPlayback () {
    if (_isStopped) return;
    AudioNoInterrupts();
    _isStopped = true;
    _player.stop();
    AudioInterrupts();
  }

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
  void setPlaybackSpeed (float speed) {
    if (_isStopped) return;
    if (speed > MAX_SPEED) speed = MAX_SPEED;
    _player.playbackSpeed = speed;
  }

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
  void skipToPosition (float position) {
    _fileInfo.setStartPosition(position);
    if (_isStopped) return;
    _player.skipTo(_fileInfo.startPlayFrom);
  }

};

#endif // AudioEngine_h
