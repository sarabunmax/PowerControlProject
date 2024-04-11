/* Microchip Technology Inc. and its subsidiaries.  You may use this software 
 * and any derivatives exclusively with Microchip products. 
 * 
 * THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS".  NO WARRANTIES, WHETHER 
 * EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED 
 * WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A 
 * PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION 
 * WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION. 
 *
 * IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, 
 * INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND 
 * WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS 
 * BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE.  TO THE 
 * FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS 
 * IN ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF 
 * ANY, THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
 *
 * MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE 
 * TERMS. 
 */

/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef CONFIG_H
#define	CONFIG_H

#include <xc.h> // include processor files - each processor file is guarded.  

#define _XTAL_FREQ 8000000  // Системна частота (Гц)

// Визначаємо піни для світлодіодів //
#define LED1 PORTAbits.RA4
#define LED2 PORTCbits.RC5
#define LED3 PORTCbits.RC4
#define LED4 PORTBbits.RB7
#define LED5 PORTAbits.RA5
#define LED_P PORTBbits.RB4

// Визначаємо піни для кнопок //
#define BUTTON_PLUS PORTAbits.RA0    // Кнопка збільшення швидкості двигуна
#define BUTTON_MINUS PORTAbits.RA3   // Кнопка зменшення швидкості двигуна
#define BUTTON_POWER PORTCbits.RC2   // Кнопка увімкнення/вимкнення пристрою
#define BUTTON_FILTER PORTCbits.RC3  // Кнопка аварійної зупинки двигуна

// Визначаємо піни для керуючих виходів //
#define OUT1 PORTAbits.RA2  // Вихід для керування двигуном
#define OUT2 PORTAbits.RA1  // Вихід для керування двигуном
#define PIT PORTCbits.RC7   // Вихід для регулювання напруги блоків живлення

// Конфігураційні біти контролера //
#pragma config FOSC = INTRCIO  // Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA4/OSC2/CLKOUT pin, I/O function on RA5/OSC1/CLKIN)
#pragma config WDTE = OFF      // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = ON      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // MCLR Pin Function Select bit (MCLR pin function is digital input, MCLR internally tied to VDD)#pragma config CP = OFF        // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF       // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = ON      // Brown-out Reset Selection bits (BOR enabled)
#pragma config IESO = OFF      // Internal External Switchover bit (Internal External Switchover mode is disabled)
#pragma config FCMEN = OFF     // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)


#endif	/* XC_HEADER_TEMPLATE_H */

