/*
 * Copyright (c) 2007 - 2014 Joseph Gaeddert
 *
 * This file is part of liquid.
 *
 * liquid is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * liquid is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with liquid.  If not, see <http://www.gnu.org/licenses/>.
 */

//
// Automatic gain control
//

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "liquid.internal.h"

// default AGC loop bandwidth
#if defined LIQUID_FIXED
#  define AGC_DEFAULT_BW    Q(_float_to_fixed)(1e-2f)
#  define AGC_GAIN_MIN      Q(_min)
#else
#  define AGC_DEFAULT_BW    (1e-2f)
#  define AGC_GAIN_MIN      (1e-16f)
#endif

// forward declaration of internal methods
void AGC(_estimate_input_energy)(AGC() _q, TC _x);
void AGC(_limit_gain)(AGC() _q);
void AGC(_update_auto_squelch)(AGC() _q, T _rssi);
void AGC(_execute_squelch)(AGC() _q);


// agc structure object
struct AGC(_s) {
    // gain variables
    T g;            // current gain value

    // gain control loop filter parameters
    float bandwidth;// bandwidth-time constant
    T alpha;        // feed-back gain

    // signal level estimate
    T y2_prime;     // filtered output signal energy estimate

    // AGC locked flag
    int is_locked;
};

// create agc object
AGC() AGC(_create)(void)
{
    // create object and initialize to default parameters
    AGC() _q = (AGC()) malloc(sizeof(struct AGC(_s)));

    // initialize bandwidth
    AGC(_set_bandwidth)(_q, AGC_DEFAULT_BW);

    // reset object
    AGC(_reset)(_q);

    // return object
    return _q;
}

// destroy agc object, freeing all internally-allocated memory
void AGC(_destroy)(AGC() _q)
{
    // free main object memory
    free(_q);
}

// print agc object internals
void AGC(_print)(AGC() _q)
{
#if defined LIQUID_FIXED
    printf("agc [rssi: %12.4f dB]:\n", Q(_fixed_to_float)( AGC(_get_rssi)(_q)) );
#else
    printf("agc [rssi: %12.4f dB]:\n", AGC(_get_rssi)(_q));
#endif
}

// reset agc object's internal state
void AGC(_reset)(AGC() _q)
{
    // reset gain estimate
    _q->g = 1.0f;

    // reset signal level estimate
    _q->y2_prime = 1.0f;

    // unlock gain control
    AGC(_unlock)(_q);
}

// execute automatic gain control loop
//  _q      :   agc object
//  _x      :   input sample
//  _y      :   output sample
void AGC(_execute)(AGC() _q,
                   TC    _x,
                   TC *  _y)
{
    // apply gain to input sample
#if defined LIQUID_FIXED && TC_COMPLEX == 1
    *_y = CQ(_mul_scalar)(_x, _q->g);
#elif defined LIQUID_FIXED && TC_COMPLEX == 0
    *_y = Q(_mul)(_x, _q->g);
#else
    *_y = _x * _q->g;
#endif

    // compute output signal energy
#if defined LIQUID_FIXED && TC_COMPLEX == 1
    T y2 = CQ(_cabs2)( *_y );
#elif defined LIQUID_FIXED && TC_COMPLEX == 0
    T y2 = Q(_mul)( *_y, *_y );
#else
    T y2 = crealf( (*_y)*conjf(*_y) );
#endif

    // smooth energy estimate using single-pole low-pass filter
#if defined LIQUID_FIXED
    // TODO: store internal 'beta' as 1-alpha
    // TODO: validate this method
    _q->y2_prime = Q(_mul)( Q(_one)-_q->alpha, _q->y2_prime ) +
                   Q(_mul)(         _q->alpha,           y2 );
#else
    _q->y2_prime = (1.0-_q->alpha)*_q->y2_prime + _q->alpha*y2;
#endif

    // return if locked
    if (_q->is_locked)
        return;

    // update gain according to output energy
#if defined LIQUID_FIXED
    T log_y2_prime    =  Q(_log)( _q->y2_prime );
    T gain_exp        = -Q(_mul)( _q->alpha >> 2 , log_y2_prime );
    T gain_correction =  Q(_exp)( gain_exp );
    _q->g             =  Q(_mul)( _q->g, gain_correction );
#else
    _q->g *= expf( -0.5f*_q->alpha*logf(_q->y2_prime) );
#endif
}

