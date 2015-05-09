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
// modem_qam.c
//

// 'Square' QAM scaling factors
#if 0
#define QAM4_ALPHA      (1./sqrt(2))
#define QAM8_ALPHA      (1./sqrt(6))
#define QAM16_ALPHA     (1./sqrt(10))
#define QAM32_ALPHA     (1./sqrt(20))
#define QAM64_ALPHA     (1./sqrt(42))
#define QAM128_ALPHA    (1./sqrt(82))
#define QAM256_ALPHA    (1./sqrt(170))
#define QAM1024_ALPHA   (1./sqrt(682))
#define QAM4096_ALPHA   (1./sqrt(2730))
#endif

// Rectangular QAM scaling factors
#if LIQUID_FPM
#  define QAM4_ALPHA      Q(_float_to_fixed)(1./sqrt(2))
#  define QAM8_ALPHA      Q(_float_to_fixed)(1./sqrt(6))
#  define QAM16_ALPHA     Q(_float_to_fixed)(1./sqrt(10))
#  define QAM32_ALPHA     Q(_float_to_fixed)(1./sqrt(26))
#  define QAM64_ALPHA     Q(_float_to_fixed)(1./sqrt(42))
#  define QAM128_ALPHA    Q(_float_to_fixed)(1./sqrt(106))
#  define QAM256_ALPHA    Q(_float_to_fixed)(1./sqrt(170))
#  define QAM512_ALPHA    Q(_float_to_fixed)(1./sqrt(426))
#  define QAM1024_ALPHA   Q(_float_to_fixed)(1./sqrt(682))
#  define QAM2048_ALPHA   Q(_float_to_fixed)(1./sqrt(1706))
#  define QAM4096_ALPHA   Q(_float_to_fixed)(1./sqrt(2730))
#else
#  define QAM4_ALPHA      (1./sqrt(2))
#  define QAM8_ALPHA      (1./sqrt(6))
#  define QAM16_ALPHA     (1./sqrt(10))
#  define QAM32_ALPHA     (1./sqrt(26))
#  define QAM64_ALPHA     (1./sqrt(42))
#  define QAM128_ALPHA    (1./sqrt(106))
#  define QAM256_ALPHA    (1./sqrt(170))
#  define QAM512_ALPHA    (1./sqrt(426))
#  define QAM1024_ALPHA   (1./sqrt(682))
#  define QAM2048_ALPHA   (1./sqrt(1706))
#  define QAM4096_ALPHA   (1./sqrt(2730))
#endif

// create a qam (quaternary amplitude-shift keying) modem object
MODEM() MODEM(_create_qam)(unsigned int _bits_per_symbol)
{
    if (_bits_per_symbol < 1 ) {
        fprintf(stderr,"error: modem_create_qam(), modem must have at least 2 bits/symbol\n");
        exit(1);
    }

    MODEM() q = (MODEM()) malloc( sizeof(struct MODEM(_s)) );

    MODEM(_init)(q, _bits_per_symbol);

    if (q->m % 2) {
        // rectangular qam
        q->data.qam.m_i = (q->m + 1) >> 1;
        q->data.qam.m_q = (q->m - 1) >> 1;
    } else {
        // square qam
        q->data.qam.m_i = q->m >> 1;
        q->data.qam.m_q = q->m >> 1;
    }

    q->data.qam.M_i = 1 << (q->data.qam.m_i);
    q->data.qam.M_q = 1 << (q->data.qam.m_q);

    //assert(q->data.qam.m_i + q->data.qam.m_q == q->m);
    //assert(q->data.qam.M_i * q->data.qam.M_q == q->M);

    switch (q->M) {
    case 4:    q->data.qam.alpha = QAM4_ALPHA;    q->scheme = LIQUID_MODEM_QAM4;   break;
    case 8:    q->data.qam.alpha = QAM8_ALPHA;    q->scheme = LIQUID_MODEM_QAM8;   break;
    case 16:   q->data.qam.alpha = QAM16_ALPHA;   q->scheme = LIQUID_MODEM_QAM16;  break;
    case 32:   q->data.qam.alpha = QAM32_ALPHA;   q->scheme = LIQUID_MODEM_QAM32;  break;
    case 64:   q->data.qam.alpha = QAM64_ALPHA;   q->scheme = LIQUID_MODEM_QAM64;  break;
    case 128:  q->data.qam.alpha = QAM128_ALPHA;  q->scheme = LIQUID_MODEM_QAM128; break;
    case 256:  q->data.qam.alpha = QAM256_ALPHA;  q->scheme = LIQUID_MODEM_QAM256; break;
#if 0
    case 512:  q->data.qam.alpha = QAM512_ALPHA;     break;
    case 1024: q->data.qam.alpha = QAM1024_ALPHA;    break;
    case 2048: q->data.qam.alpha = QAM2048_ALPHA;    break;
    case 4096: q->data.qam.alpha = QAM4096_ALPHA;    break;
    default:
        // calculate alpha dynamically
        // NOTE: this is only an approximation
        q->data.qam.alpha = sqrtf(2.0f / (T)(q->M) );
#else
    default:
        fprintf(stderr,"error: modem_create_qam(), cannot support QAM with m > 8\n");
        exit(1);
#endif
    }

    unsigned int k;
    for (k=0; k<(q->m); k++)
        q->ref[k] = (1<<k) * q->data.qam.alpha;

    q->modulate_func = &MODEM(_modulate_qam);
    q->demodulate_func = &MODEM(_demodulate_qam);

    // initialize symbol map
    q->symbol_map = (TC*)malloc(q->M*sizeof(TC));
    MODEM(_init_map)(q);
    q->modulate_using_map = 1;

    // initialize soft-demodulation look-up table
    if      (q->m == 3) MODEM(_demodsoft_gentab)(q, 3);
    else if (q->m >= 4) MODEM(_demodsoft_gentab)(q, 4);

    // reset and return
    MODEM(_reset)(q);
    return q;
}

