/*
 * RobotCommunication.c
 *
 *  Created on: 2 d’abr. 2019
 *      Author: mat.aules
 */

#include <UART_S.h>

uint8_t Byte_Recibido = 0;
uint8_t DatoLeido_UART = 0;

void init_UART(void){
    UCA2CTLW0 |= UCSWRST;       //Fem un reset de la USCI, desactiva la USCI
    UCA2CTLW0 |= UCSSEL__SMCLK; //UCSYNC=0 mode  asíncron
                                // UCMODEx=0 seleccionem mode UART
                                //UCSPB=0 nomes 1 stop bit
                                //UC7BIT=0 8 bits de dades
                                //UCMSB=0 bit de menys pes primer
                                //UCPAR=x ja que no es fa servir bit de paritat
                                //UCPEN=0 sense bit de paritat
                                //Triem SMCLK (24MHz) com a font del clock BRCLK
    UCA2MCTLW = UCOS16;         // Necessitem sobre - mostreig => bit 0 = UCOS16 = 1
    UCA2BRW = 3;                //Prescaler de BRCLK fixat a 3. Com SMCLK va a24MHz,
                                //volem un baud rate de 500kb/s i fem sobre - mostreig de 16
                                //el rellotge de la UART ha de ser de 8MHz (24MHz/3).
    //Configurem els pins de la UART
    P3SEL0 |= BIT2 | BIT3;      //I/O funció: P3.3 = UART2TX, P3.2 = UART2RX
    P3SEL1 &= ~ (BIT2 | BIT3);
    //Configurem pin de selecció del sentit de les dades Transmissió/Recepeció
    P3SEL0 &= ~BIT0;            //Port P3.0 com GPIO
    P3SEL1 &= ~BIT0;
    P3DIR |= BIT0;              //Port P3.0 com sortida ( Data Direction selector Tx/Rx)
    P3OUT &= ~BIT0;             //Inicialitzem Sentit Dades a 0 (Rx)
    UCA2CTLW0 &= ~UCSWRST;      //Reactivem la línia de comunicacions sèrie
    // UCA2IE |= UCRXIE;        //Això només s’ha d’activar quan tinguem la rutina de recepció
}

/* funcions per canviar el sentit de les comunicacions */
void Sentit_Dades_Rx(void) {
                    //Configuració del Half Duplex dels motors: Recepció
    P3OUT &= ~BIT0; //El pin P3.0 (DIRECTION_PORT) el posem a 0 ( Rx )
}

void Sentit_Dades_Tx(void){
                    //Configuració del Half Duplex dels motors: Transmissió
    P3OUT |= BIT0;  //El pin P3.0 (DIRECTION_PORT) el posem a 1 (Tx)
}

/* funció TxUAC0(byte): envia un byte de dades per la UART 0 */
void TxUAC2(byte bTxdData)
{
    while (!TXD2_READY); // Espera a que estigui preparat el buffer de transmissió
    UCA2TXBUF = bTxdData;
}

// TxPacket()  3 paràmetres: ID del Dynamixel , Mida dels paràmetres, Instruction byte. torna la mida del " Return packet "
byte TxPacket(byte bID, byte bParameterLength, byte bInstruction, byte Parametros[16])
{
    byte bCount,bCheckSum,bPacketLength;
    byte TxBuffer [32];
    Sentit_Dades_Tx(); //El pin P3.0 (DIRECTION_PORT) el posem a 1 (Transmetre)
    TxBuffer[0] = 0xff; //Primers 2 bytes que indiquen inici de trama FF, FF.
    TxBuffer[1] = 0xff;
    TxBuffer[2] = bID; //ID del mòdul al que volem enviar el missatge
    TxBuffer [3] = bParameterLength+2; // Length (Parameter,Instruction,Checksum)
    TxBuffer[4] = bInstruction ; //Instrucció que enviem al Mòdul
    for(bCount= 0; bCount < bParameterLength; bCount++) //Comencem a generar la trama que hem d’enviar
    {
        TxBuffer[bCount+5] = Parametros[bCount];
    }
    bCheckSum = 0;
    bPacketLength = bParameterLength+4+2;
    for(bCount= 2;bCount< bPacketLength-1;bCount++) //Càlcul delchecksum
    {
        bCheckSum += TxBuffer[bCount];
    }
    TxBuffer[bCount] = ~bCheckSum; //Escriu el Checksum (complement a 1)
    for(bCount= 0; bCount < bPacketLength; bCount++) //Aquest bucle és el que envia la trama al Mòdul Robot
    {
        TxUAC2(TxBuffer[bCount]);
    }
    while((UCA2STATW&UCBUSY));
    //Espera fins que s’ha transmès el últim byte
    Sentit_Dades_Rx ();
    //Posem la línia de dades en Rx perquè el mòdul Dynamixel envia resposta
    return (bPacketLength);
}

//Aquest exemple no és complert, en principi RxPacket() torna una estructura “Status packet" que bàsicament
//consisteix en un array amb “Status Packet ”+ un byte indicant si hi ha un TimeOut. Això s’ha fet així perquè en C no
//es pot posar un array com paràmetre de tornada.
//Per altre banda, la part mostrada només llegeix els primers 4 bytes del status packet. Això és perquè el quart byte
//indica precisament quants bytes queden per llegir, el que vol dir que s’ha de fer un altre bucle “for” semblant per
//llegir els bytes que falten ....
//Evidentment ,es podria fer d’altres maneres, per exemple enviant com paràmetre el número de bytes a llegir ...
struct RxReturn RxPacket(void)
{
    struct RxReturn respuesta;
    byte bCount;//, bLenght, bChecksum;
    byte Rx_time_out=0;
    Sentit_Dades_Rx(); //Ponemos la linea half duplex en Rx
    start_TA1();
    for(bCount = 0; bCount < 4; bCount++) // bRxPacketLength; bCount++)
    {
        set_TA1_timout(10000);
        Byte_Recibido=0; //No_se_ha_recibido_Byte();
        while(!Byte_Recibido) //Se_ha_recibido_Byte())
        {
            Rx_time_out=get_TA1_timout(); //tiempo en decenas de microsegundos (ara 10ms)
            if(Rx_time_out)break;//sale del while
        }
        if(Rx_time_out)break;//sale del for si ha habido Timeout
        //Si no, es que tot ha ido bien, y leemos un dato :
        respuesta.StatusPacket[bCount] = DatoLeido_UART; //Get_Byte_Leido_UART();
    }//fin del for
    if(!Rx_time_out) // Continua llegint la resta de bytes del Status Packet
    {

    }

    return (respuesta);
}

void EUSCIA2_IRQHandler(void)  { //interrupcion de recepcion en la UART A2
    UCA2IE &= ~UCRXIE; //Interrupciones desactivadas en RX
    DatoLeido_UART = UCA2RXBUF;
    Byte_Recibido = 1; // byte recieved = True
    UCA2IE |= UCRXIE; //Interrupciones reactivadas en RX
}

