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
// modem_arb.c
//

// create arbitrary digital modem object from floating-point array
MODEM() MODEM(_create_arbitrary)(float complex * _table,
                                 unsigned int    _M)
{
    // strip out bits/symbol
    unsigned int m = liquid_nextpow2(_M);
    if ( (1<<m) != _M ) {
        // TODO : eventually support non radix-2 constellation sizes
        fprintf(stderr,"error: modem_create_arbitrary(), input constellation size must be power of 2\n");
        exit(1);
    }

    // create arbitrary modem object, not initialized
    MODEM() q = MODEM(_create_arb)(m);

    // initialize object from table
    MODEM(_arb_init)(q, _table, _M);

    // return object
    return q;
}


// create an arbitrary modem object
MODEM() MODEM(_create_arb)(unsigned int _bits_per_symbol)
{
    MODEM() q = (MODEM()) malloc( sizeof(struct MODEM(_s)) );
    q->scheme = LIQUID_MODEM_ARB;

    MODEM(_init)(q, _bits_per_symbol);

    q->M = q->M;
    q->symbol_map = (TC*) calloc( q->M, sizeof(TC) );

    q->modulate_func   = &MODEM(_modulate_arb);
    q->demodulate_func = &MODEM(_demodulate_arb);

    return q;
}

// modulate arbitrary modem type
void MODEM(_modulate_arb)(MODEM()      _q,
                          unsigned int _sym_in,
                          TC *         _y)
{
    if (_sym_in >= _q->M) {
        fprintf(stderr,"error: modulate_arb(), input symbol exceeds maximum\n");
        exit(1);
    }

    // map sample directly to output
    *_y = _q->symbol_map[_sym_in]; 
}

// demodulate arbitrary modem type
void MODEM(_demodulate_arb)(MODEM()        _q,
                            TC             _x,
                            unsigned int * _sym_out)
{
    //printf("modem_demodulate_arb() invoked with I=%d, Q=%d\n", x);
    
    // search for symbol nearest to received sample
    unsigned int i;
    unsigned int s=0;
#if LIQUID_FPM
    int64_t d_min = 0;
#else
    T d_min = 0;
#endif

    for (i=0; i<_q->M; i++) {
        // compute distance from received symbol to constellation point
#if LIQUID_FPM
        int64_t di = _x.real - _q->symbol_map[i].real;
        int64_t dq = _x.imag - _q->symbol_map[i].imag;
        int64_t d = di*di + dq*dq;
#else
        T d = cabsf(_x - _q->symbol_map[i]);
#endif

        // retain symbol with minimum distance
        if ( i==0 || d < d_min ) {
            d_min = d;
            s = i;
        }
    }

    // set output symbol
    *_sym_out = s;

    // re-modulate symbol and store state
    MODEM(_modulate_arb)(_q, *_sym_out, &_q->x_hat);
    _q->r = _x;
}

// create a V.29 modem object (4 bits/symbol)
MODEM() MODEM(_create_V29)()
{
    MODEM() q = MODEM(_create_arb)(4);
    MODEM(_arb_init)(q,(float complex*)modem_arb_V29,16);
    return q;
}

// create an arb16opt (optimal 16-qam) modem object
MODEM() MODEM(_create_arb16opt)()
{
    MODEM() q = MODEM(_create_arb)(4);
    MODEM(_arb_init)(q,(float complex*)modem_arb16opt,16);
    return q;
}

// create an arb32opt (optimal 32-qam) modem object
MODEM() MODEM(_create_arb32opt)()
{
    MODEM() q = MODEM(_create_arb)(5);
    MODEM(_arb_init)(q,(float complex*)modem_arb32opt,32);
    return q;
}

// create an arb64opt (optimal 64-qam) modem object
MODEM() MODEM(_create_arb64opt)()
{
    MODEM() q = MODEM(_create_arb)(6);
    MODEM(_arb_init)(q,(float complex*)modem_arb64opt,64);
    return q;
}

// create an arb128opt (optimal 128-qam) modem object
MODEM() MODEM(_create_arb128opt)()
{
    MODEM() q = MODEM(_create_arb)(7);
    MODEM(_arb_init)(q,(float complex*)modem_arb128opt,128);
    return q;
}

// create an arb256opt (optimal 256-qam) modem object
MODEM() MODEM(_create_arb256opt)()
{
    MODEM() q = MODEM(_create_arb)(8);
    MODEM(_arb_init)(q,(float complex*)modem_arb256opt,256);
    return q;
}

// create an arb64vt (64-qam vt logo) modem object
MODEM() MODEM(_create_arb64vt)()
{
    MODEM() q = MODEM(_create_arb)(6);
    MODEM(_arb_init)(q,(float complex*)modem_arb_vt64,64);
    return q;
}

// initialize an arbitrary modem object
//  _mod        :   modem object
//  _symbol_map :   arbitrary modem symbol map
//  _len        :   number of symbols in the map
void MODEM(_arb_init)(MODEM()         _q,
                      float complex * _symbol_map,
                      unsigned int    _len)
{
#ifdef LIQUID_VALIDATE_INPUT
    if (_q->scheme != LIQUID_MODEM_ARB) {
        fprintf(stderr,"error: modem_arb_init(), modem is not of arbitrary type\n");
        exit(1);
    } else if (_len != _q->M) {
        fprintf(stderr,"error: modem_arb_init(), array sizes do not match\n");
        exit(1);
    }
#endif

    // copy values
    unsigned int i;
    for (i=0; i<_len; i++) {
#if LIQUID_FPM
        _q->symbol_map[i] = CQ(_float_to_fixed)(_symbol_map[i]);
#else
        _q->symbol_map[i] = _symbol_map[i];
#endif
    }

    // balance I/Q channels
    if (_q->scheme == LIQUID_MODEM_ARB)
        MODEM(_arb_balance_iq)(_q);

    // scale modem to have unity energy
    MODEM(_arb_scale)(_q);

}

