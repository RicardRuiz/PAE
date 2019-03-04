/******************************
 *
 * Practica_02_PAE Programació de Ports
 * i pràctica de les instruccions de control de flux:
 * "do ... while", "switch ... case", "if" i "for"
 * UB, 02/2017.
 *****************************/

#include <msp432p401r.h>
#include <stdio.h>
#include <stdint.h>
#include "lib_PAE2.h" //Libreria grafica + configuracion reloj MSP432

char saludo[16] = " PRACTICA 2 PAE";//max 15 caracteres visibles
char cadena[16];//Una linea entera con 15 caracteres visibles + uno oculto de terminacion de cadena (codigo ASCII 0)
char borrado[] = "               "; //una linea entera de 15 espacios en blanco
uint8_t linea = 0;
uint8_t estado=0;
uint8_t estado_anterior = 8;
int index = 0; // index para bucles
uint8_t velocidad = 1; // Velocidad para el movimiento de los leds
uint8_t direccion = 0; // Dirección de los leds (0 quietos)(1 izquierda)(2 derecha)
uint32_t retraso = 500000;

#define Pulsador_S1 1
#define Pulsador_S2 2
#define Jstick_Left 3
#define Jstick_Right 4
#define Jstick_Up 5
#define Jstick_Down 6
#define Jstick_Center 7

/**************************************************************************
 * INICIALIZACIÓN DEL CONTROLADOR DE INTERRUPCIONES (NVIC).
 *
 * Sin datos de entrada
 *
 * Sin datos de salida
 *
 **************************************************************************/
void init_interrupciones(){
    // Configuracion al estilo MSP430 "clasico":
    // --> Enable Port 4 interrupt on the NVIC.
    // Segun el Datasheet (Tabla "6-39. NVIC Interrupts", apartado "6.7.2 Device-Level User Interrupts"),
    // la interrupcion del puerto 4 es la User ISR numero 38.
    // Segun el Technical Reference Manual, apartado "2.4.3 NVIC Registers",
    // hay 2 registros de habilitacion ISER0 y ISER1, cada uno para 32 interrupciones (0..31, y 32..63, resp.),
    // accesibles mediante la estructura NVIC->ISER[x], con x = 0 o x = 1.
    // Asimismo, hay 2 registros para deshabilitarlas: ICERx, y dos registros para limpiarlas: ICPRx.

    //Int. port 3 = 37 corresponde al bit 5 del segundo registro ISER1:
    NVIC->ICPR[1] |= BIT5; //Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[1] |= BIT5; //y habilito las interrupciones del puerto
    //Int. port 4 = 38 corresponde al bit 6 del segundo registro ISERx:
    NVIC->ICPR[1] |= BIT6; //Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[1] |= BIT6; //y habilito las interrupciones del puerto
    //Int. port 5 = 39 corresponde al bit 7 del segundo registro ISERx:
    NVIC->ICPR[1] |= BIT7; //Primero, me aseguro de que no quede ninguna interrupcion residual pendiente para este puerto,
    NVIC->ISER[1] |= BIT7; //y habilito las interrupciones del puerto

    __enable_interrupt(); //Habilitamos las interrupciones a nivel global del micro.
}

/**************************************************************************
 * INICIALIZACIÓN DE LA PANTALLA LCD.
 *
 * Sin datos de entrada
 *
 * Sin datos de salida
 *
 **************************************************************************/
void init_LCD(void)
{
    halLcdInit(); //Inicializar y configurar la pantallita
    halLcdClearScreenBkg(); //Borrar la pantalla, rellenando con el color de fondo
}

/**************************************************************************
 * BORRAR LINEA
 *
 * Datos de entrada: Linea, indica la linea a borrar
 *
 * Sin datos de salida
 *
 **************************************************************************/
void borrar(uint8_t Linea)
{
    halLcdPrintLine(borrado, Linea, NORMAL_TEXT); //escribimos una linea en blanco
}

/**************************************************************************
 * ESCRIBIR LINEA
 *
 * Datos de entrada: Linea, indica la linea del LCD donde escribir
 *                   String, la cadena de caracteres que vamos a escribir
 *
 * Sin datos de salida
 *
 **************************************************************************/
void escribir(char String[], uint8_t Linea)

{
    halLcdPrintLine(String, Linea, NORMAL_TEXT); //Enviamos la String al LCD, sobreescribiendo la Linea indicada.
}

