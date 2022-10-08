#ifndef ScreenSaver_h
#define ScreenSaver_h

#include <Adafruit_SSD1306.h>

#define SCREEN_SAVER_TIMEOUT 600000

// Uncomment this for a star field animation on screen saver. It might slow down everything.

// #define STARFIELD_ANIMATION

//▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
#ifdef STARFIELD_ANIMATION
  #define STARS_COUNT 16
  class Star {
  private:
    int _x;
    int _y;
    int _z;
    void _setRandomPosition () {
      _x = random(30) - 15;
      _y = random(16) - 8;
      _z = 20 + random(20);
    }

  public:
    int px;
    int py;
    int size;

    void update () {
      _z -= 1;
      if (_z <= 0) _setRandomPosition();
      px = 64 * _x / _z + 64;
      py = 32 * _y / _z + 32;
      size = 3 - _z / 12;
    }
  };
#endif // STARFIELD_ANIMATION
//▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄


class ScreenSaver {

private:

  unsigned long _counter = 0;

#ifdef STARFIELD_ANIMATION
  Star _stars[STARS_COUNT];
#endif // STARFIELD_ANIMATION


public:
  bool isOn = false;

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
  void wakeUp () {
    _counter = 0;
    isOn     = false;
  }

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
  void update (Adafruit_SSD1306* display) {
    if (isOn) return;
    if (++_counter < SCREEN_SAVER_TIMEOUT) return;

    isOn = true;
    display->clearDisplay();

    #ifdef STARFIELD_ANIMATION
      for (int i = 0; i < STARS_COUNT; ++i) {
        _stars[i].update();
        int s = _stars[i].size;
        display->fillRect(_stars[i].px, _stars[i].py, s, s, SSD1306_WHITE);
      }
    #endif // STARFIELD_ANIMATION

    display->display();
  }
};

#endif // ScreenSaver_h
