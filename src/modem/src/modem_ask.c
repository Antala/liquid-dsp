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
// modem_ask.c
//

// ASK scaling factors
#if LIQUID_FPM
#  define ASK2_ALPHA      Q(_one)
#  define ASK4_ALPHA      Q(_float_to_fixed)(1./sqrt(5))
#  define ASK8_ALPHA      Q(_float_to_fixed)(1./sqrt(21))
#  define ASK16_ALPHA     Q(_float_to_fixed)(1./sqrt(85))
#  define ASK32_ALPHA     Q(_float_to_fixed)(1./sqrt(341))
#  define ASK64_ALPHA     Q(_float_to_fixed)(1./sqrt(1365))
#  define ASK128_ALPHA    Q(_float_to_fixed)(1./sqrt(5461))
#  define ASK256_ALPHA    Q(_float_to_fixed)(1./sqrt(21845))
#else
#  define ASK2_ALPHA      (1.)
#  define ASK4_ALPHA      (1./sqrt(5))
#  define ASK8_ALPHA      (1./sqrt(21))
#  define ASK16_ALPHA     (1./sqrt(85))
#  define ASK32_ALPHA     (1./sqrt(341))
#  define ASK64_ALPHA     (1./sqrt(1365))
#  define ASK128_ALPHA    (1./sqrt(5461))
#  define ASK256_ALPHA    (1./sqrt(21845))
#endif

// create an ask (amplitude-shift keying) modem object
MODEM() MODEM(_create_ask)(unsigned int _bits_per_symbol)
{
    MODEM() q = (MODEM()) malloc( sizeof(struct MODEM(_s)) );

    MODEM(_init)(q, _bits_per_symbol);

    switch (q->M) {
    case 2:     q->data.ask.alpha = ASK2_ALPHA;   q->scheme = LIQUID_MODEM_ASK2;   break;
    case 4:     q->data.ask.alpha = ASK4_ALPHA;   q->scheme = LIQUID_MODEM_ASK4;   break;
    case 8:     q->data.ask.alpha = ASK8_ALPHA;   q->scheme = LIQUID_MODEM_ASK8;   break;
    case 16:    q->data.ask.alpha = ASK16_ALPHA;  q->scheme = LIQUID_MODEM_ASK16;  break;
    case 32:    q->data.ask.alpha = ASK32_ALPHA;  q->scheme = LIQUID_MODEM_ASK32;  break;
    case 64:    q->data.ask.alpha = ASK64_ALPHA;  q->scheme = LIQUID_MODEM_ASK64;  break;
    case 128:   q->data.ask.alpha = ASK128_ALPHA; q->scheme = LIQUID_MODEM_ASK128; break;
    case 256:   q->data.ask.alpha = ASK256_ALPHA; q->scheme = LIQUID_MODEM_ASK256; break;
    default:
#if 0
        // calculate alpha dynamically
        q->data.ask.alpha = expf(-0.70735 + 0.63653*q->m);
#else
        fprintf(stderr,"error: modem_create_ask(), cannot support ASK with m > 8\n");
        exit(1);
#endif
    }

    unsigned int k;
    for (k=0; k<(q->m); k++)
        q->ref[k] = (1<<k) * q->data.ask.alpha;

    q->modulate_func = &MODEM(_modulate_ask);
    q->demodulate_func = &MODEM(_demodulate_ask);

    // initialize soft-demodulation look-up table
    if (q->m >= 2 && q->m < 8)
        MODEM(_demodsoft_gentab)(q, 2);

    // reset modem and return
    MODEM(_reset)(q);
    return q;
}

// modulate ASK
void MODEM(_modulate_ask)(MODEM()      _q,
                          unsigned int _sym_in,
                          TC *         _y)
{
    // 'encode' input symbol (actually gray decoding)
    _sym_in = gray_decode(_sym_in);

    // modulate symbol
#if LIQUID_FPM
    _y[0].real = (2*(int)_sym_in - (int)(_q->M) + 1) * _q->data.ask.alpha;
    _y[0].imag = 0;
#else
    *_y = (2*(int)_sym_in - (int)(_q->M) + 1) * _q->data.ask.alpha;
#endif
}

// demodulate ASK
void MODEM(_demodulate_ask)(MODEM()        _q,
                            TC             _x,
                            unsigned int * _sym_out)
{
    // demodulate on linearly-spaced array
    unsigned int s;
    T res_i;
#if LIQUID_FPM
    MODEM(_demodulate_linear_array_ref)(_x.real,    _q->m, _q->ref, &s, &res_i);
#else
    MODEM(_demodulate_linear_array_ref)(crealf(_x), _q->m, _q->ref, &s, &res_i);
#endif

    // 'decode' output symbol (actually gray encoding)
    *_sym_out = gray_encode(s);

    // re-modulate symbol and store state
    MODEM(_modulate_ask)(_q, *_sym_out, &_q->x_hat);
    _q->r = _x;
}

