/*
 * Copyright (c) 2014 ELL-i co-operative
 *
 * This file is part of ELL-i software.
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
 * along with ELL-i software.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @author Pekka Nikander <pekka.nikander@ell-i.org>  2014
 *
 * @brief The Arduino Serial class
 */

#include <stm32f0xx.h>
#include <ellduino_gpio.h>   // XXX To be placed into the variant.h!
#include <ellduino_usart.h>  // XXX To be placed into the variant.h!
#include <arduelli_thread.h> // XXX TBD -- is this the right file name?
#include <Arduino_Stream.h>

class Serial : public Stream {
public:
    const USART usart_;
    constexpr Serial(const USART &);
    void begin(uint32_t) const;
    void write(uint8_t) const;
};

#define DEFINE_SERIAL(usart_number, tx_letter, tx_pin, tx_af, rx_letter, rx_pin, rx_af) \
    ({ DEFINE_USART_STRUCT(usart_number, tx_letter, tx_pin, tx_af, rx_letter, rx_pin, rx_af) })

constexpr Serial::Serial(const USART &usart)
    : usart_(usart) {}

inline void Serial::begin(uint32_t baudrate) const {
    /* Place the GPIO pins into the right alternative function */
    PinFunctionActivate(&usart_.usart_tx_function_);
    PinFunctionActivate(&usart_.usart_rx_function_);

    /* Set the baud rate -- use 16 bit oversampling */
    usart_.usart_->BRR = SystemCoreClock / baudrate;

    /* Enable the transmitter and the USART */
    usart_.usart_->CR1 |= USART_CR1_TE | USART_CR1_UE;
}

inline void Serial::write(uint8_t c) const {
    usart_.usart_->TDR = c;

    while ((usart_.usart_->ISR & USART_ISR_TXE) == 0) {
        yield();
    }
}