/**************************************************************************
 * INICIALIZACIÓN DE LOS BOTONES & LEDS DEL BOOSTERPACK MK II.
 *
 * Sin datos de entrada
 *
 * Sin datos de salida
 *
 **************************************************************************/
void init_botons(void)
{
    //Configuramos botones y leds
    //***************************

    //Leds RGB del MK II:
      P2DIR |= 0x50;  //Pines P2.4 (G), 2.6 (R) como salidas Led (RGB)
      P5DIR |= 0x40;  //Pin P5.6 (B)como salida Led (RGB)
      P2OUT &= 0xAF;  //Inicializamos Led RGB a 0 (apagados)
      P5OUT &= ~0x40; //Inicializamos Led RGB a 0 (apagados)

    //Boton S1 del MK II:
      P5SEL0 &= ~0x02;   //Pin P5.1 como I/O digital,
      P5SEL1 &= ~0x02;   //Pin P5.1 como I/O digital,
      P5DIR &= ~0x02; //Pin P5.1 como entrada
      P5IES &= ~0x02;   // con transicion L->H
      P5IE |= 0x02;     //Interrupciones activadas en P5.1,
      P5IFG = 0;    //Limpiamos todos los flags de las interrupciones del puerto 5
      //P5REN: Ya hay una resistencia de pullup en la placa MK II

    //Boton S2 del MK II:
      P3SEL0 &= ~0x20;   //Pin P3.5 como I/O digital,
      P3SEL1 &= ~0x20;   //Pin P3.5 como I/O digital,
      P3DIR &= ~0x20; //Pin P3.5 como entrada
      P3IES &= ~0x20;   // con transicion L->H
      P3IE |= 0x20;   //Interrupciones activadas en P3.5
      P3IFG = 0;  //Limpiamos todos los flags de las interrupciones del puerto 3
      //P3REN: Ya hay una resistencia de pullup en la placa MK II

    //Configuramos los GPIOs del joystick del MK II:
      P4DIR &= ~(BIT1 + BIT5 + BIT7);   //Pines P4.1, 4.5 y 4.7 como entrades,
      P4SEL0 &= ~(BIT1 + BIT5 + BIT7);  //Pines P4.1, 4.5 y 4.7 como I/O digitales,
      P4SEL1 &= ~(BIT1 + BIT5 + BIT7);
      P4REN |= BIT1 + BIT5 + BIT7;  //con resistencia activada
      P4OUT |= BIT1 + BIT5 + BIT7;  // de pull-up
      P4IE |= BIT1 + BIT5 + BIT7;   //Interrupciones activadas en P4.1, 4.5 y 4.7,
      P4IES &= ~(BIT1 + BIT5 + BIT7);   //las interrupciones se generaran con transicion L->H
      P4IFG = 0;    //Limpiamos todos los flags de las interrupciones del puerto 4

      P5DIR &= ~(BIT4 + BIT5);  //Pines P5.4 y 5.5 como entrades,
      P5SEL0 &= ~(BIT4 + BIT5); //Pines P5.4 y 5.5 como I/O digitales,
      P5SEL1 &= ~(BIT4 + BIT5);
      P5IE |= BIT4 + BIT5;  //Interrupciones activadas en 5.4 y 5.5,
      P5IES &= ~(BIT4 + BIT5);  //las interrupciones se generaran con transicion L->H
      P5IFG = 0;    //Limpiamos todos los flags de las interrupciones del puerto 4
    // - Ya hay una resistencia de pullup en la placa MK II
}

/**************************************************************************
 * DELAY - A CONFIGURAR POR EL ALUMNO - con bucle while
 *
 * Datos de entrada: Tiempo de retraso. 1 segundo equivale a un retraso de 1000000 (aprox)
 *
 * Sin datos de salida
 *
 **************************************************************************/
void delay_t (uint32_t temps)
{
   volatile uint32_t i = 0;

    do {
        // Incrementamos una variable temporal
        i++;
        // Comprobamos que la variable temporal sea menor o igual al retraso de entrada
    } while(i <= temps);

}

/*****************************************************************************
 * CONFIGURACIÓN DEL PUERTO 7. A REALIZAR POR EL ALUMNO
 *
 * Sin datos de entrada
 *
 * Sin datos de salida
 *
 ****************************************************************************/
