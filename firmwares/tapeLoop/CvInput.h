#ifndef CvInput_h
#define CvInput_h

// CV jack input or potentiometer

#define EPSILON 0.01
#define MAX_ANALOG_IN 1023.0 // 10 bits

class CvInput {

private:
  unsigned int _pin;

  float _minValue = 0.0;
  float _maxValue = 1.0;
  float _range    = 1.0;

public:
  float value;
  bool  hasChanged;

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
  void setup (unsigned int pin, float min, float max) {
    _pin      = pin;
    _minValue = min;
    _maxValue = max;
    _range    = max - min;
    value     = (float) analogRead(_pin) / MAX_ANALOG_IN * _range + _minValue;
  }

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
  bool update () {
    hasChanged = false;

    long r = analogRead(_pin);
    float v = (float) r / MAX_ANALOG_IN * _range + _minValue;

    // TODO: Implement low-pass filter to smooth value change and remove noise

    if (abs(v - value) < EPSILON) return false;

    value = v;
    hasChanged = true;

    return true;
  }
};

#endif // CvInput_h
