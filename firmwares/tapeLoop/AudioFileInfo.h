#ifndef AudioFileInfo_h
#define AudioFileInfo_h

#include "Arduino.h"

#define MAX_CUE_POINTS 64

static uint32_t SAMPLE_RATES[5] = { 11025, 22050, 44100, 48000, 96000 };


class AudioFileInfo {
public:
	String name;

	// Size doesn't include the header for wav files, just the length of the audio
	uint32_t size;
  uint16_t channels;
  uint32_t sampleRate;
  uint8_t  bytesPerSample;

	// For wav files where the audio data starts
	uint32_t dataOffset = 0;

	// Store where playback ended if we want to resume
	uint32_t startPlayFrom = 0;

	// Cue points
	uint8_t  nCuePoints = 0;
	uint32_t cuePoints[MAX_CUE_POINTS];

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
	void reset () {
		name           = "";
		size           = 0;
		channels       = 1;
		sampleRate     = 44100;
		bytesPerSample = 2;
		dataOffset     = 0;
		startPlayFrom  = 0;
		nCuePoints     = 0;
	}

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
	void setChannels (uint8_t value) {
    channels = value;
	}

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
	uint16_t getChannels () {
		return channels;
	}

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
	// Return false is sample rate not supported
	bool setSampleRate (uint32_t value) {
    sampleRate = value;
    for (int i = 0; i < 5; ++i) {
      if (SAMPLE_RATES[i] == value) return true;
    }
    sampleRate = 44100;
    return false;
	}

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
	uint32_t getSampleRate () {
		return sampleRate;
	}

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
	void setBitsPerSample (uint8_t value) {
    bytesPerSample = value >> 3;
	}

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
	uint8_t getBytesPerSample() {
		return bytesPerSample;
	}

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
	void resetCuePoints () {
		nCuePoints = 0;
	}

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
	void setStartPosition (float position) {
		// wrap position in range [0..1[
		while (position >= 1.0) position -= 1.0;

	  // use cue points
		if (nCuePoints > 1) {
      uint32_t cueIndex = position * nCuePoints;
      startPlayFrom = bytesPerSample * cuePoints[cueIndex];
			return;
		}

		position *= size / bytesPerSample;
    startPlayFrom = bytesPerSample * (uint32_t) position;
	}

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
	void addCuePoint (uint32_t cuePoint) {
		if (nCuePoints >= MAX_CUE_POINTS) return;
		cuePoints[nCuePoints++] = cuePoint;
	}

};

#endif // AudioFileInfo_h
