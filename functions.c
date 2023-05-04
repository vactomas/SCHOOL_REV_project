/*
File:   functions.c
Author: Tomas Vacek
*/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <xc.h>                 // XC8

#include "lcd.h"                // LCD knihovna
#include "UART.h"               // UART knihovna

#pragma config FOSC = HSMP      // External oscilator
#pragma config PLLCFG = ON      // 4X PLL - 32MHz
#pragma config FCMEN = ON       // Fail-Safe Clock 
#pragma config WDTEN = OFF      // Watchdog Timer OFF

#define _XTAL_FREQ 32E6

// #define DEBUG

#define SETDUTY(x) CCPR1L = x


// Declare functions

int menu(uint8_t *BTN1_debounce, uint8_t *BTN2_debounce, uint8_t *BTN3_debounce, uint8_t previous_menu_state);
void GPIO(uint8_t *BTN4_debounce);
void init_UART(void);
void UART(uint8_t *BTN4_debounce);
void init_PWM(void);
void PWM(uint8_t *BTN4_debounce);
void init_ADC(void);
void ADC(uint8_t *BTN4_debounce);
void init_DAC(void);
void DAC(uint8_t *BTN4_debounce);
void GAME(uint8_t *BTN1_debounce, uint8_t *BTN2_debounce, uint8_t *BTN4_debounce);
void init_HW(void);
void HW(uint8_t *BTN4_debounce);
void driveLED(char in);
void LCD_Update(char LCD_TEXT1[16], char LCD_TEXT2[16]);


// Menu

int menu(uint8_t *BTN1_debounce, uint8_t *BTN2_debounce, uint8_t *BTN3_debounce, uint8_t previous_menu_state) {

    typedef enum {
        GPIO_m = 1,
        UART_m = 2,
        PWM_m = 3,
        ADC_m = 4,
        DAC_m = 5,
        GAME_m = 6,
        HW_m = 7,
    } Menu_states;

    char LCD_TEXT_1[16], LCD_TEXT_2[16];

    Menu_states state = previous_menu_state;

    uint8_t return_state = 0;

    while (1) {

        // Menu movement

        if (*BTN1_debounce >= 63) {
            state = state + 1;
            if (state > 7) {
                state = 1;
            }
        }
        if (*BTN2_debounce >= 63) {
            state = state - 1;
            if (state < 1) {
                state = 7;
            }
        }

        // Menu FSM

        switch (state) {

            case GPIO_m:
                sprintf(LCD_TEXT_1, "> GPIO          ");
                sprintf(LCD_TEXT_2, "  UART          ");
                LCD_Update(LCD_TEXT_1, LCD_TEXT_2);
                return_state = 1;
                break;

            case UART_m:
                sprintf(LCD_TEXT_1, "> UART          ");
                sprintf(LCD_TEXT_2, "  PWM           ");
                LCD_Update(LCD_TEXT_1, LCD_TEXT_2);
                return_state = 2;
                break;

            case PWM_m:
                sprintf(LCD_TEXT_1, "> PWM           ");
                sprintf(LCD_TEXT_2, "  ADC           ");
                LCD_Update(LCD_TEXT_1, LCD_TEXT_2);
                return_state = 3;
                break;

            case ADC_m:
                sprintf(LCD_TEXT_1, "> ADC           ");
                sprintf(LCD_TEXT_2, "  DAC           ");
                LCD_Update(LCD_TEXT_1, LCD_TEXT_2);
                return_state = 4;
                break;

            case DAC_m:
                sprintf(LCD_TEXT_1, "> DAC           ");
                sprintf(LCD_TEXT_2, "  GAME          ");
                LCD_Update(LCD_TEXT_1, LCD_TEXT_2);
                return_state = 5;
                break;

            case GAME_m:
                sprintf(LCD_TEXT_1, "> GAME          ");
                sprintf(LCD_TEXT_2, "  HW            ");
                LCD_Update(LCD_TEXT_1, LCD_TEXT_2);
                return_state = 6;
                break;
            
            case HW_m:
                sprintf(LCD_TEXT_1, "> HW            ");
                sprintf(LCD_TEXT_2, "  GPIO          ");
                LCD_Update(LCD_TEXT_1, LCD_TEXT_2);
                return_state = 7;
                break;
        }

        if (*BTN3_debounce >= 63) {
            
            #ifdef DEBUG
            printf("\nReturning state - %i", return_state);
            #endif

            __delay_ms(200);
            break;
        }

        __delay_ms(100);

    }

    return return_state;
}


// GPIO - knight rider

