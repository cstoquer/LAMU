#ifndef WavHeaderReader_h
#define WavHeaderReader_h

#include <Audio.h>
#include "SD.h"
#include "AudioFileInfo.h"
#include "debug.h"


// WAV file format documentation:
// https://sites.google.com/site/musicgapi/technical-documents/wav-file-format


//▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
#define CHUNK_ID_RIFF 0x46464952
#define CHUNK_ID_WAVE 1163280727
#define CHUNK_ID_FTM  544501094
#define CHUNK_ID_DATA 1635017060
#define CHUNK_ID_CUE  0x20657563
#define PCM_AUDIO     1


class WavHeaderReader {
private:

  File* _waveFile;

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
  uint16_t _readShort () {
    uint16_t value = _waveFile->read();
    value = _waveFile->read() << 8 | value;
    return value;
  }

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
  uint32_t _readLong () {
    // Wav files are little endian
    int32_t value = _waveFile->read();
    for (byte i = 8; i < 32; i += 8) {
      value = _waveFile->read() << i | value;
    }
    return value;
  }

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
  bool _skipUntilChunk (uint32_t chunkId) {
    while (_readLong() != chunkId) {
      uint32_t chunkSize = _readLong();
      // Word align chunkSize
      if (chunkSize % 2 == 1) chunkSize += 1;
      _waveFile->seek(_waveFile->position() + chunkSize);
      if (!_waveFile->available()) return false;
    }
    return true;
  }

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
  void _skipNextBytes (uint32_t nBytes) {
    _waveFile->seek(_waveFile->position() + nBytes);
  }


public:

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
  bool readWavHeader (File* file, AudioFileInfo* info) {
    if (!file->available()) return false;
    _waveFile = file;

    if (_readLong() != CHUNK_ID_RIFF) return false;
    uint32_t fileSize = _readLong(); // fileSize
    if (_readLong() != CHUNK_ID_WAVE) return false;
    if (!_skipUntilChunk(CHUNK_ID_FTM)) return false;
    uint32_t ftmChunkSize = _readLong();

    // Audio Format
    if (_readShort() != PCM_AUDIO) return false;

    info->setChannels(_readShort());
    if(!info->setSampleRate(_readLong())) return false;
    _skipNextBytes(6); // byteRate (4 bytes), blockAlign (2 bytes)
    uint16_t bitsPerSample = _readShort();
    if (bitsPerSample % 8 != 0) return false;
    info->setBitsPerSample(bitsPerSample);

    // Extended format chunk
    if (ftmChunkSize > 16) _skipNextBytes(ftmChunkSize - 16);

    // Audio data chunk size
    _skipUntilChunk(CHUNK_ID_DATA);
    uint32_t audioDataChunkSize = _readLong();
    if (ftmChunkSize + audioDataChunkSize > fileSize) return false;
    info->size = audioDataChunkSize;
    info->dataOffset = file->position();

    // Skip audio data chunk
    _skipNextBytes(audioDataChunkSize);

    // TODO: read other types of chunk (i.e. 'smpl')

    // Cue points
    info->resetCuePoints();

    if (_skipUntilChunk(CHUNK_ID_CUE)) {
      _skipNextBytes(4); // chunkSize
      uint32_t cueCount = _readLong();

      // Add a cue point at the very begining of the sample
      if (cueCount > 0) info->addCuePoint(0);

      // Read each cue points in the list
      for (uint32_t i = 0; i < cueCount; ++i) {
        _skipNextBytes(4); // cueId
        uint32_t cueStart = _readLong();
        if (cueStart > 0 && cueStart < audioDataChunkSize) {
          info->addCuePoint(cueStart);
        }
        _skipNextBytes(16); // 'data', chunkStart, blockStart, sampleOffset
      }
    }

    return true;
  }

};

#endif // WavHeaderReader_h
