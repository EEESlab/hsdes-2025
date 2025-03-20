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
#define VERBOSE

int global_ticks = -1;
int global = 64;

__attribute__ ((interrupt)) 
void timer_handler (void) {
  global--;
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

int main() {

  // disable interrupts
  hal_irq_disable();

  // compute duration of 10ms in ticks
  int us = 10000; // 10ms
  int ticks = us / (1000000 / ARCHI_REF_CLOCK) + 1;
  printf("Ticks=%d\n", ticks);
  global_ticks = ticks;

  // set up timer_handler as IRQ handler for ARCHI_FC_EVT_TIMER0_HI
  pos_irq_set_handler(ARCHI_FC_EVT_TIMER0_HI, timer_handler);
  // masking
  pos_irq_mask_set(0 << ARCHI_FC_EVT_TIMER0_HI);

  // arm timer with global_ticks
  timer_activate();

  // re-enable interrupts
  hal_irq_enable();

  do {
    hal_itc_wait_for_interrupt();
    hal_irq_disable(); // disable interrupts
#ifdef VERBOSE
    printf("%d\n", global);
#endif /* VERBOSE */
    timer_activate(); // re-arm timer
    hal_irq_enable(); // re-enable interrupts
  }
  while(global > 0);

  return 0;
}
