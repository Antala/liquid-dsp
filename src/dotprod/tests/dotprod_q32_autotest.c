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

#include <string.h>

#include "autotest/autotest.h"
#include "liquid.internal.h"

// helper function prototypes
void dotprod_rrrq32_test(float *      _hf,
                         float *      _xf,
                         unsigned int _n,
                         float        _tol);

void dotprod_crcq32_test(float *         _hf,
                         float complex * _xf,
                         unsigned int    _n,
                         float           _tol);

void dotprod_cccq32_test(float complex * _hf,
                         float complex * _xf,
                         unsigned int    _n,
                         float           _tol);

// 
// AUTOTEST: basic dot product (rrr)
//
void autotest_dotprod_rrrq32_basic()
{
    float tol = expf(-sqrtf(q32_fracbits));

    float hf[16] = {
    -0.050565, -0.952580,  0.274320,  1.232400, 
     1.268200,  0.565770,  0.800830,  0.923970, 
     0.517060, -0.530340, -0.378550, -1.127100, 
     1.123100, -1.006000, -1.483800, -0.062007
    };

    float xf[16] = {
    -0.384280, -0.812030,  0.156930,  1.919500, 
     0.564580, -0.123610, -0.138640,  0.004984, 
    -1.100200, -0.497620,  0.089977, -1.745500, 
     0.463640,  0.592100,  1.150000, -1.225400
    };

    // run check
    dotprod_rrrq32_test(hf, xf, 16, tol);
}

// 
// AUTOTEST: basic dot product (crc)
//
void autotest_dotprod_crcq32_basic()
{
    float tol = expf(-sqrtf(q32_fracbits));

    float hf[16] = {
      5.5375e-02,  -6.5857e-01,  -1.7657e+00,   7.7444e-01, 
      8.0730e-01,  -5.1340e-01,  -9.3437e-02,  -5.6301e-01, 
     -6.6480e-01,  -2.1673e+00,   9.0269e-01,   3.5284e+00, 
     -9.7835e-01,  -6.9512e-01,  -1.2958e+00,   1.1628e+00
    };

    float complex xf[16] = {
      1.3164e+00+  5.4161e-01*_Complex_I,   1.8295e-01+ -9.0284e-02*_Complex_I, 
      1.3487e+00+ -1.8148e+00*_Complex_I,  -7.4696e-01+ -4.1792e-01*_Complex_I, 
     -9.0551e-01+ -4.4294e-01*_Complex_I,   6.0591e-01+ -1.5383e+00*_Complex_I, 
     -7.5393e-01+ -3.5691e-01*_Complex_I,  -4.5733e-01+  1.1926e-01*_Complex_I, 
     -1.4744e-01+ -4.7676e-02*_Complex_I,  -1.2422e+00+ -2.0213e+00*_Complex_I, 
      3.3208e-02+ -1.3756e+00*_Complex_I,  -4.8573e-01+  1.0977e+00*_Complex_I, 
      1.5053e+00+  2.1141e-01*_Complex_I,  -8.4062e-01+ -1.0211e+00*_Complex_I, 
     -1.3932e+00+ -4.8491e-01*_Complex_I,  -1.4234e+00+  2.0333e-01*_Complex_I
    };

    // run check
    dotprod_crcq32_test(hf, xf, 16, tol);
}