void config_P7_LEDS (void)
{

    // Establecemos el P7 cómo I/O digital.
    P7SEL0 &= 0x00;
    P7SEL1 &= 0x00;

    // Establecemos todos los pines del P7 cómo pines de salida.
    P7DIR |= 0xFF;

    // Iniciamos todos los pines apagados.
    P7OUT &= 0x00;

}

/*****************************************************************************
 * ACTIVADOR DE LED RGB POR COLORES
 *
 * Datos de entrada: Flags para activar/desactivar el led RGB
 *
 * Sin datos de salida
 *
 ****************************************************************************/

void on_off_RGB_LED(uint8_t red, uint8_t green, uint8_t blue){
    if (red == 1)    P2OUT |= 0x40; else P2OUT &= ~0x40;
    if (green == 1)  P2OUT |= 0x10; else P2OUT &= ~0x10;
    if (blue == 1)   P5OUT |= 0x40; else P5OUT &= ~0x40;
}

void main(void) {

    WDTCTL = WDTPW+WDTHOLD;         // Paramos el watchdog timer

    //Inicializaciones:
    init_ucs_24MHz();       //Ajustes del clock (Unified Clock System)
    init_botons();         //Configuramos botones y leds
    init_interrupciones();  //Configurar y activar las interrupciones de los botones
    init_LCD();             // Inicializamos la pantalla

    config_P7_LEDS();       // Iniciamos los leds de P7

    halLcdPrintLine(saludo,linea, INVERT_TEXT); //escribimos saludo en la primera linea
    linea++;                    //Aumentamos el valor de linea y con ello pasamos a la linea siguiente

    //Bucle principal (infinito):
    do {

        if (estado_anterior != estado) {            // Dependiendo del valor del estado se encenderá un LED u otro.
            sprintf(cadena,"Estado %02d", estado);  // Guardamos en cadena la siguiente frase: Estado "valor del estado",
                                                    //con formato decimal, 2 cifras, rellenando con 0 a la izquierda.
            escribir(cadena,linea); // Escribimos la cadena al LCD
            estado_anterior = estado; // Actualizamos el valor de estado_anterior, para que no esté siempre escribiendo.

            switch(estado){
            case Jstick_Left:
                // Establecemos la dirección de los leds (izquierda)
                direccion = 1;

                // Encendemos los leds RGB (1/1/1)
                // P2OUT |= 0x50;
                // P5OUT |= 0x40;
                on_off_RGB_LED(1, 1, 1);
                break;
            case Jstick_Right:
                // Establecemos la dirección de los leds (derecha)
                direccion = 2;

                // Encendemos los leds RGB (0/1/1)
                // P2OUT &= ~0x40;
                // P2OUT |= 0x10;
                // P5OUT |= 0x40;
                on_off_RGB_LED(0, 1, 1);
                break;
            case Jstick_Center:
                // INVERTIR ESTADO LED RGB (ON/FF) POR CADA COLOR
                P2OUT ^= 0x40;
                P2OUT ^= 0x10;
                P5OUT ^= 0x40;
                break;
            case Jstick_Up:
                // Aumentamos la velocidad de parpadeo de los leds
                velocidad++;

                // Encendemos los leds RGB (1/0/1)
                // P2OUT |= 0x40;
                // P2OUT &= ~0x10;
                // P5OUT |= 0x40;
                on_off_RGB_LED(1, 0, 1);
                break;
            case Jstick_Down:
                // Disminuios la velocidad de parpadeo de los leds
                velocidad--;
                // Comprobamos que no bajemos de 0
                if (velocidad <= 0){
                    velocidad = 1;
                }

                // Encendemos los leds RGB (1/1/0)
                // P2OUT |= 0x50;
                // P5OUT &= ~0x40;
                on_off_RGB_LED(1, 1, 0);
                break;
            case Pulsador_S1:
                // Encendemos los leds RGB (1/1/1)
                // P2OUT |= 0x50;
                // P5OUT |= 0x40;
                on_off_RGB_LED(1, 1, 1);
                break;
            case Pulsador_S2:
                // Apagamos los leds RGB (0/0/0)
                // P2OUT &= ~0x50;
                // P5OUT &= ~0x40;
                on_off_RGB_LED(0, 0, 0);
                break;
            }

        }

        if (estado == 0){
             P2OUT ^= 0x40;     // Conmutamos el estado del LED R (bit 6)
             delay_t(retraso);  // periodo del parpadeo
             P2OUT ^= 0x10;     // Conmutamos el estado del LED G (bit 4)
             delay_t(retraso);  // periodo del parpadeo
             P5OUT ^= 0x40;     // Conmutamos el estado del LED B (bit 6)
             delay_t(retraso);  // periodo del parpadeo
        }

        /* Comentar el if de control y descometnar esto junto con el for
        # Encendemos el bit del lado segun el lado en el que nos vemos
        if(direccion == 1){ 
            P7OUT = 0x08;
        } else if (direccion == 2) {
            P7OUT = 0x01;
        }
        */
        // for(index = 0; index < 8; index++){ 

            // Si la dirección es 0, significa que no tienen dirección de movimiento
            // por lo tanto no hace falta hacer el delay para ver los leds moverse
            // Comprobamos también la velocidad por si acaso es 0
            if ((direccion != 0) && (velocidad != 0)){
                delay_t(retraso / velocidad);
            }

            // Movemos los leds en la dirección correspondiente
            if(direccion == 1){ 
                P7OUT >>= 1; // Desplazamos el bit hacia la derecha
            } else if (direccion == 2) {
                P7OUT <<= 1; // Desplazamos el bit hacia la izquierda
            }

            // Comprobamos que no nos hayamos pasado con la posición del bit en P7OUT
            if (P7OUT == 0x00){
                P7OUT = 0x08;
            } else if (P7OUT > 0x08){
                P7OUT = 0x01;
            }

        // }

    } while(1); //Condicion para que el bucle sea infinito
}


