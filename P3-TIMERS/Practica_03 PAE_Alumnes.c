/******************************
 *
 * Practica_03_PAE
 *****************************/

#include <msp432p401r.h>
#include <stdio.h>
#include <stdint.h>
#include "lib_PAE2.h" //Libreria grafica + configuracion reloj MSP432

char saludo[16] = " PRACTICA 3 PAE";//max 15 caracteres visibles
char cadena[16];//Una linea entera con 15 caracteres visibles + uno oculto de terminacion de cadena (codigo ASCII 0)
char borrado[] = "               "; //una linea entera de 15 espacios en blanco
uint8_t linea = 0;
uint8_t estado=0;
uint8_t estado_anterior = 8;
int index = 0; // index para bucles
uint8_t direccion = 0; // Direccion de los leds (0 quietos)(1 izquierda)(2 derecha)
uint8_t change = 0; // Variable para controlar cuando el led llega al limite
uint32_t retraso = 500000;

uint16_t max_led_speed = 7500; // Velocidad más lenta del movimiento de leds
uint16_t min_led_speed = 500; // Velocidad más rapida del movimiento de leds

uint8_t seleccion = 0; // Selecciona entre (1 alarma) o (0 reloj)
uint8_t modificar = 0; // Flag para comprobar si estamos o no modificando algun valor del reloj o la alarma
uint8_t posicion = 0; // Posicion para determinar que modificamos (0 horas) (1 minutos) (2 segundos)

uint8_t parpadeo = 0; // Flag para comprobar y gestinar un sistema de parpadeo

uint8_t alarma = 0; // Flag para comprobar si esta encendida o apagada la alarma

// Variables del reloj
uint8_t horas = 0;
uint8_t minutos = 0;
uint8_t segundos = 0;

// Variables de la alarma
uint8_t alarma_horas = 0;
uint8_t alarma_minutos = 0;
uint8_t alarma_segundos = 0;

#define Pulsador_S1 1
#define Pulsador_S2 2
#define Jstick_Left 3
#define Jstick_Right 4
#define Jstick_Up 5
#define Jstick_Down 6
#define Jstick_Center 7

/**************************************************************************
 * INICIALIZACIï¿½N DEL CONTROLADOR DE INTERRUPCIONES (NVIC).
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
    
    //Init del timer 0, BIT8 del primer registro ISER0
    NVIC->ICPR[0] |= BIT8; // Limpiamos el resudio que pueda quedar
    NVIC->ISER[0] |= BIT8; // Habilitamos las interrupciones del timer 0 en el NVIC

    //Init del timer 0, BITA del primer registro ISER0
    NVIC->ICPR[0] |= BITA; // Limpiamos el resudio que pueda quedar
    NVIC->ISER[0] |= BITA; // Habilitamos las interrupciones del timer 1 en el NVIC

    __enable_interrupt(); //Habilitamos las interrupciones a nivel global del micro.
}

/**************************************************************************
 * INICIALIZACIï¿½N DE LA PANTALLA LCD.
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
 * INICIALIZACIï¿½N DE LOS BOTONES & LEDS DEL BOOSTERPACK MK II.
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
 * CONFIGURACIï¿½N DEL PUERTO 7. A REALIZAR POR EL ALUMNO
 *
 * Sin datos de entrada
 *
 * Sin datos de salida
 *
 ****************************************************************************/
void config_P7_LEDS (void)
{

    // Establecemos el P7 cï¿½mo I/O digital.
    P7SEL0 &= 0x00;
    P7SEL1 &= 0x00;

    // Establecemos todos los pines del P7 cï¿½mo pines de salida.
    P7DIR |= 0xFF;

    // Iniciamos todos los pines apagados.
    P7OUT &= 0x01;

}

/*****************************************************************************
 * Init del timer 0
 *
 * Sin datos de entrada
 *
 * Sin datos de salida
 *
 ****************************************************************************/
void init_TA0(void){

    // Activamos las interrupciones del tiemr
    TA0CCTL0 |= CCIE;
    TA0CCTL0 &= ~CCIFG;

    // Establecemos el limite de tiempo del timer a ~1ms
    TA0CCR0 = 33;

    // Establecemos la frequencia a 2^15Hz
    TA0CTL = TASSEL__ACLK + ID__1 + MC__UP;

}

