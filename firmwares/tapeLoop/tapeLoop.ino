#include <SPI.h>
#include <SD.h>
#include <Wire.h>
#include <Audio.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "AudioEngine.h"
#include "Scroller.h"
#include "DirectoryManager.h"
#include "CvInput.h"
#include "ScreenSaver.h"
#include "Bounce2.h"
#include "hardwareConfig.h"
#include "reboot.h"
#include "debug.h"


//▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
// Display
#if (SSD1306_LCDHEIGHT != 64)
  #error("Height incorrect, please fix Adafruit_SSD1306.h");
#endif

//▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
Adafruit_SSD1306 display(PIN_OLED_RESET);
Scroller         scroller;
DirectoryManager directoryManager;
AudioEngine      audioEngine;
ScreenSaver      screenSaver;
Bounce           button1Debouncer;
Bounce           button2Debouncer;
CvInput          faderX;
CvInput          faderY;
CvInput          inputX;
CvInput          inputY;

//▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
void initializeSD () {
  // SD CARD SETTINGS FOR AUDIO SHIELD
  SPI.setMOSI(PIN_SD_MOSI);
  SPI.setSCK(PIN_SD_SCK);
  LOG("SD initialized");
}

bool hasSD = false;

bool loadSD () {
  hasSD = false;

  display.clearDisplay();
  display.setCursor(7, 28);
  display.print("looking for SD card");
  display.display();

  int retry = 0;
  while (!SD.begin(SS)) {
    if (++retry > 5) {
      display.clearDisplay();
      display.setCursor(43, 28);
      display.print("NO DATA");
      display.display();
      return false;
    }

    display.setCursor(42 + retry * 6, 40);
    display.print(".");
    display.display();

    LOG("Cannot find SD. Retrying");
    delay(200 * retry);
  }

  hasSD = true;
  directoryManager.openRootDirectory();
  scroller.setBounds(0, directoryManager.totalItems - 1, 0);
  return true;
}

//▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
void initializeDisplay () {
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.clearDisplay();
}

void helloScreen () {
  display.clearDisplay();
  display.setCursor(16, 28);
  display.print("LAMU - tape v1.0");
  display.display();
}

//▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
float rootNote = 0.0;

void updateRootNote (float value) {
  // TODO: scale value (in range [0..1]) to 1 octave, and quantize
  unsigned int note = (unsigned int)(value * 12.0);
  rootNote = (float)(note) / 12.0;
}

//▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
// A separate variable for tracking reset CV only
volatile bool requestRetrigger = false;
void onTriggerCV () {
  requestRetrigger = true;
}

//▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
void setup () {
  #ifdef DEBUG
    while(!Serial) delay(10);
  #endif

  // Initializations
  initializeDisplay();
  initializeSD();

  // Audio setup
  audioEngine.initializeAudioEngine();

  // Interface
  pinMode(PIN_BUTTON_LEFT,  OUTPUT);
  pinMode(PIN_BUTTON_RIGHT, OUTPUT);
  uint16_t bounceInterval = 5;
	button1Debouncer.attach(PIN_BUTTON_LEFT);
  button2Debouncer.attach(PIN_BUTTON_RIGHT);
	button1Debouncer.interval(bounceInterval);
  button2Debouncer.interval(bounceInterval);

  // trigger CV
  pinMode(PIN_CV_T, INPUT);
  attachInterrupt(PIN_CV_T, onTriggerCV, RISING);

  // CV inputs
  faderX.setup(PIN_POT_X, 0.00, 1.00);
  faderY.setup(PIN_POT_Y, 0.10, 1.90);
  inputX.setup(PIN_CV_X,  0.00, 1.00);
  inputY.setup(PIN_CV_Y,  0.00, 1.00);

  // Start
  loadSD();
  if (hasSD) helloScreen();
}

//▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
void loop() {
  // Menu interface (Encoder and 2 buttons)
  button1Debouncer.update();
  button2Debouncer.update();

  if (!hasSD) {
    if (button1Debouncer.rose() || button2Debouncer.rose()) {
      display.clearDisplay();
      display.display();
      if (loadSD()) {
        directoryManager.displayFiles(&display, scroller.value);
        screenSaver.wakeUp();
      }
    }
    return;
  }

  // Left button stop sound
  if (button1Debouncer.rose()) {
    audioEngine.stopPlayback();
  }

  // Right button do select the highlighted file or folder
  if (button2Debouncer.rose()) {
    if (scroller.value >= directoryManager.dirCount) {
      // check if the file is the same, and if so, retrigger
      String filePath = directoryManager.getFilePathAtIndex(scroller.value);
      if (audioEngine.isCurrentlyPlaying(filePath)) {
        requestRetrigger = true;
      } else {
        audioEngine.playFile(filePath);
      }
    } else {
      directoryManager.openDirectoryAtIndex(scroller.value);
      scroller.setBounds(0, directoryManager.totalItems - 1, directoryManager.getPosition());
      directoryManager.displayFiles(&display, scroller.value);
      screenSaver.wakeUp();
    }
  } else if (scroller.update()) {
    directoryManager.displayFiles(&display, scroller.value);
    screenSaver.wakeUp();
  }

  // Screen saver
  screenSaver.update(&display);

  // Restart / start position
  if (requestRetrigger) {
    requestRetrigger = false;
    faderX.update();
    inputX.update();
    inputY.update();
    audioEngine.skipToPosition(faderX.value + inputX.value);
  }

  // Pitch
  if (inputY.hasChanged) updateRootNote(inputY.value);
  if (faderY.update() || inputY.hasChanged) {
    audioEngine.setPlaybackSpeed(rootNote + faderY.value);
  }
}