// 
// AUTOTEST: basic dot product (ccc)
//
void autotest_dotprod_cccq32_basic()
{
    float tol = expf(-sqrtf(q32_fracbits));

    float complex hf[16] = {
      0.17702709 +   1.38978455*_Complex_I,  0.91294148 +   0.39217381*_Complex_I,
     -0.80607338 +   0.76477512*_Complex_I,  0.05099755 +  -0.87350051*_Complex_I,
      0.44513826 +  -0.49490569*_Complex_I,  0.14754967 +   2.04349962*_Complex_I,
      1.07246623 +   1.08146290*_Complex_I, -1.14028088 +   1.83380899*_Complex_I,
      0.38105361 +  -0.45591846*_Complex_I,  0.32605401 +   0.34440081*_Complex_I,
     -0.05477144 +   0.60832595*_Complex_I,  1.81667523 +  -1.12238075*_Complex_I,
     -0.87190497 +   1.10743858*_Complex_I,  1.30921403 +   1.24438643*_Complex_I,
      0.55524695 +  -1.94931519*_Complex_I, -0.87191170 +   0.91693119*_Complex_I,
    };

    float complex xf[16] = {
     -0.19591953 +  -0.93229692*_Complex_I,  0.17150376 +   0.56165114*_Complex_I,
      1.58354529 +  -0.50696037*_Complex_I,  1.40929619 +   0.87868803*_Complex_I,
     -0.75505072 +  -0.30867372*_Complex_I, -0.09821367 +  -0.73949106*_Complex_I,
      0.03785571 +   0.72763665*_Complex_I, -1.20262636 +  -0.88838102*_Complex_I,
      0.23323685 +   0.12456235*_Complex_I,  0.34593736 +   0.02529594*_Complex_I,
      0.33669564 +   0.39064649*_Complex_I, -2.45003867 +  -0.54862205*_Complex_I,
     -1.64870707 +   0.33444473*_Complex_I, -0.92284477 +  -2.45121397*_Complex_I,
      0.24852918 +  -0.62409860*_Complex_I, -0.87039907 +   0.90921212*_Complex_I,
    };

    // run check
    dotprod_cccq32_test(hf, xf, 16, tol);
}


// 
// AUTOTEST: compare structured object to ordinal computation (rrr)
//
void autotest_dotprod_rrrq32_varying_size()
{
    float tol = expf(-sqrtf(q32_fracbits));

    // run many, many tests
    unsigned int i;
    unsigned int n;
    for (n=1; n<=128; n++) {
        // create arrays for random values
        float h[n];
        float x[n];

        // initialize values, ensuring result is within range of
        // fixed-point type
        float sigma = 10.0f;
        float result = 0.0f;
        do {
            for (i=0; i<n; i++) {
                h[i] = (2*randf() - 1)*sigma;
                x[i] = (2*randf() - 1)*sigma;
            }
            dotprod_rrrf_run(h,x,n,&result);
            sigma *= 0.9f;
        } while (fabsf(result) > 0.9*q32_fixed_to_float(q32_max));

        // run test
        dotprod_rrrq32_test(h, x, n, tol);
    }
}

// 
// AUTOTEST: compare structured object to ordinal computation (crc)
//
void autotest_dotprod_crcq32_varying_size()
{
    float tol = expf(-sqrtf(q32_fracbits));

    // run many, many tests
    unsigned int i;
    unsigned int n;
    for (n=1; n<=128; n++) {
        // create arrays for random values
        float h[n];
        float complex x[n];

        // initialize values, ensuring result is within range of
        // fixed-point type
        float sigma = 10.0f;
        float complex result = 0.0f;
        do {
            for (i=0; i<n; i++) {
                h[i] = (2*randf() - 1)*sigma;
                x[i] = (2*randf() - 1)*sigma + _Complex_I*(2*randf()-1)*sigma;
            }
            dotprod_crcf_run(h,x,n,&result);
            sigma *= 0.9f;
        } while (fabsf(crealf(result)) > 0.9*q32_fixed_to_float(q32_max) ||
                 fabsf(cimagf(result)) > 0.9*q32_fixed_to_float(q32_max));

        // run test
        dotprod_crcq32_test(h, x, n, tol);
    }
}


// 
// AUTOTEST: compare structured object to ordinal computation (ccc)
//
void autotest_dotprod_cccq32_varying_size()
{
    float tol = expf(-sqrtf(q32_fracbits));

    // run many, many tests
    unsigned int i;
    unsigned int n;
    for (n=1; n<=128; n++) {
        // create arrays for random values
        float complex h[n];
        float complex x[n];

        // initialize values, ensuring result is within range of
        // fixed-point type
        float sigma = 10.0f;
        float complex result = 0.0f;
        do {
            for (i=0; i<n; i++) {
                h[i] = (2*randf() - 1)*sigma + _Complex_I*(2*randf()-1)*sigma;
                x[i] = (2*randf() - 1)*sigma + _Complex_I*(2*randf()-1)*sigma;
            }
            dotprod_cccf_run(h,x,n,&result);
            sigma *= 0.9f;
        } while (fabsf(crealf(result)) > 0.9*q32_fixed_to_float(q32_max) ||
                 fabsf(cimagf(result)) > 0.9*q32_fixed_to_float(q32_max));

        // run test
        dotprod_cccq32_test(h, x, n, tol);
    }
}






