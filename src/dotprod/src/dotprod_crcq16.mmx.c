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
// Fixed-point dot product (MMX)
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "liquid.internal.h"

// include proper SIMD extensions for x86 platforms
// NOTE: these pre-processor macros are defined in config.h

#if HAVE_MMINTRIN_H
#include <mmintrin.h>   // MMX
#endif

#if HAVE_XMMINTRIN_H
#include <xmmintrin.h>  // SSE
#endif

#if HAVE_EMMINTRIN_H
#include <emmintrin.h>  // SSE2
#endif

#if HAVE_PMMINTRIN_H
#include <pmmintrin.h>  // SSE3
#endif

#if HAVE_TMMINTRIN_H
#include <tmmintrin.h>  // SSSE3
#endif

#define DEBUG_DOTPROD_CRCQ16_MMX   0

#if DEBUG_DOTPROD_CRCQ16_MMX
void _mm_printq16_epi16(__m128i _v);
void _mm_printq16_epi32(__m128i _v);
#endif

// internal methods
void dotprod_crcq16_execute_mmx(dotprod_crcq16 _q, cq16_t * _x, cq16_t * _y);
void dotprod_crcq16_execute_mmx4(dotprod_crcq16 _q, cq16_t * _x, cq16_t * _y);

// alternate methods
void dotprod_crcq16_execute_mmx_packed(dotprod_crcq16 _q, cq16_t * _x, cq16_t * _y);

// basic dot product
//  _h      :   coefficients array [size: 1 x _n]
//  _x      :   input array [size: 1 x _n]
//  _n      :   input lengths
//  _y      :   output dot product
void dotprod_crcq16_run(q16_t *      _h,
                        cq16_t *     _x,
                        unsigned int _n,
                        cq16_t *     _y)
{
    // initialize accumulators (separate I/Q)
    q16_at ri = 0;
    q16_at rq = 0;

    unsigned int i;
    for (i=0; i<_n; i++) {
        ri += (q16_at)_h[i] * (q16_at)(_x[i].real);
        rq += (q16_at)_h[i] * (q16_at)(_x[i].imag);
    }

    // return result
    (*_y).real = (ri >> q16_fracbits);
    (*_y).imag = (rq >> q16_fracbits);
}

// basic dotproduct, unrolling loop
//  _h      :   coefficients array [size: 1 x _n]
//  _x      :   input array [size: 1 x _n]
//  _n      :   input lengths
//  _y      :   output dot product
void dotprod_crcq16_run4(q16_t *      _h,
                         cq16_t *     _x,
                         unsigned int _n,
                         cq16_t *     _y)
{
    // initialize accumulator (separate I/Q)
    q16_at ri = 0;
    q16_at rq = 0;

    // t = 4*(floor(_n/4))
    unsigned int t=(_n>>2)<<2; 

    // compute dotprod in groups of 4
    unsigned int i;
    for (i=0; i<t; i+=4) {
        // real component
        ri += (q16_at)_h[i  ] * (q16_at)(_x[i  ].real);
        ri += (q16_at)_h[i+1] * (q16_at)(_x[i+1].real);
        ri += (q16_at)_h[i+2] * (q16_at)(_x[i+2].real);
        ri += (q16_at)_h[i+3] * (q16_at)(_x[i+3].real);

        // imaginary component
        rq += (q16_at)_h[i  ] * (q16_at)(_x[i  ].imag);
        rq += (q16_at)_h[i+1] * (q16_at)(_x[i+1].imag);
        rq += (q16_at)_h[i+2] * (q16_at)(_x[i+2].imag);
        rq += (q16_at)_h[i+3] * (q16_at)(_x[i+3].imag);
    }

    // clean up remaining
    for ( ; i<_n; i++) {
        ri += (q16_at)_h[i] * (q16_at)(_x[i].real);
        rq += (q16_at)_h[i] * (q16_at)(_x[i].imag);
    }

    // return result
    (*_y).real = (ri >> q16_fracbits);
    (*_y).imag = (rq >> q16_fracbits);
}


