/*
    NewSevenSegment
    Author: 邱柏宇
    email: poyu39.tw@gmail.com
*/
#include <stdio.h>
#include "NUC100Series.h"
#include "GPIO.h"
#include "SYS.h"
#include "NewSevenSegment.h"

#define SEG_NONE 0xFF
#define SEG_N0 0x82
#define SEG_N1 0xEE
#define SEG_N2 0x07
#define SEG_N3 0x46
#define SEG_N4 0x6A
#define SEG_N5 0x52
#define SEG_N6 0x12
#define SEG_N7 0xE6
#define SEG_N8 0x02
#define SEG_N9 0x62
#define SEG_N10 0x22
#define SEG_N11 0x1A
#define SEG_N12 0x93
#define SEG_N13 0x0E
#define SEG_N14 0x13
#define SEG_N15 0x33

uint8_t seg_map[17] = {SEG_NONE, SEG_N0, SEG_N1, SEG_N2, SEG_N3, SEG_N4, SEG_N5, SEG_N6, SEG_N7, SEG_N8, SEG_N9, SEG_N10, SEG_N11, SEG_N12, SEG_N13, SEG_N14, SEG_N15};

// save seg buffer (3~0)
int8_t seg_buffer[4];

// use SEG_LOOP to control seg loop
uint8_t SEG_LOOP = 1;

volatile uint8_t seg_index = 0;

/**
  * @brief                          set one segment display
  *
  * @param[in]  no                  the segment index (3~0)
  * @param[in]  sn                  the number or letter to display: -1 for none, 0-9 for numbers, 10-15 for letters A-F
  *
  */
void show_one_seg(uint8_t no, int8_t sn) {
    PE->DOUT = seg_map[sn + 1];
    GPIO_PIN_DATA(2, no + 4) = 1;
}

// close all seg
void close_seg(void) {
    PC4 = 0; PC5 = 0; PC6 = 0; PC7 = 0;
}

// set seg buffer number
void set_seg_buffer_number(uint16_t number, uint8_t fill_zero) {
    static uint16_t last_number;
    if (number == last_number) return;
    seg_buffer[3] = number / 1000;
    seg_buffer[2] = (number / 100) % 10;
    seg_buffer[1] = (number / 10) % 10;
    seg_buffer[0] = number % 10;
    if (fill_zero == FALSE) {
        if (seg_buffer[3] == 0) seg_buffer[3] = -1;
        if (seg_buffer[2] == 0 && seg_buffer[3] == -1) seg_buffer[2] = -1;
        if (seg_buffer[1] == 0 && seg_buffer[2] == -1) seg_buffer[1] = -1;
    }
    last_number = number;
}

// clear seg buffer
void clear_seg_buffer(void) {
    seg_buffer[3] = -1;
    seg_buffer[2] = -1;
    seg_buffer[1] = -1;
    seg_buffer[0] = -1;
}

void TMR0_IRQHandler(void) {
    TIMER_ClearIntFlag(TIMER0);
    if (!SEG_LOOP) return;
    close_seg();
    show_one_seg(seg_index, seg_buffer[seg_index]);
    seg_index = (seg_index + 1) % 4;
}

void init_timer0(uint8_t timer_hz) {
    TIMER_Open(TIMER0, TIMER_PERIODIC_MODE, timer_hz);
    TIMER_EnableInt(TIMER0);
    NVIC_EnableIRQ(TMR0_IRQn);
    TIMER_Start(TIMER0);
}

/**
  * @brief                          intialize seven segment display
  *
  * @param[in]  use_timer           enable timer for seg loop control
  * @param[in]  timer_hz            timer hz
  *
  * @note                           timer0 will be used to control seg loop if use_timer is 1
  */
void init_seg(uint8_t use_timer, uint8_t timer_hz) {
    GPIO_SetMode(PC, (BIT4 | BIT5 | BIT6 | BIT7), GPIO_PMD_OUTPUT);
    PC4 = 0; PC5 = 0; PC6 = 0; PC7 = 0;
    GPIO_SetMode(PE, (BIT0 | BIT1 | BIT2 | BIT3 | BIT4 | BIT5 | BIT6 | BIT7), GPIO_PMD_QUASI);
    PE0 = 0; PE1 = 0; PE2 = 0; PE3 = 0; PE4 = 0; PE5 = 0; PE6 = 0; PE7 = 0;
    if (use_timer) init_timer0(timer_hz);
    clear_seg_buffer();
}
