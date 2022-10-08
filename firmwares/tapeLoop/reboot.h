#ifndef reboot_h
#define reboot_h

#include "Arduino.h"

// REBOOT CODES
#define CPU_RESTART_ADDR (uint32_t *)0xE000ED0C
#define CPU_RESTART_VAL 0x5FA0004

void reboot() {
  (*(volatile uint32_t *)CPU_RESTART_ADDR) = CPU_RESTART_VAL;
}

#endif // reboot_h