//
// structured MMX dot product
//

struct dotprod_crcq16_s {
    q16_t * h;          // coefficients array
    unsigned int n;     // length
};

dotprod_crcq16 dotprod_crcq16_create(q16_t *      _h,
                                     unsigned int _n)
{
    dotprod_crcq16 q = (dotprod_crcq16)malloc(sizeof(struct dotprod_crcq16_s));
    q->n = _n;

    // allocate memory for coefficients, 16-byte aligned
    q->h = (q16_t*) _mm_malloc( 2*q->n*sizeof(q16_t), 16);

    // set coefficients, repeated
    //  h = { _h[0], _h[0], _h[1], _h[1], ... _h[n-1], _h[n-1]}
    unsigned int i;
    for (i=0; i<q->n; i++) {
        q->h[2*i+0] = _h[i];
        q->h[2*i+1] = _h[i];
    }

    // return object
    return q;
}

// re-create the structured dotprod object
dotprod_crcq16 dotprod_crcq16_recreate(dotprod_crcq16 _dp,
                                       q16_t *        _h,
                                       unsigned int   _n)
{
    // completely destroy and re-create dotprod object
    dotprod_crcq16_destroy(_dp);
    _dp = dotprod_crcq16_create(_h,_n);
    return _dp;
}


void dotprod_crcq16_destroy(dotprod_crcq16 _q)
{
    _mm_free(_q->h);
    free(_q);
}

void dotprod_crcq16_print(dotprod_crcq16 _q)
{
    printf("dotprod_crcq16:\n");
    unsigned int i;
    // print coefficients to screen, skipping odd entries (due
    // to repeated coefficients)
    for (i=0; i<_q->n; i++)
        printf("%3u : %12.8f\n", i, q16_fixed_to_float(_q->h[2*i]));
}

// 
void dotprod_crcq16_execute(dotprod_crcq16 _q,
                            cq16_t *       _x,
                            cq16_t *       _y)
{
    // switch based on size
    if (_q->n < 64) {
        dotprod_crcq16_execute_mmx(_q, _x, _y);
    } else {
        dotprod_crcq16_execute_mmx4(_q, _x, _y);
    }
}

