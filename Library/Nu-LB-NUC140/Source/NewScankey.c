/*
    NewScankey
    Author: 邱柏宇
    email: poyu39.tw@gmail.com
*/
#include <stdio.h>
#include <NUC100Series.h>
#include "GPIO.h"
#include "SYS_init.h"

volatile uint8_t KEY_FLAG = 0;
volatile uint8_t current_key = 0;
volatile uint8_t key_held = 0;
volatile uint8_t delay = 0;
volatile uint8_t repeat_i = 0;
volatile uint16_t key_press_ticks = 0;

void TMR1_IRQHandler(void) {
    TIMER_ClearIntFlag(TIMER1);
    
    if (key_held) {
        key_press_ticks = (key_press_ticks + 1) % UINT16_MAX;
        
        if (key_press_ticks == delay) {
            // one time
            KEY_FLAG = current_key;
        } else if (key_press_ticks > delay && \
            (key_press_ticks % repeat_i == 0)) {
            // repeat
            KEY_FLAG = current_key;
        }
    }
}

void init_timer1(void) {
    TIMER_Open(TIMER1, TIMER_PERIODIC_MODE, 100); // 10ms
    TIMER_EnableInt(TIMER1);
    NVIC_EnableIRQ(TMR1_IRQn);
    TIMER_Start(TIMER1);
}

/*
        PA2   PA1   PA0
    PA3   1     2     3
    PA4   4     5     6
    PA5   7     8     9
*/
void GPAB_IRQHandler(void) {
    uint8_t i, which_PA_INT = 0xFF;
    uint32_t src = PA->ISRC;
    
    if (src & BIT0) which_PA_INT = 0;
    if (src & BIT1) which_PA_INT = 1;
    if (src & BIT2) which_PA_INT = 2;
    
    if (which_PA_INT == 0xFF) {
        PA->ISRC = src;
        return;
    }
    
    // scan key
    PA0 = PA1 = PA2 = PA3 = PA4 = PA5 = 1;
    for (i = 3; i <= 5; i++) {
        GPIO_PIN_DATA(0, i) = 0;
        if (GPIO_PIN_DATA(0, which_PA_INT) == 0) {
            current_key = (3 - which_PA_INT) + 3 * (i - 3);
            break;
        }
        GPIO_PIN_DATA(0, i) = 1;
    }
    
    PA0 = PA1 = PA2 = 1; PA3 = PA4 = PA5 = 0;
    
    if (GPIO_PIN_DATA(0, which_PA_INT) == 0) {
        // key pressed
        KEY_FLAG = current_key;
        key_held = 1;
        key_press_ticks = 0;
    } else {
        // key released
        key_held = 0;
        key_press_ticks = 0;
    }
    
    PA->ISRC = src;
}

// init keypad interrupt
void init_keypad_INT(uint8_t long_press_delay, uint8_t long_press_repeat_interval) {
    GPIO_SetMode(PA, (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5), GPIO_MODE_QUASI);
    GPIO_EnableInt(PA, 0, GPIO_INT_BOTH_EDGE);
    GPIO_EnableInt(PA, 1, GPIO_INT_BOTH_EDGE);
    GPIO_EnableInt(PA, 2, GPIO_INT_BOTH_EDGE);
    NVIC_EnableIRQ(GPAB_IRQn);
    NVIC_SetPriority(GPAB_IRQn, 3);
    GPIO_SET_DEBOUNCE_TIME(GPIO_DBCLKSRC_LIRC, GPIO_DBCLKSEL_128);
    GPIO_ENABLE_DEBOUNCE(PA, (BIT0 | BIT1 | BIT2));
    PA0 = PA1 = PA2 = 1; PA3 = PA4 = PA5 = 0;
    delay = long_press_delay;
    repeat_i = long_press_repeat_interval;
    init_timer1();
}