// 
// helper functions to keep code base small
//
void dotprod_rrrq32_test(float *      _hf,
                         float *      _xf,
                         unsigned int _n,
                         float        _tol)
{
    unsigned int i;

    // convert to fixed-point arrays, back to float (compensate for
    // roundoff error)
    q32_t  h[_n],  x[_n];
    float hf[_n], xf[_n];
    for (i=0; i<_n; i++) {
        h[i] = q32_float_to_fixed(_hf[i]);
        x[i] = q32_float_to_fixed(_xf[i]);

        hf[i] = q32_fixed_to_float(h[i]);
        xf[i] = q32_fixed_to_float(x[i]);
    }

    // compute floating point result
    float yf_test;
    dotprod_rrrf_run(hf, xf, _n, &yf_test);

    // compute fixed-point result using dotprod_rrrq32_run()
    q32_t y0;
    dotprod_rrrq32_run(h, x, _n, &y0);
    float y0f = q32_fixed_to_float(y0);

    // compute fixed-point result using dotprod_rrrq32_run4()
    q32_t y1;
    dotprod_rrrq32_run(h, x, _n, &y1);
    float y1f = q32_fixed_to_float(y1);

    // compute fixed-point result using dotprod_rrrq32 object
    q32_t y2;
    dotprod_rrrq32 q = dotprod_rrrq32_create(h,_n);
    dotprod_rrrq32_execute(q, x, &y2);
    dotprod_rrrq32_destroy(q);
    float y2f = q32_fixed_to_float(y2);

    if (liquid_autotest_verbose) {
        printf("testing dotprod_rrrq32(%u)...\n", _n);
        printf("  dotprod_rrrq32_run    : %12.8f (expected %12.8f), e=%12.8f, tol=%12.8f\n", y0f, yf_test, y0f-yf_test, _tol);
        printf("  dotprod_rrrq32_run4   : %12.8f (expected %12.8f), e=%12.8f, tol=%12.8f\n", y1f, yf_test, y1f-yf_test, _tol);
        printf("  dotprod_rrrq32 object : %12.8f (expected %12.8f), e=%12.8f, tol=%12.8f\n", y2f, yf_test, y2f-yf_test, _tol);
    }

    // run checks
    CONTEND_DELTA(y0f, yf_test, _tol);
    CONTEND_DELTA(y1f, yf_test, _tol);
    CONTEND_DELTA(y2f, yf_test, _tol);
}

void dotprod_crcq32_test(float *         _hf,
                         float complex * _xf,
                         unsigned int    _n,
                         float           _tol)
{
    unsigned int i;

    // convert to fixed-point arrays, back to float (compensate for
    // roundoff error)
    q32_t h[_n];
    cq32_t x[_n];
    float hf[_n];
    float complex xf[_n];
    for (i=0; i<_n; i++) {
        h[i] =  q32_float_to_fixed(_hf[i]);
        x[i] = cq32_float_to_fixed(_xf[i]);

        hf[i] =  q32_fixed_to_float(h[i]);
        xf[i] = cq32_fixed_to_float(x[i]);
    }

    // compute floating point result
    float complex yf_test;
    dotprod_crcf_run(hf, xf, _n, &yf_test);

    // compute fixed-point result using dotprod_rrrq32_run()
    cq32_t y0;
    dotprod_crcq32_run(h, x, _n, &y0);
    float complex y0f = cq32_fixed_to_float(y0);

    // compute fixed-point result using dotprod_rrrq32_run4()
    cq32_t y1;
    dotprod_crcq32_run(h, x, _n, &y1);
    float complex y1f = cq32_fixed_to_float(y1);

    // compute fixed-point result using dotprod_rrrq32 object
    cq32_t y2;
    dotprod_crcq32 q = dotprod_crcq32_create(h,_n);
    dotprod_crcq32_execute(q, x, &y2);
    dotprod_crcq32_destroy(q);
    float complex y2f = cq32_fixed_to_float(y2);

    if (liquid_autotest_verbose) {
        printf("testing dotprod_crcq32(%u)...\n", _n);
        printf("  dotprod_crcq32_run    : %8.4f+j%8.4f (expected %8.4f+j%8.4f)\n", crealf(y0f), cimagf(y0f), crealf(yf_test), cimagf(yf_test));
        printf("  dotprod_crcq32_run4   : %8.4f+j%8.4f (expected %8.4f+j%8.4f)\n", crealf(y1f), cimagf(y1f), crealf(yf_test), cimagf(yf_test));
        printf("  dotprod_crcq32 object : %8.4f+j%8.4f (expected %8.4f+j%8.4f)\n", crealf(y2f), cimagf(y2f), crealf(yf_test), cimagf(yf_test));
    }

    // run checks
    CONTEND_DELTA( crealf(y0f), crealf(yf_test), _tol);
    CONTEND_DELTA( cimagf(y0f), cimagf(yf_test), _tol);

    CONTEND_DELTA( crealf(y1f), crealf(yf_test), _tol);
    CONTEND_DELTA( cimagf(y1f), cimagf(yf_test), _tol);

    CONTEND_DELTA( crealf(y2f), crealf(yf_test), _tol);
    CONTEND_DELTA( cimagf(y2f), cimagf(yf_test), _tol);
}

