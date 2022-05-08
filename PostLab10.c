/*
 * File:   PostLab10.c
 * Author: jorge
 *
 * Created on 7 de mayo de 2022, 07:20 PM
 */
// PIC16F887 Configuration Bit Settings
// 'C' source line config statements
// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSC oscillator: CLKOUT function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = OFF      // Power-up Timer Enable bit (PWRT disabled)
#pragma config MCLRE = OFF      // RE3/MCLR pin function select bit (RE3/MCLR pin function is digital input, MCLR internally tied to VDD)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = OFF       // Internal External Switchover bit (Internal/External Switchover mode is disabled)
#pragma config FCMEN = OFF      // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is disabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)
// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)
// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.

#include <xc.h>
#include <stdint.h>             // int8_t, unit8_t
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define _XTAL_FREQ 1000000
int estado = 0;
int potenciometro = 0;
uint8_t seleccion;
uint8_t mensaje;
char cadena_potenciometro[]; 


void Imprimir (unsigned char *texto)
{
    while (*texto != '\0')
    {
    while (TXIF != 1);
    TXREG = *texto;
    *texto++;
    }
        
}

void __interrupt() isr (void){
    if(PIR1bits.ADIF){                                              // Interrupción por ADC
        if(ADCON0bits.CHS == 0){                                    // Interrupción por AN0
            potenciometro = ADRESH;                         // 2 bits menos significativos en DC1B    
        }
        PIR1bits.ADIF = 0;              // Limpiar la bandera de ADC
    }
    if(PIR1bits.RCIF){          // Evaluar nuevo dato en el serial
        if (estado == 0){
            seleccion = RCREG;
            RCREG = 0;
        }
        else if (estado == 1){
            estado = 0;
            mensaje = RCREG;
            TXREG = mensaje;
            RCREG = 0;
            Imprimir(" \r");
            Imprimir("\r MENU \r");
            Imprimir("a. Leer Potenciometro \r");
            Imprimir("b. Enviar ASCII \r");
        }
    }
    return;
}

void setup(void){
    ANSEL = 0b00000001;         // AN0 entrada analógica        
    ANSELH = 0;                 // I/O digitales
    
    TRISA = 0b00000001;         // RA0 como entrada
    TRISB = 0;                  // PORTB como salidas
    PORTA = 0;                  // Se limpia PORTA
    PORTB = 0;                  // Se limpia PORTB
    
    // Configuración del oscilador
    OSCCONbits.IRCF = 0b0100;   // 1 MHz
    OSCCONbits.SCS = 1;         // Oscilador interno
    
    // Configuración serial
    TXSTAbits.SYNC = 0;
    TXSTAbits.BRGH = 1;
    BAUDCTLbits.BRG16 = 1;
    SPBRGH = 0;
    SPBRG = 25;                 // Baud rate de aprox 9600
    RCSTAbits.SPEN = 1;         // Habilitar comunicacion serial
    RCSTAbits.RX9 = 0;          // Solo 8 bits
    TXSTAbits.TXEN = 1;         // Habilitar transmisor
    RCSTAbits.CREN = 1;         // Habilitar receptor
    
    INTCONbits.GIE = 1;         // Habilitar interrupciones globales
    INTCONbits.PEIE = 1;        // Habilitar interrupciones de periféricos
    PIE1bits.RCIE = 1;          // Habilitar interrupciones de recepción
    PIR1bits.ADIF = 0;          // Limpiar la bandera de ADC
    PIE1bits.ADIE = 1;          // Habilitar interrupciones de ADC
    
    // Configuración ADC
    ADCON0bits.ADCS = 0b01;         // FOSC/8
    ADCON1bits.VCFG0 = 0;           // VDD
    ADCON1bits.VCFG1 = 0;           // VSS
    ADCON0bits.CHS = 0b0000;        // Selecciona el AN0
    ADCON1bits.ADFM = 0;            // Justificador a la izquierda
    ADCON0bits.ADON = 1;            // Habilitar modulo ADC
    __delay_us(320);                 // Sample time
}

void main(void) {
    setup();
    Imprimir("MENU \r");
    Imprimir("a. Leer Potenciometro \r");
    Imprimir("b. Enviar ASCII \r");
    while(1){
        if (ADCON0bits.GO == 0){
             __delay_us(40);                // Sample time
            ADCON0bits.GO = 1;              // On
        }
        if (seleccion == 0x61){
            seleccion = 0x00;
            Imprimir("Valor del potenciometro: \r");
            __delay_ms(400);
            itoa(cadena_potenciometro,potenciometro,10); //Conversión int a ASCII
            __delay_ms(400);
            Imprimir(cadena_potenciometro);
            Imprimir(" \r");
            __delay_ms(400);
            Imprimir(" \r");
            Imprimir("MENU \r");
            Imprimir("a. Leer Potenciometro \r");
            Imprimir("b. Enviar ASCII \r");        
        }
        else if (seleccion == 0x62){
            seleccion = 0x00;
            estado = 1;
            Imprimir("Caracter en ASCII: ");
        }
    }
    return;
}
