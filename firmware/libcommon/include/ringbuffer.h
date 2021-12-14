/* Ring buffer
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */
#ifndef SIMTRACE_RINGBUF_H
#define SIMTRACE_RINGBUF_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

#define RING_BUFLEN 1024

typedef struct ringbuf {
	uint8_t buf[RING_BUFLEN];
	size_t ird;
	size_t iwr;
} ringbuf;

void rbuf_reset(volatile ringbuf * rb);
uint8_t rbuf_read(volatile ringbuf * rb);
uint8_t rbuf_peek(volatile ringbuf * rb);
int rbuf_write(volatile ringbuf * rb, uint8_t item);
bool rbuf_is_empty(volatile ringbuf * rb);
bool rbuf_is_full(volatile ringbuf * rb);

#endif /* end of include guard: SIMTRACE_RINGBUF_H */