void dotprod_cccq32_test(float complex * _hf,
                         float complex * _xf,
                         unsigned int    _n,
                         float           _tol)
{
    unsigned int i;

    // convert to fixed-point arrays, back to float (compensate for
    // roundoff error)
    cq32_t h[_n];
    cq32_t x[_n];
    float complex hf[_n];
    float complex xf[_n];
    for (i=0; i<_n; i++) {
        h[i] = cq32_float_to_fixed(_hf[i]);
        x[i] = cq32_float_to_fixed(_xf[i]);

        hf[i] = cq32_fixed_to_float(h[i]);
        xf[i] = cq32_fixed_to_float(x[i]);
    }

    // compute floating point result
    float complex yf_test;
    dotprod_cccf_run(hf, xf, _n, &yf_test);

    // compute fixed-point result using dotprod_rrrq32_run()
    cq32_t y0;
    dotprod_cccq32_run(h, x, _n, &y0);
    float complex y0f = cq32_fixed_to_float(y0);

    // compute fixed-point result using dotprod_rrrq32_run4()
    cq32_t y1;
    dotprod_cccq32_run(h, x, _n, &y1);
    float complex y1f = cq32_fixed_to_float(y1);

    // compute fixed-point result using dotprod_rrrq32 object
    cq32_t y2;
    dotprod_cccq32 q = dotprod_cccq32_create(h,_n);
    dotprod_cccq32_execute(q, x, &y2);
    dotprod_cccq32_destroy(q);
    float complex y2f = cq32_fixed_to_float(y2);

    if (liquid_autotest_verbose) {
        printf("testing dotprod_cccq32(%u)...\n", _n);
        printf("  dotprod_cccq32_run    : %8.4f+j%8.4f (expected %8.4f+j%8.4f)\n", crealf(y0f), cimagf(y0f), crealf(yf_test), cimagf(yf_test));
        printf("  dotprod_cccq32_run4   : %8.4f+j%8.4f (expected %8.4f+j%8.4f)\n", crealf(y1f), cimagf(y1f), crealf(yf_test), cimagf(yf_test));
        printf("  dotprod_cccq32 object : %8.4f+j%8.4f (expected %8.4f+j%8.4f)\n", crealf(y2f), cimagf(y2f), crealf(yf_test), cimagf(yf_test));
    }

    // run checks
    CONTEND_DELTA( crealf(y0f), crealf(yf_test), _tol);
    CONTEND_DELTA( cimagf(y0f), cimagf(yf_test), _tol);

    CONTEND_DELTA( crealf(y1f), crealf(yf_test), _tol);
    CONTEND_DELTA( cimagf(y1f), cimagf(yf_test), _tol);

    CONTEND_DELTA( crealf(y2f), crealf(yf_test), _tol);
    CONTEND_DELTA( cimagf(y2f), cimagf(yf_test), _tol);
}

