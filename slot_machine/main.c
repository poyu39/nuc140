/**
 * Author: poyu39
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "NUC100Series.h"
#include "MCU_init.h"
#include "SYS_init.h"
#include "Seven_Segment.h"
#include "Scankey.h"
#include "LCD.h"

#define LED_UPDATE_TICK 1
#define SEVEN_SEG_UPDATE_TICK 5
#define LCD_UPDATE_TICK 10
#define TICK_PER_MS 1000

#define LCD_LINE_SPACE "                "

void rand_target(uint16_t target[4], uint16_t loop_count) {
    int i;
    srand(loop_count);
    for (i = 0; i < 4; i++) {
        target[i] = (rand() % 9) + 1;
    }
}

float get_rate(uint16_t speed) {
    if (speed == 100)   return 16;
    if (speed == 200)   return 8;
    if (speed == 300)   return 4;
    if (speed == 400)   return 2;
    if (speed == 500)   return 1;
    if (speed == 600)   return 0.8;
    if (speed == 700)   return 0.6;
    if (speed == 800)   return 0.4;
    if (speed == 900)   return 0.2;
    if (speed == 1000)  return 0.1;
    return 0;
}

float cal_score(uint16_t target[4], uint16_t selected[4], float score, uint16_t speed) {
    float sam_count = 0;
    int i;
    for (i = 0; i < 4; i++) {
        if (target[i] == selected[i]) sam_count++;
    }
    score += sam_count * get_rate(speed);
    return score;
}

void update_lcd_buffer(char lcd_buffer[4][17], uint16_t target[4], uint16_t speed, float score) {
    float speed_s = speed;
    speed_s = speed_s / 1000;
    sprintf(lcd_buffer[0], "%-12s%4d",     "Target: ",   target[0] * 1000 + target[1] * 100 + target[2] * 10 + target[3]);
    sprintf(lcd_buffer[1], "%-12s%4.1f",   "Speed: ",    speed_s);
    sprintf(lcd_buffer[2], "%-12s%4.1f",   "Rate: ",     get_rate(speed));
    sprintf(lcd_buffer[3], "%-12s%4.1f",   "Score: ",    score);
}

void show_seven_seg(int s_num) {
    static uint8_t sindex = 0;
    uint8_t this_num = 0;
    sindex = (sindex + 1) % 4;
    // get the number of this index
    this_num = s_num / (int)pow(10, sindex) % 10;
    if (this_num == 0) return;
    CloseSevenSegment();
    ShowSevenSegment(sindex, this_num);
}

void show_lcd(uint8_t* lcd_need_update, char lcd_buffer[4][17], char lcd_now[4][17]) {
    // int lcd_x, int lcd_y;
    static uint8_t lcd_x = 0, lcd_y = 0;
    if (lcd_buffer[lcd_y][lcd_x] != lcd_now[lcd_y][lcd_x]) {
        lcd_now[lcd_y][lcd_x] = lcd_buffer[lcd_y][lcd_x];
        printC(lcd_x * 8, lcd_y * 16, lcd_now[lcd_y][lcd_x]);
    }
    if (lcd_y < 4) {
        if (lcd_x < 16) {
            lcd_x++;
        } else {
            lcd_x = 0;
            lcd_y++;
        }
    } else {
        lcd_x = 0;
        lcd_y = 0;
        *lcd_need_update = 0;
    }
}

void led_full(uint16_t* show_led_times) {
    uint8_t i;
    uint8_t is_on = 1;
    if (*show_led_times > 0) {
        is_on = 0;
        (*show_led_times)--;
    } else {
        is_on = 1;
    }
    for (i = 0; i < 4; i++) GPIO_PIN_DATA(2, 12 + i) = is_on;
}

void init_led() {
    uint8_t i;
    for (i = 0; i < 4; i++) {
        GPIO_SetMode(PC, BIT12 + i, GPIO_MODE_OUTPUT);
        GPIO_PIN_DATA(2, 12 + i) = 1;
    }
}

int main(void) {
    uint8_t keyin = 0, is_pressed = 0;
    uint16_t loop_count = 0;
    
    uint16_t random_target[4] = {0, 0, 0, 0};
    uint16_t selected[4] = {0, 0, 0, 0};
    uint16_t target[4] = {0, 0, 0, 0};
    float score = 0;
    uint16_t speed = 500;
    uint8_t selecting_index = 0;
    
    // lcd
    uint8_t lcd_need_update = 0;
    char lcd_buffer[4][17] = {LCD_LINE_SPACE, LCD_LINE_SPACE, LCD_LINE_SPACE, LCD_LINE_SPACE};
    char lcd_now[4][17] = {LCD_LINE_SPACE, LCD_LINE_SPACE, LCD_LINE_SPACE, LCD_LINE_SPACE};
    
    // flag
    uint8_t is_started = 0;
    uint16_t is_led_on = 0;
    
    SYS_Init();
    OpenKeyPad();
    init_LCD();
    init_led();
    clear_LCD();
    
    PD14 = 0;   // on lcd backlight
    
    printS(40, 24, "Start!");
    
    while (TRUE) {
        CLK_SysTickDelay(TICK_PER_MS);
        
        loop_count = (loop_count + 1) % UINT16_MAX;
        
        if (loop_count % speed == 0 && is_started) {
            int i;
            for (i = selecting_index; i < 4; i++) {
                random_target[i] = random_target[i] % 9 + 1;
            }
            is_led_on = 50;
        }
        
        if (loop_count % LED_UPDATE_TICK == 0) led_full(&is_led_on);
        if (loop_count % SEVEN_SEG_UPDATE_TICK == 0) show_seven_seg(random_target[0] * 1000 + random_target[1] * 100 + random_target[2] * 10 + random_target[3]);
        if (loop_count % LCD_UPDATE_TICK == 0 && lcd_need_update) show_lcd(&lcd_need_update, lcd_buffer, lcd_now);
        
        
        keyin = ScanKey();
        
        if (keyin == 0) {
            is_pressed = 0;
            continue;
        }
        
        if (is_pressed) continue;
        is_pressed = 1;
        
        // start
        if (keyin == 4 && is_started == 0) {
            rand_target(target, loop_count);
            lcd_need_update = 1;
            is_started = 1;
            clear_LCD();
        }
        
        // select
        if (keyin == 5 && is_started) {
            selected[selecting_index] = random_target[selecting_index];
            selecting_index += 1;
            
            if (selecting_index == 4) {
                selecting_index = 0;
                
                score = cal_score(target, selected, score, speed);
                
                rand_target(target, loop_count);
                selected[0] = selected[1] = selected[2] = selected[3] = 0;
                
                lcd_need_update = 1;
            }
        }
        
        // clear
        if (keyin == 6 && is_started) {
            score = 0;
            lcd_need_update = 1;
        }
        
        // up
        if (keyin == 2 && is_started) {
            if (speed < 1000) speed += 100;
            lcd_need_update = 1;
        }
        
        // down
        if (keyin == 8 && is_started) {
            if (speed > 100) speed -= 100;
            lcd_need_update = 1;
        }
        
        if (lcd_need_update) update_lcd_buffer(lcd_buffer, target, speed, score);
    }
}

