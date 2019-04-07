/*
 * RobotCommunication.h
 *
 *  Created on: 2 d’abr. 2019
 *      Author: mat.aules
 */


#ifndef UART_S_H_
#define UART_S_H_

//Includes
#include "TA1_Timer.h"

//Defines
typedef uint8_t byte;
#define TXD2_READY (UCA2IFG & UCTXIFG)

struct RxReturn {
    byte StatusPacket[4];
    byte Error;
};

void init_UART(void);
void Sentit_Dades_Rx(void);
void Sentit_Dades_Tx(void);
void TxUAC2(byte bTxdData);
byte TxPacket(byte bID, byte bParameterLength, byte bInstruction, byte Parametros[16]);
struct RxReturn RxPacket(void);

#endif /* UART_S_H_ */
