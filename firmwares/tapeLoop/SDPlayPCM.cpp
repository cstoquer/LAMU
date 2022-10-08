/* Audio Library for Teensy 3.X
 * Copyright (c) 2014, Paul Stoffregen, paul@pjrc.com
 *
 * Development of this audio library was funded by PJRC.COM, LLC by sales of
 * Teensy and Audio Adaptor boards.  Please support PJRC's efforts to develop
 * open source software by purchasing Teensy or other PJRC products.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice, development funding notice, and this permission
 * notice shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "SDPlayPCM.h"

#include "spi_interrupt.h"

#define D(x) x
#define B(x) x


void SDPlayPCM::begin(void) {
//	AudioStartUsingSPI();
	playing = false;
	finished = false;
	errors = 0;
	// Set a dummy filename so it doesn't match anything at startup.
	filename = "--------";
}

void SDPlayPCM::loopPlayback(bool loop) {
	__disable_irq();
	looping = loop;
	__enable_irq();
}

bool SDPlayPCM::changeFileTo(AudioFileInfo* info, bool closeFirst) {
	filename = info->name;
//	AudioStopUsingSPI();
	__disable_irq();
	if (closeFirst) {
		rawfile.close();
	}
	rawfile = SD.open(info->name.c_str());

	dataSize = info->size;
	bytesLeftInFile = info->size;
	dataOffset = info->dataOffset;
	channels = info->getChannels();
	bytesPerSample = info->getBytesPerSample();
	sampleRateSpeed = (float) info->getSampleRate() / 44100.0;
	bytesAvailable = 0;
	readPositionInBytes = 0;
	bufferFillPosition = 0;
	finished = false;
	readError = false;
	__enable_irq();

	return true;
}

bool SDPlayPCM::skipTo(uint32_t dataPosition) {
	if(!rawfile) return false;
	uint32_t pos = dataOffset + dataPosition;
	AudioStopUsingSPI();
	__disable_irq();
	boolean didseek = rawfile.seek(pos);
	bytesLeftInFile = dataSize - dataPosition;
	readPositionInBytes = 0;
	bytesAvailable = 0;
	bufferFillPosition = 0;
	readError = false;
	playing = true;
	finished = false;
	__enable_irq();
	AudioStartUsingSPI();
	if(!didseek) {
		return false;
	}
	return true;
}

bool SDPlayPCM::playFrom(AudioFileInfo* info) {
	uint32_t pos = info->dataOffset + info->startPlayFrom;

	// We use the same file, just seek inside it.
	if (info->name.compareTo(filename) == 0) {
		updateRequired = false;
		return skipTo(info->startPlayFrom);
	}

	if (!changeFileTo(info, true)) {
		updateRequired = false;
		return false;
	}

	skipTo(info->startPlayFrom);

	updateRequired = false;
	return true;
}

void SDPlayPCM::restart() {
	__disable_irq();
	rawfile.seek(dataOffset);
	bytesLeftInFile = dataSize;
	playing = true;
	finished = false;
	__enable_irq();
}

void SDPlayPCM::stop(void) {
	__disable_irq();

	if (playing) {
		playing = false;
		__enable_irq();
//		AudioStopUsingSPI();
	} else {
		__enable_irq();
	}
}

void SDPlayPCM::update(void) {

	uint16_t n = AUDIO_BLOCK_SAMPLES;
	audio_block_t *block;
	uint16_t i = 0;

	if(errors > 100) return;

	// only update if we're playing
	if (!playing)
		return;

	if(!looping) {
		if(finished || bytesLeftInFile == 0) {
			playing = false;
			finished = true;
			return;
		}
	}

	// allocate the audio blocks to transmit
	block = allocate();
	if (block == NULL)
		return;

	inUpdate = true;
	read = 0;
	speed = playbackSpeed * sampleRateSpeed;
	bytesRequired = ceil(AUDIO_BLOCK_SAMPLES * speed) * bytesPerSample * channels;

	if (bytesRequired > AUDIOBUFSIZE) {
		bytesRequired = AUDIOBUFSIZE;
	}

	if (bytesAvailable < bytesRequired) {

		bool bufferReadOk = false;
		if (bytesRequired
				< AUDIO_BLOCK_SAMPLES * bytesPerSample * channels) {
			bufferReadOk = fillBuffer(AUDIO_BLOCK_SAMPLES * bytesPerSample * channels);
		} else {
			bufferReadOk = fillBuffer(bytesRequired);
		}

		if (read >= 0 && read < bytesRequired) {
			n = min(AUDIO_BLOCK_SAMPLES,
					floor(bytesAvailable / (bytesPerSample * channels))
							/ speed);
		} else if (readError || !bufferReadOk) {
			errors++;
			readError = false;
			// Fill block with zero.
			for (i = 0; i < AUDIO_BLOCK_SAMPLES; i++) {
				block->data[i] = 0;
			}
			transmit(block);
			release(block);
			updates++;
			return;
		}

	}

	int16_t* out = block->data;

	l0 = 0;
	lowSamplePos = 0;
	float sp = speed * channels;

	// Check for buffer wrapping here so we don't do it 128 times in the loop.
	boolean bufferWrap = ((n*sp) * bytesPerSample) + readPositionInBytes >= AUDIOBUFSIZE;
	if (bytesPerSample == 2) {
		// 16 bit copy
		if(bufferWrap) {
			for (i = 0; i < n; i++) {
				lowSamplePos = (i * speed);
				l0 = readPositionInBytes + (lowSamplePos << channels);
				if (l0 >= AUDIOBUFSIZE) l0 -= AUDIOBUFSIZE;
				memcpy(out++, &audioBuffer[l0],2);
			}
		} else {
			// We won't wrap the buffer edge so don't check
			for (i = 0; i < n; i++) {
				lowSamplePos = (i * speed);
				l0 = readPositionInBytes + (lowSamplePos << channels);
				memcpy(out++, &audioBuffer[l0],2);
			}
		}
	} else if (bytesPerSample == 3) {

		if(channels == 1) {
			if(bufferWrap) {
				// 24 bit copy.
				for (i = 0; i < n; i++) {
					lowSamplePos = (i * speed);
					l0 = readPositionInBytes + (lowSamplePos * 3);
					if (l0 >= AUDIOBUFSIZE) l0 -= AUDIOBUFSIZE;
					*out++ = (audioBuffer[l0] | (audioBuffer[++l0] << 8) | (audioBuffer[++l0] << 16)) >> 8;
				}
			} else {
				// 24 bit copy.
				for (i = 0; i < n; i++) {
					lowSamplePos = (i * speed);
					l0 = readPositionInBytes + (lowSamplePos * 3);
					*out++ = (audioBuffer[l0] | (audioBuffer[++l0] << 8) | (audioBuffer[++l0] << 16)) >> 8;
				}
			}
		} else {
			if(bufferWrap) {
				// 24 bit copy.
				for (i = 0; i < n; i++) {
					lowSamplePos = (i * speed);
					l0 = readPositionInBytes + ((lowSamplePos * 3) << 1);
					if (l0 >= AUDIOBUFSIZE) l0 -= AUDIOBUFSIZE;
					*out++ = (audioBuffer[l0] | (audioBuffer[++l0] << 8) | (audioBuffer[++l0] << 16)) >> 8;
				}
			} else {
				// 24 bit copy.
				for (i = 0; i < n; i++) {
					lowSamplePos = (i * speed);
					l0 = readPositionInBytes + ((lowSamplePos * 3) << 1);
					*out++ = (audioBuffer[l0] | (audioBuffer[++l0] << 8) | (audioBuffer[++l0] << 16)) >> 8;
				}
			}

		}
	}

	if (n != AUDIO_BLOCK_SAMPLES) {
		// There wasn't enough data earlier to fill the audio block

		bytesUsed = (n * sp) * bytesPerSample;
		// Fill rest of block with zeros.
		for (i = n; i < AUDIO_BLOCK_SAMPLES; i++) {
			*out++ = 0;
		}
		if (!looping)
			finished = true;
	} else {
		bytesUsed = bytesRequired;
	}

	readPositionInBytes += bytesUsed;
	readPositionInBytes %= AUDIOBUFSIZE;

	bytesAvailable -= bytesUsed;


	transmit(block);
	release(block);
	inUpdate = false;
	updates++;
	if(readError) errors++;
	readError = false;
}

bool SDPlayPCM::fillBuffer(int32_t requiredBytes) {

// How many bytes until the end of the buffer
	spaceLeftInBuffer = AUDIOBUFSIZE - bufferFillPosition;

	read = 0;

	int32_t read2 = 0;

	// Is there enough left to read from the file?
	if (bytesLeftInFile < requiredBytes - bytesAvailable) {
		if (!looping) {
			// we're not looping and the file will run out
			// so only attempt to read the rest of the file

			requiredBytes = bytesLeftInFile;
		}
	}

	if (requiredBytes > spaceLeftInBuffer) {
		// If we read all the data we need it will go across the buffer boundary

		if (spaceLeftInBuffer <= bytesLeftInFile) {
			// Fill to end of buffer
			read = rawfile.read(&(audioBuffer[bufferFillPosition]),
					spaceLeftInBuffer);
			if (read == -1 && !readError) {
				readError = true;
				goto endfill;
			}
			bufferFillPosition = 0;

			if (requiredBytes <= bytesLeftInFile) {
				// Not enough buffer space, but enough left in file. No seek required
				// Fill buffer from start
				if (requiredBytes - read > 0) {
					read2 = rawfile.read(audioBuffer, requiredBytes - read);
					if (read2 == -1 && !readError) {
						readError = true;
						goto endfill;
					}
					read += read2;
					bufferFillPosition = read2;
				} else {
					bufferFillPosition = 0;
				}
				bytesLeftInFile -= read;

			} else {
				// There is enough data in the file to fill the end of the buffer
				// BUT NOT enough in the file to get all the data we need. Seek is required

				// We've already filled to the end of the buffer

				// now fill from start of buffer with what's left in the file
				if (bytesLeftInFile - read > 0) {
					read2 = rawfile.read(audioBuffer, bytesLeftInFile - read);
					if (read2 == -1 && !readError) {
						readError = true;
						goto endfill;
					}
					read += read2;
					bufferFillPosition = read2;
				}

				// Seek to start of file
				rawfile.seek(dataOffset);

				// get the bit thats left
				int32_t lastBit = requiredBytes - read;

				int32_t read3 = rawfile.read(&(audioBuffer[bufferFillPosition]),
						lastBit);
				if (read3 == -1 && !readError) {
					readError = true;
					goto endfill;
				}


				read += read3;
				bytesLeftInFile = dataSize - lastBit;
				bufferFillPosition += lastBit;
			}
		} else {
			// Buffer has enough space to read the rest of the file.

			// Get the rest from the file
			read = rawfile.read(&(audioBuffer[bufferFillPosition]),
					bytesLeftInFile);
			if (read == -1 && !readError) {
				readError = true;
				goto endfill;
			}
			// go to start of file
			rawfile.seek(dataOffset);

			spaceLeftInBuffer -= read;

			// read to end of buffer
			read2 = rawfile.read(&(audioBuffer[bufferFillPosition + read]),
					spaceLeftInBuffer);
			if (read2 == -1 && !readError) {
				readError = true;
				goto endfill;
			}

			read += read2;

			// Fill buffer from start with amount that is left
			int32_t read3 = rawfile.read(audioBuffer, requiredBytes - read);
			if (read3 == -1 && !readError) {
				readError = true;
				goto endfill;
			}

			read += read3;
			bytesLeftInFile = dataSize - (read2 + read3);
			bufferFillPosition = read3;
		}
	} else {

		// There's enough space in the buffer, is there enough in the file
		if (bytesLeftInFile >= requiredBytes) {
//			Serial.println("Fill style 4.");
			// Enough in file, just read it
			read = rawfile.read(&(audioBuffer[bufferFillPosition]), requiredBytes);
			if (read == -1 && !readError) {
				readError = true;
				goto endfill;
			} else {
				bytesLeftInFile -= read;
				bufferFillPosition += read;
			}
		} else {

			// Not enough in file
			read = rawfile.read(&(audioBuffer[bufferFillPosition]),
					bytesLeftInFile);
			if (read == -1 && !readError) {
				readError = true;
				goto endfill;
			}

			if (looping) {
				// If we're looping, seek back to start and fill from there
				rawfile.seek(dataOffset);

				read2 = rawfile.read(&(audioBuffer[bufferFillPosition + read]),
						requiredBytes - bytesLeftInFile);
				if (read2 == -1) {
					readError = true;
					goto endfill;
				}

				read += read2;
				bytesLeftInFile = dataSize - read2;
				bufferFillPosition += read;
			} else {
				// otherwise flag playback finished
				finished = true;
			}

		}
	}

	endfill:

	if(read == -1 || readError) {
		return false;
	} else {
		bytesAvailable += read;
		bufferFillPosition %= AUDIOBUFSIZE;
		bufferFills++;
		if(bytesLeftInFile == 0) finished = true;
		return true;
	}
}

// Progress in file scaled from 0 to 1
float SDPlayPCM::offset(void) {
	// For now fudge it a bit and shift it forward in time by 2 blocks.
	uint32_t bytes = bytesLeftInFile <= (bytesRequired * 2) ? bytesLeftInFile : bytesLeftInFile - (bytesRequired * 2);
	float off = (float)(dataSize - bytes) / dataSize;
	return off;
}

// Note rawfile.available() is clamped to 16 bit signed int
// not sure why the SD library does that.
// Just do the same as SD does but don't clamp the result.
uint32_t SDPlayPCM::fileAvailable() {
	return rawfile.size() - rawfile.position();
}