// use MMX/SSE extensions
void dotprod_crcq16_execute_mmx(dotprod_crcq16 _q,
                                cq16_t *       _x,
                                cq16_t *       _y)
{
    // type cast input as real array
    q16_t * x = (q16_t*) _x;

    // input, coefficients, multiply/accumulate vectors
    __m128i v;   // input vector
    __m128i h;   // coefficients vector
    __m128i s;   // dot product
    __m128i sum = _mm_setzero_si128();

    // double effective length
    unsigned int n = 2*_q->n;
    
    // t = 8*(floor(_n/8))
    unsigned int t = (n >> 3) << 3;

    //
    unsigned int i;
    for (i=0; i<t; i+=8) {
        // load inputs into register (unaligned)
        // v: { x0.real, x0.imag, x1.real, x1.imag, x2.real, x2.imag, x3.real, x3.imag }
        v = _mm_loadu_si128( (__m128i*) (&x[i]) );
        // shuffle values in v:
        // v: { x0.real, x1.real, x0.imag, x1.imag, x2.real, x3.real, x2.imag, x3.imag }
        v = _mm_shufflehi_epi16(v, _MM_SHUFFLE(3,1,2,0));
        v = _mm_shufflelo_epi16(v, _MM_SHUFFLE(3,1,2,0));

        // load coefficients into register (aligned)
        h = _mm_load_si128( (__m128i*) (&_q->h[i]) );
        // shuffle values in h:
        // h: { h0.real, h1.real, h0.imag, h1.imag, h2.real, h3.real, h2.imag, h3.imag }
        h = _mm_shufflehi_epi16(h, _MM_SHUFFLE(3,1,2,0));
        h = _mm_shufflelo_epi16(h, _MM_SHUFFLE(3,1,2,0));

        // multiply and accumulate two 8x16-bit registers
        // into one 4x32-bit register
        s = _mm_madd_epi16(v, h);
       
        // parallel addition
        // NOTE: this addition contributes significantly to processing complexity
        sum = _mm_add_epi32(sum, s);
#if DEBUG_DOTPROD_CRCQ16_MMX
        printf("**i=%3u, t=%3u\n", i, t);
        printf("  v     :   "); _mm_printq16_epi16(v);   printf("\n");
        printf("  h     :   "); _mm_printq16_epi16(h);   printf("\n");
        printf("  s     :   "); _mm_printq16_epi32(s);   printf("\n");
        printf("  sum   :   "); _mm_printq16_epi32(sum); printf("\n");
#endif
    }

    // aligned output array: one 4x32-bit register
    q16_at w[4] __attribute__((aligned(16)));

#if 0 //HAVE_TMMINTRIN_H
    // TODO : check this
    // SSE3: fold down to single value using _mm_hadd_epi32()
    __m128i z = _mm_setzero_si128(); // set to zero
    sum = _mm_hadd_epi32(sum, z);
    sum = _mm_hadd_epi32(sum, z);
    sum = _mm_hadd_epi32(sum, z);
   
    // unload single (lower value)
    _mm_store_si128((__m128i*)w, sum);
    q16_at total = w[0];
#else
    // SSE2 and below: unload packed array and perform manual sum
    _mm_store_si128((__m128i*)w, sum);
    
    // add in-phase and quadrature components
    // NOTE: this addition contributes significantly to processing complexity
    w[0] += w[2];   // real
    w[1] += w[3];   // imag
#endif

    // cleanup (note: n is even)
    for (; i<n; i+=2) {
        w[0] += (q16_at)x[i  ] * (q16_at)(_q->h[i  ]);
        w[1] += (q16_at)x[i+1] * (q16_at)(_q->h[i+1]);
    }

    // set return value, shifting appropriately
    (*_y).real = (q16_t)(w[0] >> q16_fracbits);
    (*_y).imag = (q16_t)(w[1] >> q16_fracbits);
}

