/*
 * Copyright (c) 2007 - 2020 Joseph Gaeddert
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

//
// square root approximation
//

#include <stdio.h>
#include <stdlib.h>

#include "liquidfpm.internal.h"

#define DEBUG_SQRT_NEWTON 0
#define DEBUG_SQRT_LOGEXP 0

// use iterative process via Newton's method
Q(_t) Q(_sqrt_newton)( Q(_t) _x, unsigned int _n)
{
    if (_x < 0) {
        fprintf(stderr,"warning: qxx_sqrt_newton(), x must be >= 0");
        return 0;
    }

    if (_x == 0)
        return 0;

    // initial guess: x0 = _x
    //Q(_t) x0 = _x;

    // initial guess: set x0 to 2^floor(msb_index/2)
    // NOTE: we need to compensate for the decimal position of q32
    Q(_t) x0 = 1 << (((liquid_msb_index(_x)-Q(_fracbits))/2)+Q(_fracbits));

    Q(_t) x1=0;

    unsigned int i;
    for (i=0; i<_n; i++) {
#if DEBUG_SQRT_NEWTON
        printf("%4u : %18.14f\n", i, Q(_fixed_to_float)(x0));
#endif
        x0 += (x0 == 0);    // ensure no division by zero
        x1 = (x0 + Q(_div)(_x,x0)) / 2;

        // break if change in this iteration is negligible
        if (abs(x1-x0) < 1)
            break;

        x0 = x1;
    }

    //return x1 << (Q(_fracbits)/2);
    return Q(_abs)(x1);
}

//
// use log2 and exp2 (look-up table) functions to approximate square root
//
Q(_t) Q(_sqrt_logexp_frac)(Q(_t) _x)
{
    if (_x < 0) {
        fprintf(stderr,"error: qxx_sqrt(), x must be > 0");
        exit(1);
    } else if (_x == 0) {
        return 0;
    }

    // TODO : ensure precision is maintained by splitting log2
    //        output into base and fractional components

    // compute logarithm
    Q(_t) log2x = Q(_log2_frac)(_x);

    // divide by 2 (logical bit shift)
    Q(_t) log2x_by_2 = log2x >> 1;

    // compute exponential
    Q(_t) qsqrt = Q(_exp2_frac)(log2x_by_2);

#if DEBUG_SQRT_LOGEXP
    printf("    x           = %12.10f\n", Q(_fixed_to_float)(_x));
    printf("    log2(x)     = %12.10f\n", Q(_fixed_to_float)(log2x));
    printf("    log2(x)/2   = %12.10f\n", Q(_fixed_to_float)(log2x_by_2));
    printf("    sqrt(x)     = %12.10f\n", Q(_fixed_to_float)(qsqrt));
#endif
    return qsqrt;
}

//
// use log2 and exp2 (shift|add) functions to approximate square root
//
Q(_t) Q(_sqrt_logexp_shiftadd)(Q(_t) _x,
                               unsigned int _precision)
{
    if (_x < 0) {
        fprintf(stderr,"error: qxx_sqrt(), x must be > 0");
        exit(1);
    } else if (_x == 0) {
        return 0;
    }

    // TODO : ensure precision is maintained by splitting log2
    //        output into base and fractional components

    // compute logarithm
    Q(_t) log2x = Q(_log2_shiftadd)(_x,_precision);

    // divide by 2 (logical bit shift)
    Q(_t) log2x_by_2 = log2x >> 1;

    // compute exponential
    Q(_t) qsqrt = Q(_exp2_shiftadd)(log2x_by_2,_precision);

#if DEBUG_SQRT_LOGEXP
    printf("    x           = %12.10f\n", Q(_fixed_to_float)(_x));
    printf("    log2(x)     = %12.10f\n", Q(_fixed_to_float)(log2x));
    printf("    log2(x)/2   = %12.10f\n", Q(_fixed_to_float)(log2x_by_2));
    printf("    sqrt(x)     = %12.10f\n", Q(_fixed_to_float)(qsqrt));
#endif
    return qsqrt;
}

