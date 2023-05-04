/*
File:   main.c
Author: Tomas Vacek

Created on 04 April 2023, 19:58
*/

#include <stdio.h>
#include <stdint.h>
#include <xc.h>

#include "functions.h"

#define BTN1 PORTCbits.RC0
#define BTN2 PORTAbits.RA4
#define BTN3 PORTAbits.RA3
#define BTN4 PORTAbits.RA2

#define _XTAL_FREQ 32E6

// #define DEBUG

// Define global variables

uint8_t ACT_BTN1 = 0, ACT_BTN2 = 0, ACT_BTN3 = 0, ACT_BTN4 = 0;


// FMS typedef

typedef enum {
    IDLE = 0,
    GPIO_f = 1,
    UART_f = 2,
    PWM_f = 3,
    ADC_f = 4,
    DAC_f = 5,
    GAME_f = 6,
    HW_f = 7,
} State_type;


// Interrupt

void __interrupt() ISR_debouncer() {

    // Debounce

    if (TMR5IF && TMR5IE) {
        if (BTN1) {
            ACT_BTN1 <<= 1;
            ACT_BTN1 |= BTN1;
        } else {
            ACT_BTN1 = 0;
        }

        if (BTN2) {
            ACT_BTN2 <<= 1;
            ACT_BTN2 |= BTN2;
        } else {
            ACT_BTN2 = 0;
        }

        if (BTN3) {
            ACT_BTN3 <<= 1;
            ACT_BTN3 |= BTN3;
        } else {
            ACT_BTN3 = 0;
        }

        if (BTN4) {
            ACT_BTN4 <<= 1;
            ACT_BTN4 |= BTN4;
        } else {
            ACT_BTN4 = 0;
        }

        TMR5 = 0xFFFF - 999;
        TMR5IF = 0;       
    }

    // Timer3 reset

    if (TMR3IF && TMR3IE) {
        TMR3 = 0;
        TMR3IF = 0;
    }

}


// Main function

void main(void) {

    init();

    State_type current_state = 0;

    uint8_t previous_menu_state = 1;

    while(1) {

        switch (current_state) {

            case IDLE:

                #ifdef DEBUG
                printf("\nIDLE");
                #endif

                current_state = menu(&ACT_BTN1, &ACT_BTN2, &ACT_BTN3, previous_menu_state);

                break;

            case GPIO_f:

                #ifdef DEBUG
                printf("\nGPIO");
                #endif

                GPIO(&ACT_BTN4);
                previous_menu_state = 1;
                current_state = 0;
                break;

            case UART_f:

                #ifdef DEBUG
                printf("\nUART");
                #endif

                UART(&ACT_BTN4);
                previous_menu_state = 2;
                current_state = 0;
                break;

            case PWM_f:

                #ifdef DEBUG
                printf("\nPWM");
                #endif

                PWM(&ACT_BTN4);
                previous_menu_state = 3;
                current_state = 0;
                break;

            case ADC_f:

                #ifdef DEBUG
                printf("\nADC");
                #endif

                ADC(&ACT_BTN4);
                previous_menu_state = 4;
                current_state = 0;
                break;

            case DAC_f:

                #ifdef DEBUG
                printf("\nDAC");
                #endif

                DAC(&ACT_BTN4);
                previous_menu_state = 5;
                current_state = 0;
                break;

            case GAME_f:

                #ifdef DEBUG
                printf("\nGAME");
                #endif

                GAME(&ACT_BTN1, &ACT_BTN2, &ACT_BTN4);
                previous_menu_state = 6;
                current_state = 0;
                break;

            case HW_f:

                #ifdef DEBUG
                printf("\nHW");
                #endif

                HW(&ACT_BTN4);
                previous_menu_state = 7;
                current_state = 0;
                break;
        }

        __delay_ms(50);
    }

    return;
}