void GPIO(uint8_t *BTN4_debounce) {
    
    char led_state = 0;
    driveLED(63);

    // LCD

    char LCD_TEXT1[16], LCD_TEXT2[16];

    sprintf(LCD_TEXT1, "      GPIO     ");
    sprintf(LCD_TEXT2, "               ");

    LCD_Update(LCD_TEXT1, LCD_TEXT2);
    
    while (1) {

        for (int8_t i = 5; i >= 0; i--) {

            led_state &= ~(1 << i);
            driveLED(led_state);
            __delay_ms(100);

            if (*BTN4_debounce > 14) break;

        }

        if (*BTN4_debounce > 14) {
            driveLED(63);
            break;
        }

        for (int8_t i = 0; i < 6; i++) {

            led_state |= (1 << i);
            driveLED(led_state);
            __delay_ms(100);

            if (*BTN4_debounce > 14) break;

        }

        // Stop

        if (*BTN4_debounce > 14) {
            break;
        }
        
    }

    driveLED(63);
    
    return;

}

// PWM - control motor and LED5

void PWM(uint8_t *BTN4_debounce) {
    
    init_PWM();
    
    while (1) {
        
        GODONE = 1;             // Start conversion
        while(GODONE) {};       // Wait for conversion to finish
        
        LATC1 = 0;              // Set direction
        SETDUTY(ADRESH);

        // Turn off PWM, steering, break

        if (*BTN4_debounce > 14) {
            
            SETDUTY(0);
            PSTR1CONbits.STR1A = 0;
            PSTR1CONbits.STR1B = 0;

            break;
        }

    }

    return;
    
}

// UART - morse code

void UART(uint8_t *BTN4_debounce) {
    
    char incoming_char;
    char message_out[5];

    printf("\n\n\nUART\n\n\n");
    
    /*
    A - /.-/
    B - /-.../
    C - /-.-./
    D - /-../
    E - /./ 
    */

    // LCD  

    char LCD_TEXT1[16], LCD_TEXT2[16]; 

    sprintf(LCD_TEXT1, "      UART     ");
    sprintf(LCD_TEXT2, "               ");

    LCD_Update(LCD_TEXT1, LCD_TEXT2);
    
    // UART listening loop

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

        if (*BTN4_debounce > 14) break;            
    }
}

// ADC - take value of POT1 and POT2 and send it to PC over UART

void ADC(uint8_t *BTN4_debounce) {

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

        printf("%lu\n", (long) pot12);

        __delay_ms(50);

        if (*BTN4_debounce > 20) break;

    }
    
}

// DAC - triak

void DAC(uint8_t *BTN4_debounce) {

    while (1) {

        if (*BTN4_debounce >= 20) break;

    }

}

// GAME

    // LCD Update function

void LCD_Update(char LCD_TEXT_1[16], char LCD_TEXT_2[16]) {
    LCD_ShowString(1, LCD_TEXT_1);
    LCD_ShowString(2, LCD_TEXT_2);
}

    // String update

void string_update(char * LCD_TEXT_1, char * LCD_TEXT_2, uint8_t * obstacle, uint8_t position) {
    
    if (position == 1) {
        *LCD_TEXT_1 = '>';
        *LCD_TEXT_2 = ' ';
    } else {
        *LCD_TEXT_1 = ' ';
        *LCD_TEXT_2 = '>';
    }
    
    for (uint8_t i = 1; i < 16; i++) {

        // Check if obstacle exists
        if (*(obstacle + 2) == 1) {

            #ifdef DEBUG
            printf("\nObstacle exists.");
            #endif
            
            // Check horizontal position of obstacle
            if (*(obstacle + 1) == (i)) {

                #ifdef DEBUG
                printf("\nRight horizontal position");
                #endif

                // Check vertical position of obstacle
                if (*obstacle == 1) {
                    *(LCD_TEXT_1 + i) = 'x';
                    *(LCD_TEXT_2 + i) = ' ';
                } else {
                    *(LCD_TEXT_1 + i) = ' ';
                    *(LCD_TEXT_2 + i) = 'x';
                }

            } else {
                *(LCD_TEXT_1 + i) = ' ';
                *(LCD_TEXT_2 + i) = ' ';
            }

        } else {
            *(LCD_TEXT_1 + i) = ' ';
            *(LCD_TEXT_2 + i) = ' ';
        }
    }
}

    // Main game