// initialize an arbitrary modem object on a file
//  _mod        :   modem object
//  _filename   :   name of the data file
void MODEM(_arb_init_file)(MODEM() _q,
                           char *  _filename)
{
    // try to open file
    FILE * fid = fopen(_filename, "r");
    if (fid == NULL) {
        fprintf(stderr,"error: modem_arb_init_file(), could not open file\n");
        exit(1);
    }

    unsigned int i, results;
    float sym_i, sym_q;
    for (i=0; i<_q->M; i++) {
        if ( feof(fid) ) {
            fprintf(stderr,"error: modem_arb_init_file(), premature EOF for '%s'\n", _filename);
            exit(1);
        }

        results = fscanf(fid, "%f %f\n", &sym_i, &sym_q);
#if LIQUID_FPM
        _q->symbol_map[i].real = sym_i;
        _q->symbol_map[i].imag = sym_q;
#else
        _q->symbol_map[i] = sym_i + _Complex_I*sym_q;
#endif

        // ensure proper number of symbols were read
        if (results < 2) {
            fprintf(stderr,"error: modem_arb_init_file(), unable to parse line\n");
            exit(1);
        }
    }

    fclose(fid);

    // balance I/Q channels
    if (_q->scheme == LIQUID_MODEM_ARB)
        MODEM(_arb_balance_iq)(_q);

    // scale modem to have unity energy
    MODEM(_arb_scale)(_q);
}

// scale arbitrary modem constellation points
void MODEM(_arb_scale)(MODEM() _q)
{
    unsigned int i;

    // calculate energy
    float complex v;
    float e = 0.0f;
    for (i=0; i<_q->M; i++) {
#if LIQUID_FPM
        v = CQ(_fixed_to_float)(_q->symbol_map[i]);
#else
        v = _q->symbol_map[i];
#endif
        e += crealf( v*conjf(v) );
    }

    e = sqrtf( e / _q->M );

    for (i=0; i<_q->M; i++) {
#if LIQUID_FPM
        v = CQ(_fixed_to_float)(_q->symbol_map[i]) / e;
        _q->symbol_map[i] = CQ(_float_to_fixed)(v);
#else
        _q->symbol_map[i] /= e;
#endif
    }
}

// balance an arbitrary modem's I/Q points
void MODEM(_arb_balance_iq)(MODEM() _q)
{
    float complex mean = 0.0f;
    unsigned int i;

#if LIQUID_FPM
    // accumulate average signal
    for (i=0; i<_q->M; i++)
        mean += CQ(_fixed_to_float)(_q->symbol_map[i]);

    mean /= (float) (_q->M);

    // subtract mean value from reference levels
    for (i=0; i<_q->M; i++) {
        float complex v = CQ(_fixed_to_float)(_q->symbol_map[i]) - mean;
        _q->symbol_map[i] = CQ(_float_to_fixed)(v);
    }
#else
    // accumulate average signal
    for (i=0; i<_q->M; i++)
        mean += _q->symbol_map[i];

    mean /= (float) (_q->M);

    // subtract mean value from reference levels
    for (i=0; i<_q->M; i++)
        _q->symbol_map[i] -= mean;
#endif
}

// demodulate arbitrary modem type (soft)
void MODEM(_demodulate_soft_arb)(MODEM()         _q,
                                 TC              _r,
                                 unsigned int  * _s,
                                 unsigned char * _soft_bits)
{
    unsigned int bps = _q->m;
    unsigned int M   = _q->M;

    // gamma = 1/(2*sigma^2), approximate for constellation size
    T gamma = 1.2f*_q->M;

    unsigned int s=0;       // hard decision output
    unsigned int k;         // bit index
    unsigned int i;         // symbol index
    T d;                // distance for this symbol
    TC x_hat;    // re-modulated symbol

    T dmin_0[bps];
    T dmin_1[bps];
    for (k=0; k<bps; k++) {
        dmin_0[k] = 4.0f;
        dmin_1[k] = 4.0f;
    }
    T dmin = 0.0f;

    for (i=0; i<M; i++) {
        // compute distance from received symbol
        x_hat = _q->symbol_map[i];
#if LIQUID_FPM
        d = CQ(_cabs2)( CQ(_sub)(_r,x_hat) );
#else
        d = crealf( (_r-x_hat)*conjf(_r-x_hat) );
#endif

        // set hard-decision...
        if (d < dmin || i==0) {
            s = i;
            dmin = d;
        }

        for (k=0; k<bps; k++) {
            // strip bit
            if ( (s >> (bps-k-1)) & 0x01 ) {
                if (d < dmin_1[k]) dmin_1[k] = d;
            } else {
                if (d < dmin_0[k]) dmin_0[k] = d;
            }
        }
    }

    // make assignments
    for (k=0; k<bps; k++) {
        int soft_bit = ((dmin_0[k] - dmin_1[k])*gamma)*16 + 127;
        if (soft_bit > 255) soft_bit = 255;
        if (soft_bit <   0) soft_bit = 0;
        _soft_bits[k] = (unsigned char)soft_bit;
    }

    // hard decision

    // set hard output symbol
    *_s = s;

    // re-modulate symbol and store state
    MODEM(_modulate_arb)(_q, *_s, &_q->x_hat);
    _q->r = _r;
}

