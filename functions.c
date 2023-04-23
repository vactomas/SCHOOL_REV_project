#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <xc.h>                 // XC8

#include "lcd.h"                // LCD knihovna
#include "UART.h"               // UART knihovna

#pragma config FOSC = HSMP      // External oscilator
#pragma config PLLCFG = ON      // 4X PLL - 32MHz
#pragma config FCMEN = ON       // Fail-Safe Clock 
#pragma config WDTEN = OFF      // Watchdog Timer OFF

#define _XTAL_FREQ 32E6

#define SETDUTY(x) CCPR1L = x

void GPIO(void);
void driveLED(char in);
void init_UART(void);
void UART(void);
void init_PWM(void);
void PWM(void);
void init_ADC(void);
void ADC();

void GPIO(void) {
    
    char led_state = 0;
    
    while (1) {

        for (int8_t i = 0; i < 6; i++) {

            led_state |= (1 << i);
            driveLED(led_state);
            __delay_ms(100);

        }

        for (int8_t i = 5; i >= 0; i--) {

            led_state &= ~(1 << i);
            driveLED(led_state);
            __delay_ms(100);

        }
        
//        driveLED(63);
        
    }
    
    return;

}

void PWM(void) {
    
    init_PWM();
    
    while (1) {
        
        GODONE = 1;             // Start conversion
        while(GODONE) {};       // Wait for conversion to finish
        
        LATC1 = 0;              // Set direction
        SETDUTY(ADRESH);
        
    }
    
    return;
    
}

void UART(void) {
    
    char incoming_char;
    char message_out[5];

    init_UART();
    
    /*
    A - /.-/
    B - /-.../
    C - /-.-./
    D - /-../
    E - /./ 
    */
    
    while(1) {
        
        if(UART_CharAvailable()) {
            incoming_char = UART_GetChar();

            switch (incoming_char) {
                case 'A':
                    printf(".-");
                    break;
                case 'B':
                    printf("-...");
                    break;
                case 'C':
                    printf("-.-.");
                    break;
                case 'D':
                    printf("-..");
                    break;
                case 'E':
                    printf(".");
                    break;
            }
        }            
    }
}

void ADC(void) {

    init_ADC();
    
    uint32_t pot1, pot2;
    uint32_t pot12;

    char message[8];
    
    while(1) {
        ADCON0bits.CHS = 5;
        GODONE = 1;
        while(GODONE);
        pot2 = (ADRESH << 8) | ADRESL;
        
        ADCON0bits.CHS = 4;
        GODONE = 1;
        while(GODONE);
        pot1 = (ADRESH << 8) | ADRESL;
        
        pot12 = pot2 << 10;
        pot12 += pot1;

        // convert_val(pot12, &message);

        // for (uint8_t i = 0; i < 20; i++) {
        //     UART_PutChar(message[i]);
        // }
        
        // sprintf(message,"%lu\n", (long) pot12);
        // UART_PutStr(message);

        printf("%lu\n", (long) pot12);

        __delay_ms(50);
    }
    
}

void init(void) {
    
    __delay_ms(500);
        
    TRISDbits.TRISD2 = 0;       // output LED1
    TRISDbits.TRISD3 = 0;       // output LED2
    TRISCbits.TRISC4 = 0;       // output LED3
    TRISDbits.TRISD4 = 0;       // output LED4
    TRISDbits.TRISD5 = 0;       // output LED5
    TRISDbits.TRISD6 = 0;       // output LED6
    
    ANSELC = 0;
    ANSELD = 0;
    
    LCD_Init();
    LCD_Clear();
    
    char loading [16], loading2 [16];
    sprintf(loading, "_Loading       ");
    sprintf(loading2, " ...            ");
    
    for (uint8_t i = 0; i < 5; i++) {
        LCD_ShowString(1, loading);
        __delay_ms(500);
        LCD_ShowString(2, loading2);
        __delay_ms(500);
        LCD_Clear();
    }
    
    sprintf(loading, "Ready           ");
    LCD_ShowString(1, loading);
    
    PEIE = 1;                   // peripheral interrupt enable
    GIE = 1;                    // global interrupt enable
    
    driveLED(63);
}

void init_PWM(void) {
    
    // PWM init
    
    TRISDbits.RD5 = 1;
    TRISCbits.RC2 = 1;
    
    CCPTMRS0bits.C1TSEL = 0b00;
    PR2 = 200;
    CCP1CONbits.P1M = 0b00;
    CCP1CONbits.CCP1M = 0b1101;
    CCPR1L = 0;
    TMR2IF = 0;
    TMR2ON = 1;
    while(!TMR2IF){};
    PSTR1CON |= 0b11;
    
    TRISDbits.RD5 = 0;
    TRISCbits.RC2 = 0;
    
    // ADC for potentiometer
    
    ANSELE = 0b1;                   // RE0/AN5
    ADCON2bits.ADFM = 0;            // Left justified
    ADCON2bits.ADCS = 0b110;        // Fosc/64
    ADCON2bits.ACQT = 0b110;        // 16 Tad
    ADCON0bits.ADON = 1;            // ADC enable
    ADCON0bits.CHS = 5;             // Channel AN5
    
    TRISD = 0b10000011;             // LEDs: 2..6 out
    TRISC = 0b00000001;             // RC0 BTN1, RC4 LED
    ANSELC = 0;
    ANSELD = 0;
    
    // Turn off LEDs
    
    LATD2 = 1;
    LATD3 = 1;
    LATC4 = 1;
    LATD4 = 1;
    LATD5 = 1;
    LATD6 = 1;
    
}

void init_UART(void) {
    
    TRISCbits.TRISC6 = 0;       // TX - input
    TRISCbits.TRISC7 = 1;       // RX - input
    
    SPBRG1 = 25;                // BAUD rate 19200 - 25
    
    TXSTA1bits.SYNC = 0;        // Async
    RCSTA1bits.SPEN = 1;        // UART enable
    TXSTA1bits.TXEN = 1;        // TX enable
    RCSTA1bits.CREN = 1;        // RX enable
    
    char UART_init[16];
    sprintf(UART_init, "UART            ");
    LCD_ShowString(1, UART_init);
    
}

void init_ADC(void) {
    
    // ADC for potentiometer
    
    ANSELE = 0b1;                   // RE0/AN5
    ANSELA = 0b00100000;            // RA5/AN4
    
    ADCON2bits.ADFM = 1;            // Right justified
    ADCON2bits.ADCS = 0b110;        // Fosc/64
    ADCON2bits.ACQT = 0b110;        // 16 Tad
    ADCON0bits.ADON = 1;            // ADC enable
    
    TRISCbits.TRISC6 = 0;           // TX - input
    TRISCbits.TRISC7 = 1;           // RX - input
    
    SPBRG1 = 25;                    // BAUD rate 19200 - 25
    
    TXSTA1bits.SYNC = 0;            // Async
    RCSTA1bits.SPEN = 1;            // UART enable
    TXSTA1bits.TXEN = 1;            // TX enable
    RCSTA1bits.CREN = 1;            // RX enable

    char ADC_init[16];
    sprintf(ADC_init, "ADC             ");
    LCD_ShowString(1, ADC_init);
}

void driveLED(char in){
    
        LATD2 = in & 1;             // LED0
        LATD3 = in & 2 ? 1 : 0;     // LED1
        LATC4 = in & 4 ? 1 : 0;     // LED2
        LATD4 = in & 8 ? 1 : 0;     // LED3
        LATD5 = in & 16 ? 1 : 0;    // LED4
        LATD6 = in & 32 ? 1 : 0;    // LED5
        
        return;
}