void GAME(uint8_t *BTN1_debounce, uint8_t *BTN2_debounce, uint8_t *BTN4_debounce) {


    // Define variables and default values


    uint8_t lives, position, obstacle[3];
    char LCD_TEXT_1[16], LCD_TEXT_2[16];
    srand(TMR3);

    #ifdef DEBUG
    printf("\n\n\nGAME\n\n\nTMR3 - %i", TMR3);
    #endif

    uint16_t distance = 0;          // Distance driven

    lives = 0b000111;
    position = (rand() > RAND_MAX / 2);          // Player position
    obstacle[0] = (rand() > RAND_MAX / 2);       // Position of obstacle (top/bottom)
    obstacle[1] = 15;               // Position (distance) - 0-15
    obstacle[2] = 1;                // Obstacle exists?


    // Initial LCD setting


    string_update(&LCD_TEXT_1, &LCD_TEXT_2, &obstacle, position);
    LCD_Update(LCD_TEXT_1, LCD_TEXT_2);


    // Game loop


    while(1) {


        // Update lives counter


        if (lives == 0b111) {
            LATD2 = 0;     // LED0
            LATD3 = 0;     // LED1
            LATC4 = 0;     // LED2
        } else if (lives == 0b011) {
            LATD2 = 0;
            LATD3 = 0;
            LATC4 = 1;
        } else if (lives == 0b001) {
            LATD2 = 0;
            LATD3 = 1;
            LATC4 = 1;
        }


        // Movement


        if (*BTN1_debounce >= 20) {
            position = 1;
        } else if (*BTN2_debounce >= 20) {
            position = 0;
        }


        // Generate obstacle if it doesn't already exist


        if (obstacle[2] == 0) {
            obstacle[2] = 1;
            obstacle[1] = 15;
            obstacle[0] = (rand() > RAND_MAX / 2);
        }
    

        // You hit an obstacle (-1 live) or you successfully moved one tile further


        if (position == obstacle[0] && obstacle[1] == 0 && obstacle[2] == 1) {
            lives >>= 1;    
            obstacle[2] = 0;
        } else if (obstacle[1] > 0) {
            obstacle[1] -= 1;
            distance += 1;
        } else {
            distance += 1;
            obstacle[2] = 0;
        }


        // Display functionality

            // Update strings

        string_update(&LCD_TEXT_1, &LCD_TEXT_2, &obstacle, position);

            // Update display

        LCD_Update(LCD_TEXT_1, LCD_TEXT_2);


        // Variable speed of game (difficulty increase with time)


        if (distance <= 50) {
            __delay_ms(100);
        } else if (distance > 50 && distance <= 150) {
            __delay_ms(80);
        } else if (distance > 150 && distance <= 450) {
            __delay_ms(70);
        } else if (distance > 450 && distance <= 800) {
            __delay_ms(60);
        } else if (distance > 800 && distance <= 1500) {
            __delay_ms(50);
        } else {
            __delay_ms(40);
        }


        // Exit game and gameover mechanics


        if (*BTN4_debounce > 20) {
            driveLED(63);
            break;
        } else if (lives < 1) {
            driveLED(63);
            sprintf(LCD_TEXT_1, "   GAME OVER!  ");
            sprintf(LCD_TEXT_2, "Distance: %i   ", distance);
            LCD_Update(LCD_TEXT_1, LCD_TEXT_2);
            __delay_ms(4000);
            break;
        }
    }
}

// HW - optoclen

void HW(uint8_t *BTN4_debounce) {

}

// inits

// Main init

