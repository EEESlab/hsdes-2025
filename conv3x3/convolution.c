/*
 * Copyright (C) 2015-2020 ETH Zurich and University of Bologna
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

#include "pmsis.h"
#include "conv_kernel.h"
#include "data.h"

#define NB_ITER 1

// global define
static uint8_t  Out[IMG_DIM];
static uint8_t  In[IMG_DIM];
static uint8_t  Kernel[FILT_DIM];

// function prototypes
void check_function (int *errors);
void __attribute__ ((noinline))  InitData         (uint8_t * __restrict__ Img,    int size);
void __attribute__ ((noinline))  InitZero         (uint8_t * __restrict__ Img,    int size);
void __attribute__ ((noinline))  InitKernel       (uint8_t * __restrict__ Kernel, int size);
int  __attribute__ ((noinline))  checkresult      (uint8_t * __restrict__ Out, uint8_t * __restrict__ OutGold, int N);

int main()
{
  int errors = 0;
  for(int i=0; i<NB_ITER; i++) {
    check_function(&errors);
  }

  printf("errors=%d\n", errors);

  int mac = (IMG_ROW-2)*(IMG_COL-2)*9;
  int instr_cnt = pi_perf_read(PI_PERF_INSTR);
  int cycles_cnt = pi_perf_read(PI_PERF_CYCLES);
  float ipc = (float) instr_cnt / (float) cycles_cnt;
  float mac_cycles = (float) mac / (float) cycles_cnt;

  printf("Number of Instructions: %d\nClock Cycles: %d\nIPC: %f\nMAC/cycle: %f\n\n", 
      instr_cnt, cycles_cnt, ipc, mac_cycles);
  return errors;
}



void check_function(int *errors) {

  printf("2D Convolution WINDOW=%d, DATA FORMAT Q%d.%d\n",FILT_WIN,8-FRACTIONARY_BITS,FRACTIONARY_BITS);

  // upper and lower bounds 
  unsigned int lb =1;           // does not compute first row (black board)
  unsigned int ub = IMG_ROW -1; // does not compute last row (black board)

  InitKernel(Kernel,FILT_WIN);
  InitData(In, IMG_DIM);
  InitZero(Out, IMG_DIM);


  pi_perf_stop(); // stop the performance counters
  pi_perf_reset();
  pi_perf_start();

  //convolution task
  Conv3x3(In, Out, IMG_ROW, lb, ub, IMG_COL, Kernel);

  pi_perf_stop();

  *errors = checkresult(Out, Gold_Out_Img, IMG_DIM);

}

// load kernel
void __attribute__ ((noinline)) InitKernel(uint8_t * __restrict__ Kernel, int size)
{
  int i;
  int n = size*size;
  for (i=0; i < n; i++) {
      Kernel[i] = Filter_Kern[i];
  }
}

// load input img
void __attribute__ ((noinline)) InitData(uint8_t * __restrict__ Img, int size)
{
  int i;

  for (i=0; i < size; i++) 
      Img[i] = In_Img[i];

}

// load initialize out to 0
void __attribute__ ((noinline)) InitZero(uint8_t * __restrict__ Img, int size)
{
  int i;

  for (i=0; i < size; i++) 
      Img[i] = 0;

}

int  __attribute__ ((noinline)) checkresult(uint8_t * __restrict__ Out, uint8_t * __restrict__ OutGold, int N)
{
  int i;
  int err = 0;

  for (i = 0; i<N; i++) {
    if (Out[i]!=OutGold[i]) {
#ifdef CONV2D_DEBUG
      printf("At index %d: Actual value: %x: Expected: %x\n", i, Out[i],  OutGold[i]);
#endif
      err++;
    }
  }
  return err;
}

void print_image(uint8_t* image, int N, int M) {
  for (int y = 0; y < M; y++) {
    for (int x = 0; x < N; x++)
      printf("%d,", (uint8_t) image[M * y + x]);

    printf("\n");
  }

  printf("\n");
}
