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
// sin/cos using CORDIC
//

#include <stdio.h>
#include <stdlib.h>

#include "liquid.internal.h"
#include "liquidfpm.internal.h"

// debug sin/cos computation using CORDIC method
#define DEBUG_SINCOS_CORDIC 0

// 
// sin/cos CORDIC
// 

unsigned int Q(_quadrant_cordic)(Q(_t) _theta)
{
    // Ensure theta >= 0
    // Because the qtype representation of angles is [-2*pi,2*pi] we can guarantee
    // adding 2*pi will make theta non-negative
    if ( _theta < 0 )
        _theta += Q(_2pi);

    // _theta now has the form : 0QQB BBBB BBBx xxxx xxxx xxxx xxxx xxxx
    // Q : 2-bit quadrant
    // B : 8-bit phase resolution
    // x : remaining 21 bits phase resolution, ignored

    // Extract quadrant
    //   1. Shift by 29, len(x) + len(B)
    //   2. Mask with 0x0003 to ensure 2-bit value
    return (_theta >> (Q(_bits)-3)) & 0x00000003;
}

Q(_t) Q(_sin_cordic)(Q(_t) _theta, unsigned int _n)
{
    Q(_t) sine;
    Q(_t) cosine;
    Q(_sincos_cordic)(_theta,&sine,&cosine,_n);
    return sine;
}

Q(_t) Q(_cos_cordic)(Q(_t) _theta, unsigned int _n)
{
    Q(_t) sine;
    Q(_t) cosine;
    Q(_sincos_cordic)(_theta,&sine,&cosine,_n);
    return cosine;
}

void Q(_sincos_cordic)(Q(_t) _theta,
                       Q(_t) * _sin,
                       Q(_t) * _cos,
                       unsigned int _n)
{
    // constrain input angle to be in [-pi, pi]
    // TODO : determine more efficient way of doing this
    if (_theta >= Q(_pi))
        _theta -= Q(_2pi);
    else if (_theta <= -Q(_pi))
        _theta += Q(_2pi);

    // invert phase: constrain to [-pi/2,pi/2]
    // This is necessary because the cordic will only converge
    // if -r < _theta < r, where
    //      r = sum(k=0, infty) ( arctan(2^-k) )
    //      r ~ 1.743286620472340
    // Because r > pi/2 (1.57079632), constraining the phase
    // to be in [-pi/2,pi/2] will ensure convergence.
    int invert=0;
    if (_theta > Q(_pi_by_2)) {
        _theta -= Q(_pi);
        invert = 1;
    } else if( _theta < -Q(_pi_by_2)) {
        _theta += Q(_pi);
        invert = 1;
    } else {
        invert = 0;
    }

#if DEBUG_SINCOS_CORDIC
    printf("  theta : %12.8f\n", Q(_angle_fixed_to_float)(_theta));
#endif

    Q(_t) x = Q(_cordic_k_inv); // TODO : initialize with cordic_Kinv_tab[_n-1];
    Q(_t) y = 0;
    Q(_t) z = _theta;
    Q(_t) d,tx,ty,tz;
    unsigned int i;
    unsigned int n = _n > Q(_bits) ? Q(_bits) : _n;

#if DEBUG_SINCOS_CORDIC
    printf("   n           x            y            z         -d*An\n");
    printf("init %12.8f %12.8f %12.8f %12.8f\n",
            Q(_fixed_to_float)(x),
            Q(_fixed_to_float)(y),
            Q(_fixed_to_float)(z),
            0.0);
#endif
    for (i=0; i<n; i++) {
        d = ( z>=0 ) ? 0 : -1;
        // d = z >> 31;

        tx = x - ((y>>i)^d)-d;
        ty = y + ((x>>i)^d)-d;
        tz = z - ((Q(_cordic_Ak_tab)[i]^d)-d);
        x = tx;
        y = ty;
        z = tz;
#if DEBUG_SINCOS_CORDIC
        printf("%4u %12.8f %12.8f %12.8f %12.8f\n",
            i,
            Q(_fixed_to_float)(x),
            Q(_fixed_to_float)(y),
            Q(_fixed_to_float)(z),
            Q(_fixed_to_float)(Q(_cordic_Ak_tab)[i])*(z>=0?1.0:-1.0));
#endif
    }

    // negate values if phase has been inverted
    if (!invert) {
        // returning values without negation
        *_cos =  x;
        *_sin =  y;
    } else {
        // negating values due to phase inversion
        *_cos = -x;
        *_sin = -y;
    }
}

