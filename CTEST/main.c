#include <stdio.h>
#include <stdint.h>

char saludo[16] = " PRACTICA 3 PAE";//max 15 caracteres visibles
char cadena[16];//Una linea entera con 15 caracteres visibles + uno oculto de terminacion de cadena (codigo ASCII 0)
char borrado[] = "               "; //una linea entera de 15 espacios en blanco
uint8_t linea = 0;
uint8_t estado=0;
uint8_t estado_anterior = 8;
int index = 0; // index para bucles
uint8_t direccion = 0; // Direcci�n de los leds (0 quietos)(1 izquierda)(2 derecha)
uint8_t change = 0; // Variable para controlar cuando el led llega al limite
uint32_t retraso = 500000;

uint16_t max_led_speed = 9500;
uint16_t min_led_speed = 500;

uint8_t seleccion = 0; // Seleccionador entre alamra y reloj // seleccion 0 = reloj, seleccion 1 = alarma
uint8_t modificar = 0; // Activador para saber si hay que modificar o no
uint8_t posicion = 0; // una vez se modifica algo, que posicion si horas minutos o segundos // 0 horas, 1 minutos 2 segundos

uint8_t parpadeo = 0; // variable para alteranar si escribir o no, en modo parpadear

uint8_t horas = 0;
uint8_t minutos = 0;
uint8_t segundos = 0;

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

void main(void) {

    do {

        if (estado_anterior != estado) {
           
            estado_anterior = estado;
            switch(estado){
            case Jstick_Left:

                break;
            case Jstick_Right:

                break;
            case Jstick_Center:

                break;
            case Jstick_Up:
                if (modificar != 0){
                    if (seleccion > 0){
                        seleccion--;
                    } else {
                        seleccion = 1;
                    }
                } else {
                    switch (posicion)
                    {
                        case 0:
                            if (seleccion == 0){
                                if (horas < 100){
                                    horas++;
                                } else {
                                    horas = 0;
                                }
                            } else {
                                if (alarma_horas < 100){
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

                break;
            case Jstick_Down:
                if (modificar != 0){
                    if (seleccion < 1){
                        seleccion++;
                    } else {
                        seleccion = 0;
                    }
                } else {
                    switch (posicion)
                    {
                        case 0:
                            if (seleccion == 0){
                                if (horas > 0){
                                    horas--;
                                } else {
                                    horas = 99;
                                }
                            } else {
                                if (alarma_horas > 0){
                                    alarma_horas--;
                                } else {
                                    alarma_horas = 99;
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

                break;
            case Pulsador_S1:

                break;
            case Pulsador_S2:

                break;
            default: // Estado 0

                break;
            }

        }

        // RENDER



    } while(1);
}