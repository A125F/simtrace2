/* ISO7816-3 Fi/Di tables + computation
 *
 * (C) 2010-2015 by Harald Welte <laforge@gnumonks.org>
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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307, USA
 */
#pragma once

#include <stdint.h>

/* Table 7 of ISO 7816-3:2006 */
extern const uint16_t iso7816_3_fi_table[16];

/* Table 8 from ISO 7816-3:2006 */
extern const uint8_t iso7816_3_di_table[16];

/* compute the F/D ratio based on F_index and D_index values */
int iso7816_3_compute_fd_ratio(uint8_t f_index, uint8_t d_index);
