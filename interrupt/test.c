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

int main() {

  // disable interrupts (not simulated in GVSOC)
  hal_irq_disable();

  // push interrupt to now + 1ms
  int us = 1000; // 1ms
  int ticks = us / (1000000 / ARCHI_REF_CLOCK) + 1;
  printf("Ticks=%d\n", ticks);
    // In order to simplify time comparison, we sacrify the MSB to avoid overflow
  // as the given amount of time must be short
  uint32_t current_time = timer_count_get(timer_base_fc(0, 1));
  uint32_t future_time = (current_time & 0x7fffffff) + ticks;

  // set comparator
  timer_cmp_set(timer_base_fc(0, 1), future_time);

  timer_conf_set(timer_base_fc(0, 1),
                  TIMER_CFG_LO_ENABLE(1) |
                  TIMER_CFG_LO_IRQEN(1) |
                  TIMER_CFG_LO_CCFG(1));

  // re-enable interrupts (not simulated in GVSOC)
  hal_irq_enable();
  // WFI
  hal_itc_wait_for_interrupt();

  printf("HELLO!\n");
  printf("Start=%d\n", current_time);
  printf("End=%d vs set=%d\n", timer_count_get(timer_base_fc(0, 1)), future_time);

  return 0;
}
