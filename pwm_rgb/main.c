/*
    ADC, PWM control RGB LED color by turn button
    Author: 邱柏宇
    email: poyu39.tw@gmail.com
*/

#include <stdio.h>
#include <stdlib.h>
#include "NUC100Series.h"
#include "MCU_init.h"
#include "SYS_init.h"

volatile uint8_t isConverted = 0;

void ADC_IRQHandler(void) {
    uint32_t u32Flag;
    u32Flag = ADC_GET_INT_FLAG(ADC, ADC_ADF_INT);
    if (u32Flag & ADC_ADF_INT) isConverted = 1;
    ADC_CLR_INT_FLAG(ADC, u32Flag);
}

// init ADC
void init_turn_button(void) {
    ADC_Open(ADC, ADC_INPUT_MODE_SINGLE_END, ADC_OPERATION_MODE_SINGLE, ADC_CH_7_MASK);
    ADC_POWER_ON(ADC);
    ADC_EnableInt(ADC, ADC_ADF_INT);
    NVIC_EnableIRQ(ADC_IRQn);
}

// get turn button value
uint16_t get_turn_button(void) {
    ADC_START_CONV(ADC);
    while (isConverted == 0);
    isConverted = 0;
    ADC_STOP_CONV(ADC);
    return (uint16_t) ADC_GET_CONVERSION_DATA(ADC, 7);
}


int main(void) {
    uint16_t adc_value = 0;
    uint16_t r = 0, g = 0, b = 0;
    uint16_t hue, h_i, f, p, q, t;
    
    SYS_Init();
    init_turn_button();
    
    PWM_EnableOutput(PWM0, PWM_CH_0_MASK | PWM_CH_1_MASK | PWM_CH_2_MASK| PWM_CH_3_MASK);
    PWM_Start(PWM0, PWM_CH_0_MASK | PWM_CH_1_MASK | PWM_CH_2_MASK | PWM_CH_3_MASK);
    
    while(TRUE) {
        adc_value = get_turn_button();
        hue = (adc_value * 360) / 4096;
        
        h_i = hue / 60;
        f = ((hue % 60) * 255) / 60;
        p = 0;
        q = (255 * (255 - f)) / 255;
        t = (255 * f) / 255;
        
        switch(h_i) {
            case 0: r = 255; g = t; b = p; break;
            case 1: r = q; g = 255; b = p; break;
            case 2: r = p; g = 255; b = t; break;
            case 3: r = p; g = q; b = 255; break;
            case 4: r = t; g = p; b = 255; break;
            case 5: r = 255; g = p; b = q; break;
        }
        
        // RGB (0-255) to duty cycle (0-100)
        PWM_ConfigOutputChannel(PWM0, PWM_CH0, 128, (r * 100) / 255);
        PWM_ConfigOutputChannel(PWM0, PWM_CH1, 128, (g * 100) / 255);
        PWM_ConfigOutputChannel(PWM0, PWM_CH2, 128, (b * 100) / 255);
    }
}
