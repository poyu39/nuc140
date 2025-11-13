/*
    NewBuzzer
    Author: 邱柏宇
    email: poyu39.tw@gmail.com
*/
#include <stdio.h>
#include "NUC100Series.h"
#include "SYS_init.h"
#include "GPIO.h"
#include "SYS.h"
#include "NewBuzzer.h"

volatile uint16_t play_time = 0;
volatile uint8_t pwm_state = 0;
volatile int16_t song_index = -1;
volatile uint8_t song_len = 0;
volatile uint64_t delay_counter = 0;
volatile uint16_t note_delay = 0;

volatile uint16_t *song_freq;
volatile uint16_t *song_t;

void TMR1_IRQHandler(void) {
    if (play_time > 0) {
        if (!pwm_state) {
            PWM_EnableOutput(PWM1, PWM_CH_0_MASK);
            pwm_state = TRUE;
        }
        play_time--;
        if (play_time == 0) {
            delay_counter = note_delay / 5;
            PWM_DisableOutput(PWM1, PWM_CH_0_MASK);
        }
    } else {
        if (delay_counter > 0) {
            delay_counter--;
        } else {
            if (song_index >= 0) {
                buzzer_play(song_freq[song_index], song_t[song_index]);
                song_index++;
                if (song_index >= song_len) {
                    song_index = -1;
                    song_len = 0;
                }
            }
        }
    }
    TIMER_ClearIntFlag(TIMER1);
}

void init_timer1(void) {
    TIMER_Open(TIMER1, TIMER_PERIODIC_MODE, 200); // 5ms
    TIMER_EnableInt(TIMER1);
    NVIC_EnableIRQ(TMR1_IRQn);
    TIMER_Start(TIMER1);
}

void init_buzzer(void) {
    init_timer1();
    play_time = 0;
}

void buzzer_play(uint16_t freq, uint16_t t) {
    PWM_ConfigOutputChannel(PWM1, PWM_CH0, freq, 95);
    PWM_EnableOutput(PWM1, PWM_CH_0_MASK);
    play_time = t / 5;
}

void buzzer_play_song(uint16_t *freq, uint16_t *t, uint16_t nd, uint8_t size) {
    if (song_len != 0) return;
    song_len = size;
    song_index = 0;
    song_freq = freq;
    note_delay = nd;
    song_t = t;
    buzzer_play(song_freq[song_index], song_t[song_index]);
}

void buzzer_stop(void) {
    play_time = 0;
    PB11 = 1;
}

uint8_t buzzer_is_playing(void) {
    return play_time > 0;
}
