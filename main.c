/*
 * File:   main.c
 * Author: Tomas Vacek
 *
 * Created on 04 April 2023, 19:58
 */

#include <stdio.h>
#include <stdint.h>
#include <xc.h>

#include "functions.h"
//#include "lcd.h"


void main(void) {
    
    init();

//    GPIO();
    
//    PWM();
    
//    UART();
    
    ADC();
    
    return;
}