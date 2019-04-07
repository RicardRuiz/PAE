/**
 * TA1_Timer
 * @author Ricard Ruiz & José Blanco
 */

#ifndef TA1_TIMER_H_
#define TA1_TIMER_H_

#include <msp432p401r.h>
#include <stdio.h>
#include <stdint.h>
#include "lib_PAE2.h"

/**
 * Configura el Timer A1
 */
void init_TA1(void);

/**
 * Inicia el Timer A1, activando el reloj
 */
void start_TA1(void);

/**
 * Para el Timer A1, desactivando el reloj
 */
void stop_TA1(void);

/**
 * Resetea el valor de la cuenta atras
 */
void reset_TA1(void);

/**
 * Establece un nuevo valor para hacer la cuenta atras
 * @param timeout nuevo valor para establecer, en ms
 */
void set_TA1_timout(uint16_t timeout);

/**
 * Devuelve el valor actual de la cuenta atras
 * @return la cuenta atras, en ms
 */
uint16_t get_TA1_timout(void);

#endif /*TA1_TIMER_H_*/
