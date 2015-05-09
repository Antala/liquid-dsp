/*
 * Copyright (c) 2007 - 2015 Joseph Gaeddert
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
// exponential implementations
//

#include <stdio.h>
#include <stdlib.h>

#include "liquid.internal.h"
#include "liquidfpm.internal.h"

#define LIQUIDFPM_DEBUG_EXP2_FRAC 0

// natural logarithm
//    e^(x) = 2^(x * log2(e))
Q(_t) Q(_exp_frac)(Q(_t) _x)
{
    return Q(_exp2_frac)( Q(_mul)(_x,Q(_log2_e)) );
}

// natural logarithm (minus one)
Q(_t) Q(_expm1_frac)(Q(_t) _x)
{
    return Q(_exp_frac)(_x) - Q(_one);
}

// base-10 logarithm
//    10^(x) = 2^(x * log2(10))
Q(_t) Q(_exp10_frac)(Q(_t) _x)
{
    return Q(_exp2_frac)( Q(_mul)(_x,Q(_log2_10)) );
}

// power function
//    b^(x) = 2^(x * log2(b))
Q(_t) Q(_pow_frac)(Q(_t) _b,
                   Q(_t) _x)
{
    Q(_t) log2_b = Q(_log2_frac)(_b);
    return Q(_exp2_frac)( Q(_mul)(_x,log2_b) );
}

// computes 2^x using fractional table look-up; fast but not
// especially accurate.
//
// Procedure:
//  1. define x = s + r such that: r in [0,1), 's' is integer
//  2. compute 2^x as 2^(s+r) = (2^s)(2^r)
//      a. use look-up table for 2^r
//      b. use bit-wise shift for 2^s
//
Q(_t) Q(_exp2_frac)(Q(_t) _x)
{
    // base, fraction
    int b   = Q(_intpart)(_x);
    Q(_t) f = Q(_fracpart)(_x);

#if LIQUIDFPM_DEBUG_EXP2_FRAC
    printf("  x : %12.8f  > %d + %12.8f\n", Q(_fixed_to_float)(_x), 
                                            b,
                                            Q(_fixed_to_float)(f));
#endif

    // test if base is larger than data type
    // example: for a 32-bit fixed-point data type with 20
    //          fraction bits, 2^-20 is effectively 0
    if ( abs(b) >= Q(_fracbits) ) {
        if (b < 0) return 0;
    }

    // compute the fractional portion using look-up table,
    // stripping off only last 8 most-significant bits
    // (table is only 256 samples large)
    unsigned int index = (f >> (Q(_fracbits)-8)) & 0xff;
    Q(_t) y = Q(_exp2_frac_gentab)[index];
#if LIQUIDFPM_DEBUG_EXP2_FRAC
    printf("  i : %3u\n", index);
    printf("  y : %12.8f\n", Q(_fixed_to_float)(y));
#endif

    // post-shift the output by the integer amount (effectively
    // multiply by 2^-b)
    if (b > 0) y <<=  b;
    else       y >>= -b;
#if LIQUIDFPM_DEBUG_EXP2_FRAC
    printf("  y : %12.8f\n", Q(_fixed_to_float)(y));
#endif


    return y;
}