/*****************************************************************************
 * Init del timer 1
 *
 * Sin datos de entrada
 *
 * Sin datos de salida
 *
 ****************************************************************************/
void init_TA1 (void){

    //Definimos el valor del registro TA1CCTL0
    TA1CCTL0 |= CCIE;
    TA1CCTL0 &= ~CCIFG;

    //Definimos el registro TA1CCR0 con el numero de pulsos (1 segundo)
    TA1CCR0 = 32768;

    //definimos el valor del registro TA1CTL
    TA1CTL = TASSEL__ACLK + ID__1 + MC__UP;
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

    init_TA0(); // Iniciamos timer 0
    init_TA1(); // Iniciamos timer 1

    config_P7_LEDS();       // Iniciamos los leds de P7

    halLcdPrintLine(saludo,linea, INVERT_TEXT); //escribimos saludo en la primera linea
    linea++;                    //Aumentamos el valor de linea y con ello pasamos a la linea siguiente

    //Bucle principal (infinito):
    do {

        if (estado_anterior != estado) {            // Dependiendo del valor del estado se encenderï¿½ un LED u otro.
            sprintf(cadena,"Estado %02d", estado);  // Guardamos en cadena la siguiente frase: Estado "valor del estado",
                                                    //con formato decimal, 2 cifras, rellenando con 0 a la izquierda.
            escribir(cadena,linea); // Escribimos la cadena al LCD
            estado_anterior = estado; // Actualizamos el valor de estado_anterior, para que no estï¿½ siempre escribiendo.
            switch(estado){
            case Jstick_Left:
                if (modificar == 1){ // Si estamos modificando alguna de las horas del reloj/alarma
                    if (posicion > 0) // Comprobamos que estemos entre el rango de 0-2 (horas, minutos, segundos), sino saltamos al otro lado
                        posicion--;
                    } else {
                        posicion = 2;
                    }
                }
                // Establecemos la direcciï¿½n de los leds (izquierda)
                direccion = 1;
                // Encendemos los leds RGB (1/1/1)
                on_off_RGB_LED(1, 1, 1);
                break;
            case Jstick_Right:
                if (modificar == 1){ // Si estamos modificando alguna de las horas del reloj/alarma
                    if (posicion < 2){ // Comprobamos que estemos entre el rango de 0-2 (horas, minutos, segundos), sino saltamos al otro lado
                        posicion++;
                    } else {
                        posicion = 0;
                    }
                }
                // Establecemos la direcciï¿½n de los leds (derecha)
                direccion = 2;
                // Encendemos los leds RGB (0/1/1)
                on_off_RGB_LED(0, 1, 1);
                break;
            case Jstick_Center:
                // INVERTIR ESTADO LED RGB (ON/FF) POR CADA COLOR
                P2OUT ^= 0x40;
                P2OUT ^= 0x10;
                P5OUT ^= 0x40;
                break;
            case Jstick_Up:
                //TODO COMENTAR:
                if (modificar == 0){ // Si no estamos modificando alguna de las horas del reloj/alarma
                    if (seleccion > 0){ // Comprobamos que estemos en el rango de 0-1 (reloj, alarma), sino saltamos al otro lado
                        seleccion--;
                    } else {
                        seleccion = 1;
                    }
                } else {
                    // Comprobamos la posición para modificar entre (horas, minutos, segundos) y comprobamos los limites.
                    // Comprobamos también si estamos modificando alarma o reloj
                    switch (posicion)
                    {
                        case 0:
                            if (seleccion == 0){
                                if (horas < 23){
                                    horas++;
                                } else {
                                    horas = 0;
                                }
                            } else {
                                if (alarma_horas < 23){
                                    alarma_horas++;
                                } else {
                                    alarma_horas = 0;
                                }
                            }
                            break;
                        case 1:
                            if (seleccion == 0){
                                if (minutos < 59){
                                    minutos++;
                                } else {
                                    minutos = 0;
                                }
                            } else {
                                if (alarma_minutos < 59){
                                    alarma_minutos++;
                                } else {
                                    alarma_minutos = 0;
                                }
                            }
                            break;
                        case 2:
                            if (seleccion == 0){
                                if (segundos < 59){
                                    segundos++;
                                } else {
                                    segundos = 0;
                                }
                            } else {
                                if (alarma_segundos < 59){
                                    alarma_segundos++;
                                } else {
                                    alarma_segundos = 0;
                                }
                            }
                            break;
                        default:
                            break;
                    }
                }
                // Aumentamos velocidad
                if (TA0CCR0 > min_led_speed){
                    TA0CCR0 -= 250;
                }
                // Encendemos los leds RGB (1/0/1)
                on_off_RGB_LED(1, 0, 1);
                break;
            case Jstick_Down:
                //TODO COMENTAR:
                if (modificar == 0){ // Si no estamos modificando alguna de las horas del reloj/alarma
                    if (seleccion < 1){ // Comprobamos que estemos en el rango de 0-1 (reloj, alarma), sino saltamos al otro lado
                        seleccion++;
                    } else {
                        seleccion = 0;
                    }
                } else {
                    // Comprobamos la posición para modificar entre (horas, minutos, segundos) y comprobamos los limites.
                    // Comprobamos también si estamos modificando alarma o reloj
                    switch (posicion)
                    {
                        case 0:
                            if (seleccion == 0){
                                if (horas > 0){
                                    horas--;
                                } else {
                                    horas = 23;
                                }
                            } else {
                                if (alarma_horas > 0){
                                    alarma_horas--;
                                } else {
                                    alarma_horas = 23;
                                }
                            }
                            break;
                        case 1:
                            if (seleccion == 0){
                                if (minutos > 0){
                                    minutos--;
                                } else {
                                    minutos = 59;
                                }
                            } else {
                                if (alarma_minutos > 0){
                                    alarma_minutos--;
                                } else {
                                    alarma_minutos = 59;
                                }
                            }
                            break;
                        case 2:
                            if (seleccion == 0){
                                if (segundos > 0){
                                    segundos--;
                                } else {
                                    segundos = 59;
                                }
                            } else {
                                if (alarma_segundos > 0){
                                    alarma_segundos--;
                                } else {
                                    alarma_segundos = 59;
                                }
                            }
                            break;
                        default:
                            break;
                    }
                }
                // Disminuimos velocidad
                if (TA0CCR0 < max_led_speed){
                    TA0CCR0 += 250;
                }
                // Encendemos los leds RGB (1/1/0)
                on_off_RGB_LED(1, 1, 0);
                break;
            case Pulsador_S1:
                modificar = 1;
                // Encendemos los leds RGB (1/1/1)
                on_off_RGB_LED(1, 1, 1);
                break;
            case Pulsador_S2:
                modificar = 0;
                // Apagamos los leds RGB (0/0/0)
                on_off_RGB_LED(0, 0, 0);
                break;
            default: // Estado 0
              /*  P2OUT ^= 0x40;     // Conmutamos el estado del LED R (bit 6)
                delay_t(retraso);  // periodo del parpadeo
                P2OUT ^= 0x10;     // Conmutamos el estado del LED G (bit 4)
                delay_t(retraso);  // periodo del parpadeo
                P5OUT ^= 0x40;     // Conmutamos el estado del LED B (bit 6)
                delay_t(retraso);  // periodo del parpadeo
              */
                break;
            }

        }

        // Render

        // Velocidad de los leds
        sprintf(cadena,"V Led: %d   ", TA0CCR0);
        escribir(cadena,3);

        // Reloj
        sprintf(cadena,"R: %02d:%02d:%02d", horas, minutos, segundos);
        escribir(cadena,5);

        // Reloj de Alarma
        sprintf(cadena,"A: %02d:%02d:%02d", alarma_horas, alarma_minutos, alarma_segundos);
        escribir(cadena,6);

        // Aviso de la alarma
        sprintf(cadena,"               ");
        if (alarma == 1){
            if (parpadeo == 1){
                sprintf(cadena,"ALARMAAA");
            }
        }
        escribir(cadena,7);

        // Render 
        if(modificar == 1){
            escribir("Modificando... ",8);
            if(seleccion == 1){ // alarma
                sprintf(cadena,"A -> ");
            } else { // reloj
                sprintf(cadena,"R -> ");
            }
            switch (posicion)
            {
                case 0: //horas
                    sprintf(cadena,"%sHoras     ",cadena);
                    break;
                case 1: //minutos
                    sprintf(cadena,"%sMinutos   ",cadena);
                    break;
                case 2: //segundos
                    sprintf(cadena,"%sSegundos  ",cadena);
                    break;
                default:
                    break;
            };
            escribir(cadena,9);
        } else {
            sprintf(cadena,"S1 Modificar");
            if(seleccion == 1){
                sprintf(cadena,"%s A ",cadena);
            } else {
                sprintf(cadena,"%s R ",cadena);
            }
            escribir(cadena,8);
            escribir("S2 Stop Modif. ",9);
        }



    } while(1); //Condicion para que el bucle sea infinito
}

