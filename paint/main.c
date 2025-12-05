/**
 * Author: poyu39
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "NUC100Series.h"
#include "MCU_init.h"
#include "SYS_init.h"
#include "NewSevenSegment.h"
#include "NewLCD.h"
#include "NewScankey.h"
#include "bmp/brush.h"
#include "bmp/eraser.h"

#define DRAW 0
#define ERASE 1

uint8_t mode = 0;
uint8_t bx = 128 / 2;
uint8_t by = 64 / 2;
uint8_t bw = 16;
uint8_t bh = 16;

uint8_t panel[LCD_Xmax * LCD_Ymax / 8] = {0};

volatile uint8_t clear_flag = 0;

void EINT1_IRQHandler(void) {
    GPIO_CLR_INT_FLAG(PB, BIT15);
    clear_flag = 1;
}

void init_ext(void) {
    GPIO_SetMode(PB, BIT15, GPIO_MODE_INPUT);
    GPIO_EnableEINT1(PB, 15, GPIO_INT_FALLING);
    NVIC_EnableIRQ(EINT1_IRQn);
    GPIO_SET_DEBOUNCE_TIME(GPIO_DBCLKSRC_LIRC, GPIO_DBCLKSEL_64);
    GPIO_ENABLE_DEBOUNCE(PB, BIT15);
}

void draw_pixel_in_panel(int16_t x, int16_t y, uint16_t color) {
    if (color == FG_COLOR)
        panel[x + y / 8 * LCD_Xmax] |= (0x01 << (y % 8));
    else if (color == BG_COLOR)
        panel[x + y / 8 * LCD_Xmax] &= ~(0x01 << (y % 8));
}

void show_tool() {
    if (mode == 0) {
        draw_bitmap_in_buffer(brush_bmp_hex, bx - bw / 2, by - bh / 2, 16, 16, FG_COLOR);
    } else {
        draw_bitmap_in_buffer(eraser_bmp_hex, bx - bw / 2, by - bh / 2, 16, 16, FG_COLOR);
    }
}

void clear_panel() {
    int i;
    for (i = 0; i < LCD_Xmax * LCD_Ymax / 8; i++) {
        panel[i] = 0x00;
    }
}

int main(void) {
    SYS_Init();
    init_keypad_INT(30, 1);
    init_seg(TRUE, 200);
    init_lcd(FALSE, SPI3_CLOCK_FREQUENCY);
    init_ext();
    
    PD14 = 0;   // on lcd backlight
    
    printf_s_in_buffer(bx - bw, by + bh / 2 + 3, 5, "Paint!");
    draw_bitmap_in_buffer(brush_bmp_hex, bx - bw / 2, by - bh / 2, 16, 16, FG_COLOR);
    show_lcd_buffer();
    
    while (TRUE) {
        if (clear_flag) {
            clear_panel();
            cover_lcd_buffer(panel);
            show_tool();
            show_lcd_buffer();
            clear_flag = 0;
        }
        
        if (KEY_FLAG == 0) continue;
        
        clear_lcd_buffer();
        
        if (KEY_FLAG >= 1 && KEY_FLAG <= 3) {
            if (by > bh / 2) by -= 1;
            if (KEY_FLAG == 1 && bx > bw / 2) bx -= 1;
            if (KEY_FLAG == 3 && bx < LCD_Xmax - bw / 2) bx += 1;
        }
        if (KEY_FLAG == 4 && bx > bw / 2) bx -= 1;
        if (KEY_FLAG == 6 && bx < LCD_Xmax - bw / 2) bx += 1;
        if (KEY_FLAG >= 7 && KEY_FLAG <= 9) {
            if (by < LCD_Ymax - bh / 2) by += 1;
            if (KEY_FLAG == 7 && bx > bw / 2) bx -= 1;
            if (KEY_FLAG == 9 && bx < LCD_Xmax - bw / 2) bx += 1;
        }
        
        if (KEY_FLAG == 5) mode = (mode == DRAW) ? ERASE : DRAW;
        
        if (mode == DRAW) {
            draw_pixel_in_panel(bx - bw / 2, by + bh / 2 - 1, FG_COLOR);
        }
        if (mode == ERASE) {
            draw_pixel_in_panel(bx - bw / 2, by + bh / 2 - 1, BG_COLOR);
            draw_pixel_in_panel(bx - bw / 2 + 1, by + bh / 2 - 1, BG_COLOR);
            draw_pixel_in_panel(bx - bw / 2 + 2, by + bh / 2 - 1, BG_COLOR);
            draw_pixel_in_panel(bx - bw / 2 + 3, by + bh / 2 - 1, BG_COLOR);
            draw_pixel_in_panel(bx - bw / 2 + 4, by + bh / 2 - 1, BG_COLOR);
            draw_pixel_in_panel(bx - bw / 2 + 5, by + bh / 2 - 1, BG_COLOR);
            draw_pixel_in_panel(bx - bw / 2 + 6, by + bh / 2 - 1, BG_COLOR);
            draw_pixel_in_panel(bx - bw / 2 + 7, by + bh / 2 - 1, BG_COLOR);
            draw_pixel_in_panel(bx - bw / 2 + 8, by + bh / 2 - 1, BG_COLOR);
        }
        cover_lcd_buffer(panel);
        show_tool();
        show_lcd_buffer();
        
        
        set_seg_buffer_number(bx * 100 + by, TRUE);
        
        KEY_FLAG = 0;
    }
}