void init(void) {
    
    __delay_ms(500);

    init_UART();
        
    TRISDbits.TRISD2 = 0;       // output LED1
    TRISDbits.TRISD3 = 0;       // output LED2
    TRISCbits.TRISC4 = 0;       // output LED3
    TRISDbits.TRISD4 = 0;       // output LED4
    TRISDbits.TRISD5 = 0;       // output LED5
    TRISDbits.TRISD6 = 0;       // output LED6
    
    // Turn off analog on C and D pins and BTN2, BTN3

    ANSELC = 0;
    ANSELD = 0;
    ANSELAbits.ANSA3 = 0;
    ANSELAbits.ANSA2 = 0;
    
    // Init LCD

    LCD_Init();
    LCD_Clear();
    
    char LCD_TEXT1[16], LCD_TEXT2[16];
    sprintf(LCD_TEXT1, "Loading        ");
    sprintf(LCD_TEXT2, "...            ");
    
    for (uint8_t i = 0; i < 5; i++) {
        LCD_ShowString(1, LCD_TEXT1);
        __delay_ms(500);
        LCD_ShowString(2, LCD_TEXT2);
        __delay_ms(500);
        LCD_Clear();
    }

    // Enable interrupts
    
    PEIE = 1;                   // peripheral interrupt enable
    GIE = 1;                    // global interrupt enable
    
    #ifdef DEBUG
    printf("\nInterrupts enabled");
    #endif

    // Set BTNs as input

    TRISAbits.TRISA2 = 1;
    TRISAbits.TRISA3 = 1;
    TRISAbits.TRISA4 = 1;
    TRISCbits.TRISC0 = 1;

    #ifdef DEBUG
    printf("\nButtons set as input");
    #endif

    // Timer 5 for debounce

    T5CONbits.TMR5CS = 0b00;    // source - instruction clock
    T5CONbits.T5CKPS = 0b11;    // prescaler
    TMR5IE = 1;                 // enable timer 5 interrupt
    TMR5IF = 0;
    TMR5ON = 1;
    
    
    // Timer 3 for randomness

    T3CONbits.TMR3CS = 0b00;    // source - instruction clock
    T3CONbits.T3CKPS = 0b11;    // prescaler
    TMR3IE = 1;
    TMR3IF = 0;
    TMR5ON = 1;


    #ifdef DEBUG
    printf("\nTimer inits completed");
    #endif

        
    // // Timer 4

    // // Clock input is fixed to instruction clock => 2 MHz
    
    // T4CONbits.T4CKPS = 0b10;        // 1:16 prescaler => 125 kHz    
    // T4CONbits.T4OUTPS = 0b1111;     // 1:16 postscaler => 7.8125 kHz
    
    // PR4 = 200;                      
        
    // TMR4IE = 1;    
    // TMR4IF = 0;
    // TMR4ON = 1;

    driveLED(0);

    __delay_ms(500);

    
    driveLED(63);
}

// init PWM

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

    PSTR1CONbits.STR1A = 1;
    PSTR1CONbits.STR1B = 1;
    
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
    
    driveLED(63);

    // LCD

    char LCD_TEXT1[16], LCD_TEXT2[16];

    sprintf(LCD_TEXT1, "      PWM      ");
    sprintf(LCD_TEXT2, "               ");

    LCD_Update(LCD_TEXT1, LCD_TEXT2);
    
}

// init UART

void init_UART(void) {
    
    TRISCbits.TRISC6 = 0;       // TX - input
    TRISCbits.TRISC7 = 1;       // RX - input
    
    SPBRG1 = 25;                // BAUD rate 19200 - 25
    
    TXSTA1bits.SYNC = 0;        // Async
    RCSTA1bits.SPEN = 1;        // UART enable
    TXSTA1bits.TXEN = 1;        // TX enable
    RCSTA1bits.CREN = 1;        // RX enable
    
    #ifdef DEBUG
    printf("\nUART init complete!");
    #endif
    
}

// init ADC and UART for sending data

void init_ADC(void) {
    
    // ADC for potentiometer
    
    ANSELE = 0b1;                   // RE0/AN5
    ANSELA = 0b00100000;            // RA5/AN4
    
    ADCON2bits.ADFM = 1;            // Right justified
    ADCON2bits.ADCS = 0b110;        // Fosc/64
    ADCON2bits.ACQT = 0b110;        // 16 Tad
    ADCON0bits.ADON = 1;            // ADC enable

    // LCD

    char LCD_TEXT1[16], LCD_TEXT2[16];

    sprintf(LCD_TEXT1, "      ADC      ");
    sprintf(LCD_TEXT2, "               ");

    LCD_Update(LCD_TEXT1, LCD_TEXT2);
}

// init DAC functionality

void init_DAC(void) {

    // LCD

    char LCD_TEXT1[16], LCD_TEXT2[16];

    sprintf(LCD_TEXT1, "      DAC      ");
    sprintf(LCD_TEXT2, "               ");

    LCD_Update(LCD_TEXT1, LCD_TEXT2);

}

// init HW

void init_HW(void) {

    // LCD

    char LCD_TEXT1[16], LCD_TEXT2[16];
    
    sprintf(LCD_TEXT1, "       HW      ");
    sprintf(LCD_TEXT2, "               ");

    LCD_Update(LCD_TEXT1, LCD_TEXT2);

}

// driveLED function to control LED behaviour

void driveLED(char in){
    
        LATD2 = in & 1;             // LED0
        LATD3 = in & 2 ? 1 : 0;     // LED1
        LATC4 = in & 4 ? 1 : 0;     // LED2
        LATD4 = in & 8 ? 1 : 0;     // LED3
        LATD5 = in & 16 ? 1 : 0;    // LED4
        LATD6 = in & 32 ? 1 : 0;    // LED5
        
        return;
}