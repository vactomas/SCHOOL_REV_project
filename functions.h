/* 
 * File:   functions.h
 * Author: Tomas Vacek
 *
 * Created on 09 April 2023, 14:30
 */

#ifndef FUNCTIONS_H
#define	FUNCTIONS_H

// void var_transfer(uint8_t *BTN1_debounce, uint8_t *BTN2_debounce, uint8_t *BTN3_debounce, uint8_t *BTN4_debounce);
int menu(uint8_t *BTN1_debounce, uint8_t *BTN2_debounce, uint8_t *BTN3_debounce, uint8_t previous_menu_state);
void GPIO(uint8_t *BTN4_debounce);
void UART(uint8_t *BTN4_debounce);
void PWM(uint8_t *BTN4_debounce);
void ADC(uint8_t *BTN4_debounce);
void DAC(uint8_t *BTN4_debounce);
void GAME(uint8_t *BTN1_debounce, uint8_t *BTN2_debounce, uint8_t *BTN4_debounce);
void HW(uint8_t *BTN4_debounce);
void init(void);

#endif	/* FUNCTIONS_H */

