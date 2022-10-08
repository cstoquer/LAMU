#ifndef debug_h
#define debug_h

  //▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄▄
  //#define DEBUG

  #ifdef DEBUG
    #define LOG(x) Serial.println(x)
  #else
    #define LOG(x)
  #endif // DEBUG

#endif // debug_h
