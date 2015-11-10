/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support
 * ----------------------------------------------------------------------------
 * Copyright (c) 2009, Atmel Corporation
 * Copyright (c) 2014, Christina Quast
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * - Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the disclaimer below.
 *
 * Atmel's name may not be used to endorse or promote products derived from
 * this software without specific prior written permission.
 *
 * DISCLAIMER: THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * ----------------------------------------------------------------------------
 */

/*------------------------------------------------------------------------------
 *          Headers
 *------------------------------------------------------------------------------*/

#include "board.h"

#include <string.h>

/*------------------------------------------------------------------------------
 *         Internal definitions
 *------------------------------------------------------------------------------*/
/** Maximum ucSize in bytes of the smartcard answer to a command.*/
#define MAX_ANSWER_SIZE         10

/** Maximum ATR ucSize in bytes.*/
#define MAX_ATR_SIZE            55


/*------------------------------------------------------------------------------
 *         Internal variables
 *------------------------------------------------------------------------------*/

/** ISO7816 pins */
static const Pin pinsISO7816[]    = {PINS_ISO7816};
/** Bus switch pins */
static const Pin pinsBus[]    = {PINS_BUS_DEFAULT};
/*  SIMcard power pin   */
static const Pin pinsPower[] = {PWR_PINS};
/** ISO7816 RST pin */
static const Pin pinIso7816RstMC  = PIN_ISO7816_RSTMC;

static struct Usart_info usart_info = {.base = USART_SIM, .id = ID_USART_SIM, .state = USART_RCV};

/*------------------------------------------------------------------------------
 *         Optional smartcard detection
 *------------------------------------------------------------------------------*/

/** Smartcard detection pin.*/
static const Pin pinSmartCard = SMARTCARD_CONNECT_PIN;

/**
 * PIO interrupt service routine. Checks if the smartcard has been connected
 * or disconnected.
 */
static void ISR_PioSmartCard( const Pin *pPin )
{
    /*  Check current level on pin */
    /*  The interrupt is already erased on read from the PIO_ISR (PIO Interrupt
     *  Status Register) by the calling higher level interrupt handler
     */
    if ( PIO_Get( &pinSmartCard ) == 0 )
    {
        CCID_Insertion();
    }
    else
    {
        CCID_Removal();
    }
}

/**
 * Configures the smartcard detection pin to trigger an interrupt.
 */
static void ConfigureCardDetection( void )
{
    printf("+++++ Configure PIOs\n\r");
    PIO_Configure( &pinSmartCard, 1 ) ;
    NVIC_EnableIRQ( PIOA_IRQn );
    PIO_EnableIt( &pinSmartCard ) ;
}

/*-----------------------------------------------------------------------------
 *          Initialization and run
 *-----------------------------------------------------------------------------*/
extern CCIDDriverConfigurationDescriptors configurationDescriptorCCID;

void CCID_configure ( void ) {
    CCIDDriver_Initialize();
// FIXME: Do we need to set priority?: NVIC_SetPriority( PIOA_IRQn, 10);
    PIO_ConfigureIt( &pinSmartCard, ISR_PioSmartCard ) ;
}

void CCID_exit ( void ) {
    PIO_DisableIt( &pinSmartCard ) ;
    USART_SetTransmitterEnabled(usart_info.base, 0);
    USART_SetReceiverEnabled(usart_info.base, 0);
}

void CCID_init( void )
{
    uint8_t pAtr[MAX_ATR_SIZE];
    uint8_t ucSize ;

    // FIXME: do we want to print ATR?
    /*  Initialize Atr buffer */
    memset( pAtr, 0, sizeof( pAtr ) ) ;

    ConfigureCardDetection() ;

    // Configure ISO7816 driver
    PIO_Configure(pinsISO7816, PIO_LISTSIZE(pinsISO7816));
    PIO_Configure(pinsBus, PIO_LISTSIZE(pinsBus));
    PIO_Configure(pinsPower, PIO_LISTSIZE(pinsPower));

    /* power up the card */
//    PIO_Set(&pinsPower[0]);

    ISO7816_Init(&usart_info, CLK_MASTER);
    USART_SetTransmitterEnabled(usart_info.base, 1);
    USART_SetReceiverEnabled(usart_info.base, 1);

    ISO7816_Set_Reset_Pin(&pinIso7816RstMC);
    /*  Read ATR */
    ISO7816_warm_reset() ;

    ISO7816_Datablock_ATR( pAtr, &ucSize ) ;

    /*  Decode ATR and print it */
    ISO7816_Decode_ATR( pAtr ) ;

    if(PIO_Get(&pinSmartCard) == 0) {
        CCID_Insertion();
    } else {
        CCID_Removal();
    }
}

void CCID_run( void )
{

    //if (USBD_Read(INT, pBuffer, dLength, fCallback, pArgument);

    CCID_SmartCardRequest();
}