/**************************************************************************
 * RUTINAS DE GESTION DE LOS BOTONES:
 * Mediante estas rutinas, se detectará qué botón se ha pulsado
 *
 * Sin Datos de entrada
 *
 * Sin datos de salida
 *
 * Actualizar el valor de la variable global estado
 *
 **************************************************************************/

//ISR para las interrupciones del puerto 3:
void PORT3_IRQHandler(void){//interrupcion del pulsador S2
    uint8_t flag = P3IV; //guardamos el vector de interrupciones. De paso, al acceder a este vector, se limpia automaticamente.
    P3IE &= 0xDF;  //interrupciones del boton S2 en port 3 desactivadas
    estado_anterior=0;

    // Switch para comprobar el vector del P3
    switch(flag){
    case 0x0C: // Pulsador S2
        estado = Pulsador_S2;
        break;
    }

    P3IE |= 0x20;   //interrupciones S2 en port 3 reactivadas
}

//ISR para las interrupciones del puerto 4:
void PORT4_IRQHandler(void){  //interrupción de los botones. Actualiza el valor de la variable global estado.
    uint8_t flag = P4IV; //guardamos el vector de interrupciones. De paso, al acceder a este vector, se limpia automaticamente.
    P4IE &= 0x5D;   //interrupciones Joystick en port 4 desactivadas
    estado_anterior=0;

    // Switch para comprobar el vector del P4
    switch(flag){
    case 0x04: // Joystic Center
        estado = Jstick_Center;
        break;
    case 0x0C: // Joystic Right
        estado = Jstick_Right;
        break;
    case 0x10: // Joystic Left;
        estado = Jstick_Left;
        break;
    }

    P4IE |= 0xA2;   //interrupciones Joystick en port 4 reactivadas
}

//ISR para las interrupciones del puerto 5:
void PORT5_IRQHandler(void){  //interrupción de los botones. Actualiza el valor de la variable global estado.
    uint8_t flag = P5IV; //guardamos el vector de interrupciones. De paso, al acceder a este vector, se limpia automaticamente.
    P5IE &= 0xCD;   //interrupciones Joystick y S1 en port 5 desactivadas
    estado_anterior=0;

    // Switch para comprobar el vector del P5
    switch(flag){
    case 0x0A: // Joystic Up
        estado = Jstick_Up;
        break;
    case 0x0C: // Joystic Down
        estado = Jstick_Down;
        break;
    case 0x04: // Pulstador S1
        estado = Pulsador_S1;
        break;
    }

    P5IE |= 0x32;   //interrupciones Joystick y S1 en port 5 reactivadas
}
