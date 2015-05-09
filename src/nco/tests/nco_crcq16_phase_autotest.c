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

#include <stdlib.h>
#include <complex.h>
#include "autotest/autotest.h"
#include "liquid.h"

// autotest helper function
//  _theta  :   input phase
//  _cos    :   expected output: cos(_theta)
//  _sin    :   expected output: sin(_theta)
//  _type   :   NCO type (e.g. LIQUID_NCO)
//  _tol    :   error tolerance
void nco_crcq16_phase_test(float _theta,
                           float _cos,
                           float _sin,
                           int   _type,
                           float _tol)
{
    // create object
    nco_crcq16 nco = nco_crcq16_create(_type);

    // set phase
    nco_crcq16_set_phase(nco, q16_angle_float_to_fixed(_theta));

    // compute cosine and sine outputs
    q16_t c;    // cosine output
    q16_t s;    // sine output
    nco_crcq16_sincos(nco, &s, &c);

    // run tests
    if (liquid_autotest_verbose) {
        printf("sincos(%10.7f) = {%10.7f, %10.7f}, expected {%10.7f, %10.7f}\n",
                _theta,
                q16_fixed_to_float(s),
                q16_fixed_to_float(c),
                _sin,
                _cos);
    }
    CONTEND_DELTA( q16_fixed_to_float(c), _cos, _tol );
    CONTEND_DELTA( q16_fixed_to_float(s), _sin, _tol );

    // destroy object
    nco_crcq16_destroy(nco);
}


// test floating point precision nco phase
void autotest_nco_crcq16_phase()
{
    // error tolerance (higher for NCO)
    float tol = 0.02f;

    nco_crcq16_phase_test(-6.283185307f,  1.000000000f,  0.000000000f, LIQUID_NCO, tol);
    nco_crcq16_phase_test(-6.195739393f,  0.996179042f,  0.087334510f, LIQUID_NCO, tol);
    nco_crcq16_phase_test(-5.951041106f,  0.945345356f,  0.326070787f, LIQUID_NCO, tol);
    nco_crcq16_phase_test(-5.131745978f,  0.407173250f,  0.913350943f, LIQUID_NCO, tol);
    nco_crcq16_phase_test(-4.748043551f,  0.035647016f,  0.999364443f, LIQUID_NCO, tol);
    nco_crcq16_phase_test(-3.041191113f, -0.994963998f, -0.100232943f, LIQUID_NCO, tol);
    nco_crcq16_phase_test(-1.947799864f, -0.368136099f, -0.929771914f, LIQUID_NCO, tol);
    nco_crcq16_phase_test(-1.143752030f,  0.414182352f, -0.910193924f, LIQUID_NCO, tol);
    nco_crcq16_phase_test(-1.029377689f,  0.515352252f, -0.856978446f, LIQUID_NCO, tol);
    nco_crcq16_phase_test(-0.174356887f,  0.984838307f, -0.173474811f, LIQUID_NCO, tol);
    nco_crcq16_phase_test(-0.114520496f,  0.993449692f, -0.114270338f, LIQUID_NCO, tol);
    nco_crcq16_phase_test( 0.000000000f,  1.000000000f,  0.000000000f, LIQUID_NCO, tol);
    nco_crcq16_phase_test( 1.436080000f,  0.134309213f,  0.990939471f, LIQUID_NCO, tol);
    nco_crcq16_phase_test( 2.016119855f, -0.430749878f,  0.902471353f, LIQUID_NCO, tol);
    nco_crcq16_phase_test( 2.996498473f, -0.989492293f,  0.144585621f, LIQUID_NCO, tol);
    nco_crcq16_phase_test( 3.403689755f, -0.965848729f, -0.259106603f, LIQUID_NCO, tol);
    nco_crcq16_phase_test( 3.591162483f, -0.900634128f, -0.434578148f, LIQUID_NCO, tol);
    nco_crcq16_phase_test( 5.111428476f,  0.388533479f, -0.921434607f, LIQUID_NCO, tol);
    nco_crcq16_phase_test( 5.727585681f,  0.849584319f, -0.527452828f, LIQUID_NCO, tol);
    nco_crcq16_phase_test( 6.283185307f,  1.000000000f, -0.000000000f, LIQUID_NCO, tol);
}

// test floating point precision vco phase
void autotest_vco_crcq16_phase()
{
    // error tolerance
    float tol = expf(-sqrtf(q16_fracbits));

    nco_crcq16_phase_test(-6.283185307f,  1.000000000f,  0.000000000f, LIQUID_VCO, tol);
    nco_crcq16_phase_test(-6.195739393f,  0.996179042f,  0.087334510f, LIQUID_VCO, tol);
    nco_crcq16_phase_test(-5.951041106f,  0.945345356f,  0.326070787f, LIQUID_VCO, tol);
    nco_crcq16_phase_test(-5.131745978f,  0.407173250f,  0.913350943f, LIQUID_VCO, tol);
    nco_crcq16_phase_test(-4.748043551f,  0.035647016f,  0.999364443f, LIQUID_VCO, tol);
    nco_crcq16_phase_test(-3.041191113f, -0.994963998f, -0.100232943f, LIQUID_VCO, tol);
    nco_crcq16_phase_test(-1.947799864f, -0.368136099f, -0.929771914f, LIQUID_VCO, tol);
    nco_crcq16_phase_test(-1.143752030f,  0.414182352f, -0.910193924f, LIQUID_VCO, tol);
    nco_crcq16_phase_test(-1.029377689f,  0.515352252f, -0.856978446f, LIQUID_VCO, tol);
    nco_crcq16_phase_test(-0.174356887f,  0.984838307f, -0.173474811f, LIQUID_VCO, tol);
    nco_crcq16_phase_test(-0.114520496f,  0.993449692f, -0.114270338f, LIQUID_VCO, tol);
    nco_crcq16_phase_test( 0.000000000f,  1.000000000f,  0.000000000f, LIQUID_VCO, tol);
    nco_crcq16_phase_test( 1.436080000f,  0.134309213f,  0.990939471f, LIQUID_VCO, tol);
    nco_crcq16_phase_test( 2.016119855f, -0.430749878f,  0.902471353f, LIQUID_VCO, tol);
    nco_crcq16_phase_test( 2.996498473f, -0.989492293f,  0.144585621f, LIQUID_VCO, tol);
    nco_crcq16_phase_test( 3.403689755f, -0.965848729f, -0.259106603f, LIQUID_VCO, tol);
    nco_crcq16_phase_test( 3.591162483f, -0.900634128f, -0.434578148f, LIQUID_VCO, tol);
    nco_crcq16_phase_test( 5.111428476f,  0.388533479f, -0.921434607f, LIQUID_VCO, tol);
    nco_crcq16_phase_test( 5.727585681f,  0.849584319f, -0.527452828f, LIQUID_VCO, tol);
    nco_crcq16_phase_test( 6.283185307f,  1.000000000f, -0.000000000f, LIQUID_VCO, tol);
}

