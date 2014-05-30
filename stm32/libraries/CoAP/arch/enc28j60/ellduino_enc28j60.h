/*
 * Copyright (c) 2014 ELL-i co-operative.
 *
 * ELL-i software is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * ELL-i software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with ELL-i software.  If not, see
 * <http://www.gnu.org/licenses/>.
 */

/*
 * Authors:  Pekka Nikander <pekka.nikander@ell-i.org>  2014
 */

#ifndef _ELLDUINO_ENC28J60_H_
# define _ELLDUINO_ENC28J60_H_

# define ENC28J60_SPI SPI2struct

#include <ENC28J60Class.h>

// XXX
const uint8_t temp_eth_address[6] = { 0, 0, 0, 0, 0, 0 };

static const ENC28J60Class ENC28J60(40, temp_eth_address);

#endif//_ELLDUINO_ENC28J60_H_