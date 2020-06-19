/*
 * Copyright (c) 2007 - 2020 Joseph Gaeddert
 *
 * This file is part of liquid-fpm.
 *
 * liquid-fpm is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * liquid-fpm is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with liquid-fpm.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <math.h>

#include "liquidfpm.internal.h"
#include "autotest/autotest.h"

// API definition macro; helper function to keep code base small
#define LIQUIDFPM_AUTOTEST_SQRT_API(Q)                              \
                                                                    \
/* sqrt using Newton's method */                                    \
void Q(_test_sqrt_newton)(float        _xf,                         \
                          unsigned int _precision,                  \
                          float        _tol)                        \
{                                                                   \
    /* convert to fixed-point */                                    \
    Q(_t) x = Q(_float_to_fixed)(_xf);                              \
                                                                    \
    /* execute operation */                                         \
    Q(_t) y = Q(_sqrt_newton)(x, _precision);                       \
    float yf = sqrtf(_xf);                                          \
                                                                    \
    /* convert to floating-point */                                 \
    float ytest = Q(_fixed_to_float)(y);                            \
                                                                    \
    if (liquid_autotest_verbose) {                                  \
        printf("  sqrt(%12.8f) = %12.8f (%12.8f), e=%12.8f\n",      \
                      _xf,      ytest,  yf,       ytest-yf);        \
    }                                                               \
                                                                    \
    /* run comparison */                                            \
    CONTEND_DELTA(ytest, yf, _tol);                                 \
}                                                                   \
                                                                    \
/* sqrt using log|exp look-up table method */                       \
void Q(_test_sqrt_logexp_frac)(float        _xf,                    \
                               float        _tol)                   \
{                                                                   \
    /* convert to fixed-point */                                    \
    Q(_t) x = Q(_float_to_fixed)(_xf);                              \
                                                                    \
    /* execute operation */                                         \
    Q(_t) y = Q(_sqrt_logexp_frac)(x);                              \
    float yf = sqrtf(_xf);                                          \
                                                                    \
    /* convert to floating-point */                                 \
    float ytest = Q(_fixed_to_float)(y);                            \
                                                                    \
    if (liquid_autotest_verbose) {                                  \
        printf("  sqrt(%12.8f) = %12.8f (%12.8f), e=%12.8f\n",      \
                      _xf,      ytest,  yf,       ytest-yf);        \
    }                                                               \
                                                                    \
    /* run comparison */                                            \
    CONTEND_DELTA(ytest, yf, _tol);                                 \
}                                                                   \
                                                                    \
/* sqrt using log|exp shift|add method */                           \
void Q(_test_sqrt_logexp_shiftadd)(float        _xf,                \
                                   unsigned int _precision,         \
                                   float        _tol)               \
{                                                                   \
    /* convert to fixed-point */                                    \
    Q(_t) x = Q(_float_to_fixed)(_xf);                              \
                                                                    \
    /* execute operation */                                         \
    Q(_t) y = Q(_sqrt_logexp_shiftadd)(x, _precision);              \
    float yf = sqrtf(_xf);                                          \
                                                                    \
    /* convert to floating-point */                                 \
    float ytest = Q(_fixed_to_float)(y);                            \
                                                                    \
    if (liquid_autotest_verbose) {                                  \
        printf("  sqrt(%12.8f) = %12.8f (%12.8f), e=%12.8f\n",      \
                      _xf,      ytest,  yf,       ytest-yf);        \
    }                                                               \
                                                                    \
    /* run comparison */                                            \
    CONTEND_DELTA(ytest, yf, _tol);                                 \
}

// define autotest API
LIQUIDFPM_AUTOTEST_SQRT_API(LIQUIDFPM_MANGLE_Q32)
LIQUIDFPM_AUTOTEST_SQRT_API(LIQUIDFPM_MANGLE_Q16)

//
// q16
//

void autotest_q16_sqrt_newton()
{
    unsigned int precision = 16;    // precision
    unsigned int num_steps = 77;    // number of steps in test
    float xmin = 0.0f;
    float xmax = q16_fixed_to_float(q16_max) * 0.99f;
    float dx   = (xmax - xmin) / ((float)num_steps);
    float tol  = expf(-sqrtf(q16_fracbits));

    unsigned int i;
    float x = xmin;
    for (i=0; i<num_steps; i++) {
        // run test
        q16_test_sqrt_newton(x, precision, tol);

        // increment input parameter
        x += dx;
    }
    // test a few specific small values
    q16_test_sqrt_newton(q16_fixed_to_float(  1), precision, tol);
    q16_test_sqrt_newton(q16_fixed_to_float(  2), precision, tol);
    q16_test_sqrt_newton(q16_fixed_to_float(  9), precision, tol);
    q16_test_sqrt_newton(q16_fixed_to_float(141), precision, tol);
}

