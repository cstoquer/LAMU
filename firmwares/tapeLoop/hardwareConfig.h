#ifndef hardwareConfig_h
#define hardwareConfig_h


/*▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
█                                                        █
█            TEENSY 3.2  - connection diagram            █
█            ================================            █
█                                                        █
█                         ┏━━━┓                          █
█                    ┏━━━━┫   ┣━━━━━┓                    █
█               GND──┨GND ┖ ─ ┚  VIN┠──5V                █
█                   ─┨0 (RX)    AGND┠──GND               █
█                   ─┨1 (TX)    3.3V┠─                   █
█                   ─┨2           A9┠──potentiometer X   █
█                   ─┨3           A8┠──CV input X        █
█                   ─┨4           A7┠──potentiometer Y   █
█   pin B {encoder}──┨5           A6┠──CV input Y        █
█   pin A {encoder}──┨6       (19)A5┠──{OLED} SCLK       █
█    MOSI {SD card}──┨7       (18)A4┠──{OLED} SDA        █
█       left switch──┨8           A3┠─                   █
█      right switch──┨9           A2┠─                   █
█      SS {SD card}──┨10          A1┠─                   █
█        CV input T──┨11          A0┠──{SD card} SCLK    █
█    MISO {SD card}──┨12          13┠─                   █
█                    ┗━━━━━━━━━━━━━━┛                    █
█                                                        █
█▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄*/



// Interface
#define PIN_ENCODER_A    6
#define PIN_ENCODER_B    5
#define PIN_BUTTON_LEFT  8
#define PIN_BUTTON_RIGHT 9


// Inputs
#define PIN_CV_T         11
#define PIN_CV_X         A8
#define PIN_CV_Y         A6
#define PIN_POT_X        A9
#define PIN_POT_Y        A7


// Display
#define PIN_OLED_SDA     18
#define PIN_OLED_SCLK    19
#define PIN_OLED_RESET   -1


// SD card
#define PIN_SD_MISO      12
#define PIN_SD_MOSI      7
#define PIN_SD_SCK       14
#define PIN_SD_SS        10



#endif // hardwareConfig_h