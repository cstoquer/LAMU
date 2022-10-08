#ifndef Scroller_h
#define Scroller_h

#include <Encoder.h>
#include "hardwareConfig.h"

Encoder encoder(PIN_ENCODER_A, PIN_ENCODER_B);

class Scroller {

private:
  long _minValue;
  long _maxValue;

public:
  long value;

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
  void setBounds (long min, long max) {
    setBounds(min, max, 0);
  }

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
  void setBounds (long min, long max, int v) {
    if (v < min) v = min;
    if (v > max) v = max;
    value     = v;
    _minValue = min;
    _maxValue = max;
    encoder.write(v * 4 + 2);
  }

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
  bool update () {
    long current = encoder.read();

    // NOTE: The rotary encoders used generate 4 pulses per "clic".
    // We can still access the sub pulse by slowly moving the encoder between
    // consecutive clics. The easiest solution is to divide the value by 4.
    // (but we still need to manage 4 interupts, which is not very optimal)
    current = current >> 2;

    if (current > _maxValue) {
      encoder.write(_maxValue * 4 + 2);
      return false;
    }

    if (current < _minValue) {
      encoder.write(_minValue * 4 + 2);
      return false;
    }

    if (current == value) {
      return false;
    }

    value = current;

    return true;
  }
};

#endif // Scroller_h
