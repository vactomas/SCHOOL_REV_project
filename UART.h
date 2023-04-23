/* 
 * File:        UART.h
 * Author:      adame
 * Edited by:   Tomas Vacek
 *
 * Created on 21. ledna 2020, 13:05
 * Edited on 22 April 2023, 14:20
 */


#ifndef UART_H
#define	UART_H

int UART_GetChar(void);                     // nacte jeden znak
void UART_PutChar(char);                    // vypise jeden znak
int UART_CharAvailable(void);               // vrati nenulovou hodnotu prave kdyz je alespon jeden znak k dispozici pro cteni
void UART_PutStr(char * str);               // vypise retezec az do nuloveho znaku
void UART_putch(unsigned char data);
char UART_getch(void);

#endif	