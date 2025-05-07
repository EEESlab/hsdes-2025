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
int global = 1;
int pin_status = 1;

uint8_t data[] = "Nel mezzo del cammin di nostra vita";

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

int xor_data(uint8_t *data, size_t len, uint8_t key) {
  int i=0;
  // no checks whatsoever: who cares about safe code and buffer overflows? >:D
  for(i=0; i<len; i++) {
    if(data[i] == '\0') // just kidding, we do a (minimal!) check here!
      break;
    data[i] ^= key;
  }
  return i;
}

__attribute__ ((interrupt)) 
void timer_handler (void) {
  hal_irq_disable(); // disable interrupts
  global--;
  hal_irq_enable(); // re-enable interrupts
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
    // no wait for interrupt here!
    xor_data(data, strlen((char *) data), 0xaa);
#ifdef VERBOSE
    printf("%s\n", data);
#endif /* VERBOSE */
  }
  while(global > 0);
  printf("%d\n", global);
  printf("%s\n", data);

  return 0;
}
