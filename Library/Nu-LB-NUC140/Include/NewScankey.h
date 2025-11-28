#ifndef SCANKEY_H
#define SCANKEY_H

extern volatile uint8_t KEY_FLAG;
extern volatile uint8_t IS_PRESSED;

extern void init_keypad_INT(uint8_t long_press_delay, uint8_t long_press_repeat_interval);

#endif
