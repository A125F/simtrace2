/* ----------------------------------------------------------------------------
 *         ATMEL Microcontroller Software Support
 * ----------------------------------------------------------------------------
 * Copyright (c) 2009, Atmel Corporation
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
#include "simtrace.h"
#include "simtrace_usb.h"
#include "ringbuffer.h"
#include "iso7816_fidi.h"

#include <string.h>
#include <errno.h>

volatile uint32_t char_stat;

volatile ringbuf sim_rcv_buf = { {0}, 0, 0 };

/*-----------------------------------------------------------------------------
 *          Interrupt routines
 *-----------------------------------------------------------------------------*/
static void Callback_PhoneRST_ISR(uint8_t * pArg, uint8_t status,
				  uint32_t transferred, uint32_t remaining)
{
	printf("rstCB\n\r");
	PIO_EnableIt(&pinPhoneRST);
}

void ISR_PhoneRST(const Pin * pPin)
{
	int ret;
	// FIXME: no printfs in ISRs?
	printf("+++ Int!! %lx\n\r", pinPhoneRST.pio->PIO_ISR);
	if (((pinPhoneRST.pio->PIO_ISR & pinPhoneRST.mask) != 0)) {
		if (PIO_Get(&pinPhoneRST) == 0) {
			printf(" 0 ");
		} else {
			printf(" 1 ");
		}
	}

	if ((ret =
	     USBD_Write(SIMTRACE_USB_EP_PHONE_INT, "R", 1,
			(TransferCallback) & Callback_PhoneRST_ISR,
			0)) != USBD_STATUS_SUCCESS) {
		TRACE_ERROR("USB err status: %d (%s)\r\n", ret, __FUNCTION__);
		return;
	}

	/* Interrupt enabled after ATR is sent to phone */
	PIO_DisableIt(&pinPhoneRST);
}

/*
 *  char_stat is zero if no error occurred.
 *  Otherwise it is filled with the content of the status register.
 */
void mode_trace_usart1_irq(void)
{
	uint32_t stat;
	char_stat = 0;
	// Rcv buf full
/*	if((stat & US_CSR_RXBUFF) == US_CSR_RXBUFF) {
		TRACE_DEBUG("Rcv buf full");
		USART_DisableIt(USART1, US_IDR_RXBUFF);
	}
*/
	uint32_t csr = USART_PHONE->US_CSR;

	if (csr & US_CSR_TXRDY) {
		/* transmit buffer empty, nothing to transmit */
	}
	if (csr & US_CSR_RXRDY) {
		stat = (csr & (US_CSR_OVRE | US_CSR_FRAME |
			       US_CSR_PARE | US_CSR_TIMEOUT | US_CSR_NACK |
			       (1 << 10)));
		uint8_t c = (USART_PHONE->US_RHR) & 0xFF;
//		printf(" %x", c);

		if (stat == 0) {
			/* Fill char into buffer */
			rbuf_write(&sim_rcv_buf, c);
		} else {
			TRACE_DEBUG("e %x st: %lx\r\n", c, stat);
		}		/* else: error occurred */

		char_stat = stat;
	}
}

/*  FIDI update functions   */
void update_fidi(Usart_info *usart, uint8_t fidi)
{
	if (NULL==usart) {
		return;
	}

	uint8_t fi = fidi >> 4;
	uint8_t di = fidi & 0xf;
	int ratio = iso7816_3_compute_fd_ratio(fi, di);

	if (ratio > 0 && ratio < 0x8000) {
		/* make sure USART uses new F/D ratio */
		usart->base->US_CR |= US_CR_RXDIS | US_CR_RSTRX;
		/* disable write protection */
		if (usart->base->US_WPMR) {
			usart->base->US_WPMR = US_WPMR_WPKEY(0x555341);
		}
		usart->base->US_FIDI = (ratio & 0x7ff);
		usart->base->US_CR |= US_CR_RXEN | US_CR_STTTO;
		//TRACE_INFO("updated USART(%u) Fi(%u)/Di(%u) ratio(%d): %u\n\r", usart->id, fi, di, ratio, usart->base->US_FIDI);
	} else {
		//TRACE_WARNING("computed Fi/Di ratio %d unsupported\n\r", ratio); /* don't print since this is function is also called by ISRs */
	}
}
