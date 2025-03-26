/*
 * Copyright (C) 2025 ETH Zurich and University of Bologna
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/*
 * Authors: Francesco Conti <f.conti@unibo.it>
 */

#include "pmsis.h"
// #define VERBOSE

int global_ticks = -1;
int global = 64;
int pin_status = 1;

__attribute__ ((interrupt)) 
void timer_handler (void) {
  global--;
  pin_status = 1-pin_status;
}

static inline void timer_activate(void) {
  // In order to simplify time comparison, we sacrify the MSB to avoid overflow
  // as the given amount of time must be short
  uint32_t current_time = timer_count_get(timer_base_fc(0, 1));
  uint32_t future_time = (current_time & 0x7fffffff) + global_ticks;

  // set comparator
  timer_cmp_set(timer_base_fc(0, 1), future_time);
  timer_conf_set(timer_base_fc(0, 1),
                  TIMER_CFG_LO_ENABLE(1) |
                  TIMER_CFG_LO_IRQEN(1) |
                  TIMER_CFG_LO_CCFG(1));
}

static inline void bitbang_putbit(int b) {
  hal_irq_disable(); // disable interrupts
  hal_gpio_set_value(0x00000001, b); // switch gpio
  timer_activate(); // re-arm timer
  hal_irq_enable(); // re-enable interrupts
  hal_itc_wait_for_interrupt();
}

int bitbang_putchar(char c) {
  if(c == '\0') {
    return 0;
  }
  else {
    char c_int = c;
    // start bit
    bitbang_putbit(0);
    // payload bits, one by one, LSB first
    for(int i=0; i<8; i++) {
      bitbang_putbit(c_int & 0x1);
      // shift out bits
      c_int = c_int >> 1;
    }
    // stop bit
    bitbang_putbit(1);
  }
  return 1;
}

void bitbang_send(char *s) {
  int ret = 0;
  int i = 0;
  do {
    ret = bitbang_putchar(s[i++]);
  } while(ret);
}

int main() {

  // disable interrupts
  hal_irq_disable();

  // compute duration of 10ms in ticks
  int us = 10000; // 10ms
  int ticks = us / (1000000 / ARCHI_REF_CLOCK) + 1;
  global_ticks = ticks;

  // set up timer_handler as IRQ handler for ARCHI_FC_EVT_TIMER0_HI
  pos_irq_set_handler(ARCHI_FC_EVT_TIMER0_HI, timer_handler);
  // masking
  pos_irq_mask_set(0 << ARCHI_FC_EVT_TIMER0_HI);

  // arm timer with global_ticks
  timer_activate();

  // configure GPIO0
  int mask = 0x00000001;
  hal_gpio_set_dir(mask, 1);
  hal_gpio_en_set(hal_gpio_en_get() & ~mask);

  // set GPIO0 to current pin_status
  hal_gpio_set_value(mask, pin_status);

  // re-enable interrupts
  hal_irq_enable();

  // prime the UART line with a few ones
  for(int i=0; i<16; i++) {
    bitbang_putbit(1);
  }

  // send "Hello World!" via uart
  bitbang_send("Hello World!\n");

  return 0;
}
