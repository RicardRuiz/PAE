#include <TA1_Timer.h>

uint16_t time_out = 0; // Variable que guarda el time out establecido
uint16_t time_out_timer = 0; // Variable para control interno del crono.

void init_TA1(void){
    // Definimos el valor del registro TA1CCTL0
    TA1CCTL0 |= CCIE;
    TA1CCTL0 &= ~CCIFG;

    // Definimos el registro TA1CCR0 con el numero de pulsos (1 ms)
    TA1CCR0 = 33;

    // Definimos el valor del registro TA1CTL
    TA1CTL = TASSEL__ACLK;
}

void start_TA1(void){
    // Activa el flag MC_UP, activa el timer.
    TA1CTL |= MC__UP;
}

void stop_TA1(void){
    // Desactiva el flag MC_3, desactiva el timer.
    TA1CTL &= ~MC_3;
}

void reset_TA1(void){
    time_out_timer = time_out;
}

void set_TA1_timout(uint16_t timeout){
    // Desactivamos las interrupciones de TA1
    TA1CCTL0 &= ~CCIE;

    time_out = timeout;

    reset_TA1();

    // Limpiamos el flag de interrupción
    TA1CCTL0 &= ~CCIFG;
    // Activamos las interrcupes del TA1
    TA1CCTL0 |= CCIE;
}

uint16_t get_TA1_timout(void){
    // Devolvemos el time_out
    return time_out_timer;
}

void TA1_0_IRQHandler (void) {
    // Desactivamos las interrupciones de TA1
    TA1CCTL0 &= ~CCIE;

    if (time_out_timer > 0) {
        time_out_timer--;
    }

    // Limpiamos el flag de interrupción
    TA1CCTL0 &= ~CCIFG;
    // Activamos las interrcupes del TA1
    TA1CCTL0 |= CCIE;
}