// use MMX/SSE extensions, unrolled loop
void dotprod_crcq16_execute_mmx4(dotprod_crcq16 _q,
                                 cq16_t *       _x,
                                 cq16_t *       _y)
{
    // type cast input as real array
    q16_t * x = (q16_t*) _x;

    // double effective length
    unsigned int n = 2*_q->n;
    
    // input, coefficients, multiply/accumulate vectors
    __m128i v0, v1, v2, v3; // input vector
    __m128i h0, h1, h2, h3; // coefficients vector
    __m128i s0, s1, s2, s3; // dot product

    // load zeros into sum registers
    __m128i sum0 = _mm_setzero_si128();
    __m128i sum1 = _mm_setzero_si128();
    __m128i sum2 = _mm_setzero_si128();
    __m128i sum3 = _mm_setzero_si128();

    // r = 8*floor(n/32) : vector size is 8 samples, unrolled by 4
    unsigned int r = (n >> 5) << 3;

    //
    unsigned int i;
    for (i=0; i<r; i+=8) {
        // load inputs into register (unaligned)
        // v: { x0.real, x0.imag, x1.real, x1.imag, x2.real, x2.imag, x3.real, x3.imag }
        v0 = _mm_loadu_si128( (__m128i*) (&x[4*i+ 0]) );
        v1 = _mm_loadu_si128( (__m128i*) (&x[4*i+ 8]) );
        v2 = _mm_loadu_si128( (__m128i*) (&x[4*i+16]) );
        v3 = _mm_loadu_si128( (__m128i*) (&x[4*i+24]) );

        // shuffle values in v:
        // v: { x0.real, x1.real, x0.imag, x1.imag, x2.real, x3.real, x2.imag, x3.imag }
        v0 = _mm_shufflehi_epi16(v0, _MM_SHUFFLE(3,1,2,0));
        v0 = _mm_shufflelo_epi16(v0, _MM_SHUFFLE(3,1,2,0));
        v1 = _mm_shufflehi_epi16(v1, _MM_SHUFFLE(3,1,2,0));
        v1 = _mm_shufflelo_epi16(v1, _MM_SHUFFLE(3,1,2,0));
        v2 = _mm_shufflehi_epi16(v2, _MM_SHUFFLE(3,1,2,0));
        v2 = _mm_shufflelo_epi16(v2, _MM_SHUFFLE(3,1,2,0));
        v3 = _mm_shufflehi_epi16(v3, _MM_SHUFFLE(3,1,2,0));
        v3 = _mm_shufflelo_epi16(v3, _MM_SHUFFLE(3,1,2,0));

        // load coefficients into register (aligned)
        h0 = _mm_load_si128( (__m128i*) (&_q->h[4*i+ 0]) );
        h1 = _mm_load_si128( (__m128i*) (&_q->h[4*i+ 8]) );
        h2 = _mm_load_si128( (__m128i*) (&_q->h[4*i+16]) );
        h3 = _mm_load_si128( (__m128i*) (&_q->h[4*i+24]) );

        // shuffle values in h:
        // h: { h0.real, h1.real, h0.imag, h1.imag, h2.real, h3.real, h2.imag, h3.imag }
        h0 = _mm_shufflehi_epi16(h0, _MM_SHUFFLE(3,1,2,0));
        h0 = _mm_shufflelo_epi16(h0, _MM_SHUFFLE(3,1,2,0));
        h1 = _mm_shufflehi_epi16(h1, _MM_SHUFFLE(3,1,2,0));
        h1 = _mm_shufflelo_epi16(h1, _MM_SHUFFLE(3,1,2,0));
        h2 = _mm_shufflehi_epi16(h2, _MM_SHUFFLE(3,1,2,0));
        h2 = _mm_shufflelo_epi16(h2, _MM_SHUFFLE(3,1,2,0));
        h3 = _mm_shufflehi_epi16(h3, _MM_SHUFFLE(3,1,2,0));
        h3 = _mm_shufflelo_epi16(h3, _MM_SHUFFLE(3,1,2,0));

        // multiply and accumulate two 8x16-bit registers
        // into one 4x32-bit register
        s0 = _mm_madd_epi16(v0, h0);
        s1 = _mm_madd_epi16(v1, h1);
        s2 = _mm_madd_epi16(v2, h2);
        s3 = _mm_madd_epi16(v3, h3);
       
        // parallel addition
        // NOTE: this addition contributes significantly to processing complexity
        sum0 = _mm_add_epi32(sum0, s0);
        sum1 = _mm_add_epi32(sum1, s1);
        sum2 = _mm_add_epi32(sum2, s2);
        sum3 = _mm_add_epi32(sum3, s3);
    }

    // fold down into single 4-element register
    sum0 = _mm_add_epi32(sum0, sum1);
    sum2 = _mm_add_epi32(sum2, sum3);
    sum0 = _mm_add_epi32(sum0, sum2);

    // aligned output array: one 4x32-bit register
    q16_at w[4] __attribute__((aligned(16)));

    // SSE2 and below: unload packed array and perform manual sum
    _mm_store_si128((__m128i*)w, sum0);
    
    // add in-phase and quadrature components
    w[0] += w[2];   // real
    w[1] += w[3];   // imag

    // cleanup (note: n is even)
    for (i=4*r; i<n; i+=2) {
        w[0] += (q16_at)x[i  ] * (q16_at)(_q->h[i  ]);
        w[1] += (q16_at)x[i+1] * (q16_at)(_q->h[i+1]);
    }

    // set return value, shifting appropriately
    (*_y).real = (q16_t)(w[0] >> q16_fracbits);
    (*_y).imag = (q16_t)(w[1] >> q16_fracbits);
}