// TODO: COMENTAR
void TA0_0_IRQHandler (void) //Cas del TA0. Aquest ï¿½s el nom important
{
    TA0CCTL0 &= ~CCIE; //Convï¿½ inhabilitar la interrupciï¿½ al comenï¿½ament
    /* El que volem fer a la rutina dï¿½atenciï¿½ dï¿½Interrupciï¿½ */
    /* Aquï¿½ no hem de fer cap comprovaciï¿½ addicional ja que */
    /* nomï¿½s pot haver una causa per generar la interrupciï¿½ */
    /* que el timer corresponent ha arribat al valor de CCR0 programat */

    // Movemos los leds en la direcciï¿½n correspondiente
    if(direccion == 1){
        P7OUT >>= 1; // Desplazamos el bit hacia la derecha
    } else if (direccion == 2) {
        P7OUT <<= 1; // Desplazamos el bit hacia la izquierda
    }

    // Comprobamos si tenemos que cambiar el led de un extremo a otro
    if(change == 1){
        P7OUT = 0x01;
        change = 0;
    }

    // Comprobamos que no nos hayamos pasado con la posiciï¿½n del bit en P7OUT
    if (P7OUT == 0x00){
        P7OUT = 0x80;
    } else if (P7OUT == 0x80){
        change = 1;
    }

    TA0CCTL0 &= ~CCIFG; //Hem de netejar el flag de la interrupciï¿½
    TA0CCTL0 |= CCIE; //Sï¿½ha dï¿½habilitar la interrupciï¿½ abans de sortir
}

