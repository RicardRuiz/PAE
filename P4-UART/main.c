//#include "msp.h"
#include "lib_controllers.h"

char str[16];
uint8_t led_status[2] = {0,0};
uint8_t state = 0;
uint8_t previous_state = 0;
const uint8_t lateral_distance = 50;
const uint8_t frontal_distance = 20;

#define Pulsador_S1 1
#define Pulsador_S2 2
#define Jstick_Left 3
#define Jstick_Right 4
#define Jstick_Up 5
#define Jstick_Down 6
#define Jstick_Center 7

void init_interruptions(){
    // Configuracion al estilo MSP430 "clasico":
    // Enable Port 4 interrupt on the NVIC
    // segun datasheet (Tabla "6-12. NVIC Interrupts", capitulo "6.6.2 Device-Level User Interrupts", p80-81 del documento SLAS826A-Datasheet),
    // la interrupcion del puerto 4 es la User ISR numero 38.
    // Segun documento SLAU356A-Technical Reference Manual, capitulo "2.4.3 NVIC Registers"
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

    //Int. del TA1, BITA
    NVIC->ICPR[0] |= BITA; // Limpiamos el resudio que pueda quedar
    NVIC->ISER[0] |= BITA; // Habilitamos las interrupciones del TA1 en el NVIC

    //Int. del UART, BIT(18)
    NVIC->ICPR[0] |= BIT(18); // Limpiamos el resudio que pueda quedar
    NVIC->ISER[0] |= BIT(18); // Habilitamos las interrupciones del UART en el NVIC

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
    halLcdPrintLine("               ", Linea, NORMAL_TEXT); //escribimos una linea en blanco
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
void init_buttons(void)
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

void main(void)
{
    WDTCTL = WDTPW+WDTHOLD;         // Paramos el watchdog timer
    init_ucs_24MHz();       //Ajustes del clock (Unified Clock System)
    init_buttons();
    init_UART();
    init_interruptions();
    init_LCD();

    //change_speed(200); // initializes the speed to 200

    //uint8_t* distance; // array for the right. left and front distances

    while(1) {

        /**********************************************************+
        Para gestionar las acciones:
        Boton S1, estado = 4
        Boton S2, estado = 1
        Joystick left, estado = 3
        Joystick right, estado = 2
        Joystick up, estado = 5
        Joystick down, estado = 6
        Joystick center, estado = 1
        ***********************************************************/

        if (state != previous_state) { // if we change the state we entrer the block
            switch(state) {
                case Pulsador_S1:
                  /*if (led_status[1] == 0) { // If the leds are switched off, we switch them on (and viceversa)
                        led_status[1] = 1;
                    }
                    else {
                        led_status[1] = 0;
                    }
                    switch_led(3, led_status[1]);*/
                    break;
                case Pulsador_S2: // stops the movement
                    //stop_movement();
                    break;
                case Jstick_Left: // turns left
                    //turn_left();
                    break;
                case Jstick_Right: // turns right
                    //turn_right();
                    break;
                case Jstick_Up: // If the leds are switched off, we switch them on (and viceversa)
                    /*if (led_status[0] == 0) {
                         led_status[0] = 1;
                     }
                     else {
                         led_status[0] = 0;
                     }
                     switch_led(2, led_status[0]);*/
                     break;
                case Jstick_Down: // moves backward
                    //move_backward();
                    break;
                case Jstick_Center: //moves forward
                    //move_forward();
                    break;
                default:
                    break;
            }
            previous_state = state;
        }
        //distance = getDistanceSensor(); // gets the distance from the other objects

        // RENDER

        sprintf(str,"Timeout: %d   ", get_TA1_timout());
        escribir(str, 0);

    }
}



/**************************************************************************
 * RUTINAS DE GESTION DE LOS BOTONES:
 * Mediante estas rutinas, se detectar� qu� bot�n se ha pulsado
 *
 * Sin Datos de entrada
 *
 * Sin datos de salida
 *
 **************************************************************************/
//ISR para las interrupciones del puerto 3:
void PORT3_IRQHandler(void){//interrupcion del pulsador S2
    uint8_t flag = P3IV; //guardamos el vector de interrupciones. De paso, al acceder a este vector, se limpia automaticamente.
    P3IE &= 0xDF;  //interrupciones del boton S2 en port 3 desactivadas
    previous_state=0;

    // Switch para comprobar el vector del P3
    switch(flag){
    case 0x0C: // Pulsador S2
        state = Pulsador_S2;
        break;
    }

    P3IE |= 0x20;   //interrupciones S2 en port 3 reactivadas
}

//ISR para las interrupciones del puerto 4:
void PORT4_IRQHandler(void){  //interrupciï¿½n de los botones. Actualiza el valor de la variable global estado.
    uint8_t flag = P4IV; //guardamos el vector de interrupciones. De paso, al acceder a este vector, se limpia automaticamente.
    P4IE &= 0x5D;   //interrupciones Joystick en port 4 desactivadas
    previous_state=0;

    // Switch para comprobar el vector del P4
    switch(flag){
    case 0x04: // Joystic Center
        state = Jstick_Center;
        break;
    case 0x0C: // Joystic Right
        state = Jstick_Right;
        break;
    case 0x10: // Joystic Left;
        state = Jstick_Left;
        break;
    }

    P4IE |= 0xA2;   //interrupciones Joystick en port 4 reactivadas
}

//ISR para las interrupciones del puerto 5:
void PORT5_IRQHandler(void){  //interrupciï¿½n de los botones. Actualiza el valor de la variable global estado.
    uint8_t flag = P5IV; //guardamos el vector de interrupciones. De paso, al acceder a este vector, se limpia automaticamente.
    P5IE &= 0xCD;   //interrupciones Joystick y S1 en port 5 desactivadas
    previous_state=0;

    // Switch para comprobar el vector del P5
    switch(flag){
    case 0x0A: // Joystic Up
        state = Jstick_Up;
        break;
    case 0x0C: // Joystic Down
        state = Jstick_Down;
        break;
    case 0x04: // Pulstador S1
        state = Pulsador_S1;
        break;
    }

    P5IE |= 0x32;   //interrupciones Joystick y S1 en port 5 reactivadas
}