// execute automatic gain control on block of samples
//  _q      : automatic gain control object
//  _x      : input data array, [size: _n x 1]
//  _n      : number of input, output samples
//  _y      : output data array, [size: _n x 1]
void AGC(_execute_block)(AGC()        _q,
                         TC *         _x,
                         unsigned int _n,
                         TC *         _y)
{
    unsigned int i;
    for (i=0; i<_n; i++)
        AGC(_execute)(_q, _x[i], &_y[i]);
}

// lock agc
void AGC(_lock)(AGC() _q)
{
    _q->is_locked = 1;
}

// unlock agc
void AGC(_unlock)(AGC() _q)
{
    _q->is_locked = 0;
}

// get agc loop bandwidth
float AGC(_get_bandwidth)(AGC() _q)
{
    return _q->bandwidth;
}

// set agc loop bandwidth
//  _q      :   agc object
//  _BT     :   bandwidth
void AGC(_set_bandwidth)(AGC() _q,
                         float _bt)
{
    // check to ensure bandwidth is reasonable
    if ( _bt < 0 ) {
        fprintf(stderr,"error: agc_%s_set_bandwidth(), bandwidth must be positive\n", EXTENSION_FULL);
        exit(-1);
    } else if ( _bt > 1.0f ) {
        fprintf(stderr,"error: agc_%s_set_bandwidth(), bandwidth must less than 1.0\n", EXTENSION_FULL);
        exit(-1);
    }

    // set internal bandwidth
    _q->bandwidth = _bt;

    // compute filter coefficient based on bandwidth
    _q->alpha = _q->bandwidth;
}

// get estimated signal level (linear)
float AGC(_get_signal_level)(AGC() _q)
{
    return 1.0f / _q->g;
}

// set estimated signal level (linear)
void AGC(_set_signal_level)(AGC() _q,
                            float _signal_level)
{
    // check to ensure signal level is reasonable
    if ( _signal_level <= 0 ) {
        fprintf(stderr,"error: agc_%s_set_signal_level(), bandwidth must be greater than zero\n", EXTENSION_FULL);
        exit(-1);
    }

    // set internal gain appropriately
    _q->g = 1.0f / _signal_level;

    // reset internal output signal level
    _q->y2_prime = 1.0f;
}

// get estimated signal level (dB)
float AGC(_get_rssi)(AGC() _q)
{
    return -20*log10(_q->g);
}

// set estimated signal level (dB)
void AGC(_set_rssi)(AGC() _q,
                    float _rssi)
{
    // set internal gain appropriately
    _q->g = powf(10.0f, -_rssi/20.0f);

    // ensure resulting gain is not arbitrarily low
    if (_q->g < AGC_GAIN_MIN)
        _q->g = AGC_GAIN_MIN;

    // reset internal output signal level
    _q->y2_prime = 1.0f;
}

// get internal gain
float AGC(_get_gain)(AGC() _q)
{
    return _q->g;
}

// set internal gain
void AGC(_set_gain)(AGC() _q,
                    float _gain)
{
    // check to ensure gain is reasonable
    if ( _gain <= 0 ) {
        fprintf(stderr,"error: agc_%s_set_gain(), gain must be greater than zero\n", EXTENSION_FULL);
        exit(-1);
    }

    // set internal gain appropriately
    _q->g = _gain;
}

// initialize internal gain on input array
//  _q      : automatic gain control object
//  _x      : input data array, [size: _n x 1]
//  _n      : number of input, output samples
void AGC(_init)(AGC()        _q,
                TC *         _x,
                unsigned int _n)
{
    // ensure number of samples is greater than zero
    if ( _n == 0 ) {
        fprintf(stderr,"error: agc_%s_init(), number of samples must be greater than zero\n", EXTENSION_FULL);
        exit(-1);
    }

#if defined LIQUID_FIXED && TC_COMPLEX == 1
    // compute sum squares on input
    float x2 = Q(_fixed_to_float)( liquid_sumsqcq16(_x, _n) );
#elif defined LIQUID_FIXED && TC_COMPLEX == 0
    float x2 = Q(_fixed_to_float)( liquid_sumsqq16(_x, _n) );
#else
    // compute sum squares on input
    // TODO: use vector methods for this
    unsigned int i;
    float x2 = 0;
    for (i=0; i<_n; i++)
        x2 += crealf( _x[i]*conjf(_x[i]) );
#endif

    // compute RMS level and ensure result is positive
    x2 = sqrtf( x2 / (float) _n ) + 1e-16f;

    // set internal gain based on estimated signal level
    AGC(_set_signal_level)(_q, x2);
}