void TA1_0_IRQHandler (void) //Cas del TA0. Aquest ï¿½s el nom important
{
    TA1CCTL0 &= ~CCIE; //Convï¿½ inhabilitar la interrupciï¿½ al comenï¿½ament
    /* El que volem fer a la rutina dï¿½atenciï¿½ dï¿½Interrupciï¿½ */
    /* Aquï¿½ no hem de fer cap comprovaciï¿½ addicional ja que */
    /* nomï¿½s pot haver una causa per generar la interrupciï¿½ */
    /* que el timer corresponent ha arribat al valor de CCR0 programat */

    if (parpadeo == 1){
        parpadeo = 0;
    } else {
        parpadeo = 1;
    }

    segundos++;

    if (segundos >= 60) {
        minutos += (int)(segundos / 60);
        segundos %= 60;
    }
    if (minutos >= 60) {
        horas += (int)(minutos / 60);
        minutos %= 60;
    }
    if (horas >= 24) {
        horas %= 24;
    }

    if ((horas == alarma_horas) && (minutos == alarma_minutos) && (segundos == alarma_segundos)){
        alarma = 1;
    }

    if (alarma == 1){
        if ((horas == alarma_horas) && (minutos == alarma_minutos + 2)){
            alarma = 0;
        }
    }


    TA1CCTL0 &= ~CCIFG; //Hem de netejar el flag de la interrupciï¿½
    TA1CCTL0 |= CCIE; //Sï¿½ha dï¿½habilitar la interrupciï¿½ abans de sortir
}


/**************************************************************************
 * RUTINAS DE GESTION DE LOS BOTONES:
 * Mediante estas rutinas, se detectarï¿½ quï¿½ botï¿½n se ha pulsado
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
void PORT4_IRQHandler(void){  //interrupciï¿½n de los botones. Actualiza el valor de la variable global estado.
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
void PORT5_IRQHandler(void){  //interrupciï¿½n de los botones. Actualiza el valor de la variable global estado.
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