void autotest_q16_sqrt_logexp_frac()
{
    unsigned int num_steps = 77;    // number of steps in test
    float xmin = 0.0f;
    float xmax = q16_fixed_to_float(q16_max) * 0.99f;
    float dx   = (xmax - xmin) / ((float)num_steps);
    float tol  = expf(-sqrtf(q16_fracbits));

    unsigned int i;
    float x = xmin;
    for (i=0; i<num_steps; i++) {
        // run test
        q16_test_sqrt_logexp_frac(x, tol);

        // increment input parameter
        x += dx;
    }
}

void autotest_q16_sqrt_logexp_shiftadd()
{
    unsigned int precision = 16;    // precision
    unsigned int num_steps = 77;    // number of steps in test
    float xmin = 0.0f;
    float xmax = q16_fixed_to_float(q16_max) * 0.99f;
    float dx   = (xmax - xmin) / ((float)num_steps);
    float tol  = expf(-sqrtf(q16_fracbits));

    unsigned int i;
    float x = xmin;
    for (i=0; i<num_steps; i++) {
        // run test
        q16_test_sqrt_logexp_shiftadd(x, precision, tol);

        // increment input parameter
        x += dx;
    }
}

//
// q32
//

void autotest_q32_sqrt_newton()
{
    unsigned int precision = 16;    // precision
    unsigned int num_steps = 77;    // number of steps in test
    float xmin = 0.0f;
    float xmax = q32_fixed_to_float(q32_max) * 0.99f;
    float dx   = (xmax - xmin) / ((float)num_steps);
    float tol  = expf(-sqrtf(q32_fracbits));

    unsigned int i;
    float x = xmin;
    for (i=0; i<num_steps; i++) {
        // run test
        q32_test_sqrt_newton(x, precision, tol);

        // increment input parameter
        x += dx;
    }
}

void autotest_q32_sqrt_logexp_frac()
{
    unsigned int num_steps = 77;    // number of steps in test
    float xmin = 0.0f;
    float xmax = q32_fixed_to_float(q32_max) * 0.99f;
    float dx   = (xmax - xmin) / ((float)num_steps);
    //float tol  = expf(-sqrtf(q32_fracbits));
    float tol  = 0.03f;

    unsigned int i;
    float x = xmin;
    for (i=0; i<num_steps; i++) {
        // run test
        q32_test_sqrt_logexp_frac(x, tol);

        // increment input parameter
        x += dx;
    }
}

void autotest_q32_sqrt_logexp_shiftadd()
{
    unsigned int precision = 16;    // precision
    unsigned int num_steps = 77;    // number of steps in test
    float xmin = 0.0f;
    float xmax = q32_fixed_to_float(q32_max) * 0.99f;
    float dx   = (xmax - xmin) / ((float)num_steps);
    float tol  = expf(-sqrtf(q32_fracbits));

    unsigned int i;
    float x = xmin;
    for (i=0; i<num_steps; i++) {
        // run test
        q32_test_sqrt_logexp_shiftadd(x, precision, tol);

        // increment input parameter
        x += dx;
    }
}

//
// cube root tests (very basic)
//

void autotest_q16_cbrt_newton()
{
    unsigned int precision = 16;    // precision
    float tol  = expf(-sqrtf(q16_fracbits));

    float xf;
    q16_t x, cbrt_x;

    // sanity check
    xf     = 1.0f;
    x      = q16_float_to_fixed(xf);
    cbrt_x = q16_cbrt_newton(x, precision);
    CONTEND_DELTA( q16_fixed_to_float(cbrt_x), cbrtf(xf), tol );

    // basic test
    xf     = 0.5f;
    x      = q16_float_to_fixed(xf);
    cbrt_x = q16_cbrt_newton(x, precision);
    CONTEND_DELTA( q16_fixed_to_float(cbrt_x), cbrtf(xf), tol );

    // negative test
    xf     = -0.5f;
    x      = q16_float_to_fixed(xf);
    cbrt_x = q16_cbrt_newton(x, precision);
    CONTEND_DELTA( q16_fixed_to_float(cbrt_x), cbrtf(xf), tol );

}

void autotest_q32_cbrt_newton()
{
    unsigned int precision = 16;    // precision
    float tol  = expf(-sqrtf(q32_fracbits));

    float xf;
    q32_t x, cbrt_x;

    // sanity check
    xf     = 1.0f;
    x      = q32_float_to_fixed(xf);
    cbrt_x = q32_cbrt_newton(x, precision);
    CONTEND_DELTA( q32_fixed_to_float(cbrt_x), cbrtf(xf), tol );

    // basic test
    xf     = 0.5f;
    x      = q32_float_to_fixed(xf);
    cbrt_x = q32_cbrt_newton(x, precision);
    CONTEND_DELTA( q32_fixed_to_float(cbrt_x), cbrtf(xf), tol );

    // negative test
    xf     = -0.5f;
    x      = q32_float_to_fixed(xf);
    cbrt_x = q32_cbrt_newton(x, precision);
    CONTEND_DELTA( q32_fixed_to_float(cbrt_x), cbrtf(xf), tol );

}