// modulate QAM
void MODEM(_modulate_qam)(MODEM()      _q,
                          unsigned int _sym_in,
                          TC *         _y)
{
    // in-phase symbol: most-significant m_i bits
    unsigned int s_i = _sym_in >> _q->data.qam.m_q;

    // quadrature symbol: leas-significant m_q bits
    unsigned int s_q = _sym_in & ( (1<<_q->data.qam.m_q)-1 );

    // 'encode' symbols (actually gray decoding)
    s_i = gray_decode(s_i);
    s_q = gray_decode(s_q);

    // compute output sample
#if LIQUID_FPM
    _y[0].real = ( 2*(int)s_i - (int)(_q->data.qam.M_i) + 1) * _q->data.qam.alpha;
    _y[0].imag = ( 2*(int)s_q - (int)(_q->data.qam.M_q) + 1) * _q->data.qam.alpha;
#else
    *_y = (2*(int)s_i - (int)(_q->data.qam.M_i) + 1) * _q->data.qam.alpha +
          (2*(int)s_q - (int)(_q->data.qam.M_q) + 1) * _q->data.qam.alpha * _Complex_I;
#endif
}

// demodulate QAM
void MODEM(_demodulate_qam)(MODEM()        _q,
                            TC             _x,
                            unsigned int * _sym_out)
{
    // demodulate linearly-spaced arrays
    unsigned int s_i;   // in-phase symbol
    unsigned int s_q;   // quadrature symbol
    T res_i;            // in-phase residual
    T res_q;            // quadrature residual

#if LIQUID_FPM
    MODEM(_demodulate_linear_array_ref)(_x.real, _q->data.qam.m_i, _q->ref, &s_i, &res_i);
    MODEM(_demodulate_linear_array_ref)(_x.imag, _q->data.qam.m_q, _q->ref, &s_q, &res_q);
#else
    MODEM(_demodulate_linear_array_ref)(crealf(_x), _q->data.qam.m_i, _q->ref, &s_i, &res_i);
    MODEM(_demodulate_linear_array_ref)(cimagf(_x), _q->data.qam.m_q, _q->ref, &s_q, &res_q);
#endif

    // 'decode' output symbol (actually gray encoding)
    s_i = gray_encode(s_i);
    s_q = gray_encode(s_q);
    *_sym_out = ( s_i << _q->data.qam.m_q ) + s_q;

    // re-modulate symbol (subtract residual) and store state
#if LIQUID_FPM
    _q->x_hat.real = _x.real - res_i;
    _q->x_hat.imag = _x.imag - res_q;
#else
    _q->x_hat = _x - (res_i + _Complex_I*res_q);
#endif
    _q->r = _x;
}

