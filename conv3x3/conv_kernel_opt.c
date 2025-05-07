/*
 * Copyright (C) 2015-2025 ETH Zurich and University of Bologna
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

/*
  Inputs, weights and outputs are represented in fixed-point Q1.7 unsigned format:
  this means that each integer in [0-255] represents a real value in the range [0.0-1.0)
  The relationship between the integer I and real R representations is given by

     R = I * 2^-FRACTIONARY_BITS

 */
#define FRACTIONARY_BITS 7
#define ROUNDBIT   (1 << (FRACTIONARY_BITS -1))
#define SATURATION 255

#define COLUMN_PARALLELISM 2

void __attribute__ ((noinline)) Conv3x3  (uint8_t * In_Img, uint8_t * Out_Img, int R, int lb, int ub, int C, uint8_t  * Kernel)
{
  int r, c, k, i, j, w, t;
  uint8_t coeff;
  uint8_t data;
  int S0, S1;

  // use three pointers sweeping three lines of the image together
  uint8_t *In_Img_ptr0 = (In_Img + (lb-1)*R);
  uint8_t *In_Img_ptr1 = (In_Img + lb*R);
  uint8_t *In_Img_ptr2 = (In_Img + (lb+1)*R);

  v4u coef_v00, coef_v01, coef_v02;
  v4u coef_v10, coef_v11, coef_v12;

  coef_v00 = *(v4u *) Kernel;
  coef_v01 = *(v4u *) (Kernel + 3);
  coef_v02 = *(v4u *) (Kernel + 6);
  coef_v00[3] = 0; coef_v01[3] = 0; coef_v02[3] = 0;

  coef_v10 = *(v4u *) (Kernel - 1);
  coef_v11 = *(v4u *) (Kernel + 2);
  coef_v12 = *(v4u *) (Kernel + 5);
  coef_v10[0] = 0; coef_v11[0] = 0; coef_v12[0] = 0;

  //image board is black
  for (r=lb; r < ub; r++) {
    for (c=1; c < C-1; c+=2) {

      S0 = 0;
      S1 = 0;
      t = r*R + c;

      v4u data_v0, data_v1, data_v2;

      // for data_v0, LD + LBU is faster than packing
      data_v0 = *(v4u *) In_Img_ptr0;
      data_v1 = *(v4u *) In_Img_ptr1;
      data_v2 = *(v4u *) In_Img_ptr2;

      /* VECTORIZED LOOP */
      S0 = __builtin_pulp_sdotup4(coef_v00, data_v0, S0);
      S0 = __builtin_pulp_sdotup4(coef_v01, data_v1, S0);
      S0 = __builtin_pulp_sdotup4(coef_v02, data_v2, S0);
      S1 = __builtin_pulp_sdotup4(coef_v10, data_v0, S1);
      S1 = __builtin_pulp_sdotup4(coef_v11, data_v1, S1);
      S1 = __builtin_pulp_sdotup4(coef_v12, data_v2, S1);
      // normalization
      S0 = __builtin_pulp_adduN(S0, 0, FRACTIONARY_BITS);
      S1 = __builtin_pulp_adduN(S1, 0, FRACTIONARY_BITS);
      // Saturation
      S0 = __builtin_pulp_clipu(S0, 0, SATURATION);
      S1 = __builtin_pulp_clipu(S1, 0, SATURATION);

      Out_Img[t]   = (uint8_t)(S0);
      Out_Img[t+1] = (uint8_t)(S1);

      In_Img_ptr0+=2; // next pixel on the right
      In_Img_ptr1+=2;
      In_Img_ptr2+=2;

    }
    In_Img_ptr0+=2; // jump to next line
    In_Img_ptr1+=2;
    In_Img_ptr2+=2;
  }
}