//
// alternate methods
//

// use MMX/SSE extensions
void dotprod_crcq16_execute_mmx_packed(dotprod_crcq16 _q,
                                       cq16_t *       _x,
                                       cq16_t *       _y)
{
    // type cast input as real array
    q16_t * x = (q16_t*) _x;

    // input, coefficients, multiply/accumulate vectors
    __m128i v;   // input vector
    __m128i h;   // coefficients vector
    __m128i sl;  // dot product (lo)
    __m128i sh;  // dot product (hi)
    __m128i tl;  // unpacked (lo)
    __m128i th;  // unpacked (hi)
    __m128i sum = _mm_setzero_si128();

    // double effective length
    unsigned int n = 2*_q->n;
    
    // t = 8*(floor(_n/8))
    unsigned int t = (n >> 3) << 3;

    //
    unsigned int i;
    for (i=0; i<t; i+=8) {
        // load inputs into register (unaligned)
        // v: { x0.real, x0.imag, x1.real, x1.imag, x2.real, x2.imag, x3.real, x3.imag }
        v = _mm_loadu_si128( (__m128i*) (&x[i]) );

        // load coefficients into register (aligned)
        h = _mm_load_si128( (__m128i*) (&_q->h[i]) );

        // multiply two 8x16-bit registers into two 4x32-bit registers
        sl = _mm_mullo_epi16(v, h); // multiply, packing lower 16 bits of 32-bit result
        sh = _mm_mulhi_epi16(v, h); // multiply, packing upper 16 bits of 32-bit result

        // unpack bytes (re-align 
        tl = _mm_unpacklo_epi16(sl, sh); // [x0.real*h0, x0.imag*h0, x1.real*h1, x1.imag*h1]
        th = _mm_unpackhi_epi16(sl, sh); // [x2.real*h2, x2.imag*h2, x3.real*h3, x3.imag*h3]
       
        // parallel addition
        sum = _mm_add_epi32(sum, tl);
        sum = _mm_add_epi32(sum, th);
    }

    // aligned output array: one 4x32-bit register
    q16_at w[4] __attribute__((aligned(16)));

    // unload packed array and perform manual sum
    _mm_store_si128((__m128i*)w, sum);
    
    // add in-phase and quadrature components
    w[0] += w[2];   // real
    w[1] += w[3];   // imag

    // cleanup (note: n is even)
    for (; i<n; i+=2) {
        w[0] += (q16_at)x[i  ] * (q16_at)(_q->h[i  ]);
        w[1] += (q16_at)x[i+1] * (q16_at)(_q->h[i+1]);
    }

    // set return value, shifting appropriately
    (*_y).real = (q16_t)(w[0] >> q16_fracbits);
    (*_y).imag = (q16_t)(w[1] >> q16_fracbits);
}

// 
// debugging functions
//
#if DEBUG_DOTPROD_CRCQ16_MMX
void _mm_printq16_epi16(__m128i _v) {
    q16_t v[8] __attribute__((aligned(16)));
    _mm_store_si128((__m128i*)v, _v);
    printf("{");
    unsigned int i;
#if 1
    for (i=0; i<8; i++) printf(" %8.4f", q16_fixed_to_float(v[i]));
#else
    for (i=0; i<8; i++) printf(" %8d", v[i]);
#endif
    printf("}");
}

void _mm_printq16_epi32(__m128i _v) {
    int32_t v[4] __attribute__((aligned(16)));
    _mm_store_si128((__m128i*)v, _v);

    printf("{");
    unsigned int i;
#if 1
    for (i=0; i<4; i++) printf(" %17.8f", q16_fixed_to_float(v[i] >> q16_fracbits));
#else
    for (i=0; i<4; i++) printf(" %17d", v[i]);
#endif
    printf("}");
}
#endif

