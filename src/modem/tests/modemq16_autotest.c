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

#include "autotest/autotest.h"
#include "liquid.h"

// Help function to keep code base small
void modemq16_test_mod_demod(modulation_scheme _ms)
{
    float tol = q16_float_to_fixed(0.01f);

    // generate mod/demod
    modemq16 mod   = modemq16_create(_ms);
    modemq16 demod = modemq16_create(_ms);

    // run the test
    unsigned int i;
    unsigned int s;
    unsigned int M = 1 << modemq16_get_bps(mod);
    cq16_t x;
    float e = 0.0f;
    for (i=0; i<M; i++) {
        modemq16_modulate(mod, i, &x);
        modemq16_demodulate(demod, x, &s);
        CONTEND_EQUALITY(s, i);

        // get phase error and convert to float
        q16_t phase_error = modemq16_get_demodulator_phase_error(demod);
        CONTEND_DELTA( q16_fixed_to_float(phase_error), 0.0f, tol);
        
        // get error vector magnitude (EVM) and convert to float
        q16_t evm = modemq16_get_demodulator_evm(demod);
        CONTEND_DELTA( q16_fixed_to_float(evm), 0.0f, tol);

        // accumulate error
        float complex xf = cq16_fixed_to_float(x);
        e += crealf(xf*conjf(xf));
    }
    e = sqrtf(e / (float)M);

    // NOTE: fixed-point resolution for 256-ASK is not sufficient for
    //       normal amplitude test
    float amplitude_tolerance = (_ms == LIQUID_MODEM_ASK256) ? 0.15f : 0.03f;
    CONTEND_DELTA(e, 1.0f, amplitude_tolerance)

    // clean it up
    modemq16_destroy(mod);
    modemq16_destroy(demod);
}

// AUTOTESTS: generic PSK
void autotest_modemq16_psk2()      { modemq16_test_mod_demod(LIQUID_MODEM_PSK2);      }
void autotest_modemq16_psk4()      { modemq16_test_mod_demod(LIQUID_MODEM_PSK4);      }
void autotest_modemq16_psk8()      { modemq16_test_mod_demod(LIQUID_MODEM_PSK8);      }
void autotest_modemq16_psk16()     { modemq16_test_mod_demod(LIQUID_MODEM_PSK16);     }
void autotest_modemq16_psk32()     { modemq16_test_mod_demod(LIQUID_MODEM_PSK32);     }
void autotest_modemq16_psk64()     { modemq16_test_mod_demod(LIQUID_MODEM_PSK64);     }
void autotest_modemq16_psk128()    { modemq16_test_mod_demod(LIQUID_MODEM_PSK128);    }
void autotest_modemq16_psk256()    { modemq16_test_mod_demod(LIQUID_MODEM_PSK256);    }

// AUTOTESTS: generic DPSK
void autotest_modemq16_dpsk2()     { modemq16_test_mod_demod(LIQUID_MODEM_DPSK2);     }
void autotest_modemq16_dpsk4()     { modemq16_test_mod_demod(LIQUID_MODEM_DPSK4);     }
void autotest_modemq16_dpsk8()     { modemq16_test_mod_demod(LIQUID_MODEM_DPSK8);     }
void autotest_modemq16_dpsk16()    { modemq16_test_mod_demod(LIQUID_MODEM_DPSK16);    }
void autotest_modemq16_dpsk32()    { modemq16_test_mod_demod(LIQUID_MODEM_DPSK32);    }
void autotest_modemq16_dpsk64()    { modemq16_test_mod_demod(LIQUID_MODEM_DPSK64);    }
void autotest_modemq16_dpsk128()   { modemq16_test_mod_demod(LIQUID_MODEM_DPSK128);   }
void autotest_modemq16_dpsk256()   { modemq16_test_mod_demod(LIQUID_MODEM_DPSK256);   }

// AUTOTESTS: generic ASK
void autotest_modemq16_ask2()      { modemq16_test_mod_demod(LIQUID_MODEM_ASK2);      }
void autotest_modemq16_ask4()      { modemq16_test_mod_demod(LIQUID_MODEM_ASK4);      }
void autotest_modemq16_ask8()      { modemq16_test_mod_demod(LIQUID_MODEM_ASK8);      }
void autotest_modemq16_ask16()     { modemq16_test_mod_demod(LIQUID_MODEM_ASK16);     }
void autotest_modemq16_ask32()     { modemq16_test_mod_demod(LIQUID_MODEM_ASK32);     }
void autotest_modemq16_ask64()     { modemq16_test_mod_demod(LIQUID_MODEM_ASK64);     }
void autotest_modemq16_ask128()    { modemq16_test_mod_demod(LIQUID_MODEM_ASK128);    }
void autotest_modemq16_ask256()    { modemq16_test_mod_demod(LIQUID_MODEM_ASK256);    }

// AUTOTESTS: generic QAM
void autotest_modemq16_qam4()      { modemq16_test_mod_demod(LIQUID_MODEM_QAM4);      }
void autotest_modemq16_qam8()      { modemq16_test_mod_demod(LIQUID_MODEM_QAM8);      }
void autotest_modemq16_qam16()     { modemq16_test_mod_demod(LIQUID_MODEM_QAM16);     }
void autotest_modemq16_qam32()     { modemq16_test_mod_demod(LIQUID_MODEM_QAM32);     }
void autotest_modemq16_qam64()     { modemq16_test_mod_demod(LIQUID_MODEM_QAM64);     }
void autotest_modemq16_qam128()    { modemq16_test_mod_demod(LIQUID_MODEM_QAM128);    }
void autotest_modemq16_qam256()    { modemq16_test_mod_demod(LIQUID_MODEM_QAM256);    }

// AUTOTESTS: generic APSK (maps to specific APSK modems internally)
void autotest_modemq16_apsk4()     { modemq16_test_mod_demod(LIQUID_MODEM_APSK4);     }
void autotest_modemq16_apsk8()     { modemq16_test_mod_demod(LIQUID_MODEM_APSK8);     }
void autotest_modemq16_apsk16()    { modemq16_test_mod_demod(LIQUID_MODEM_APSK16);    }
void autotest_modemq16_apsk32()    { modemq16_test_mod_demod(LIQUID_MODEM_APSK32);    }
void autotest_modemq16_apsk64()    { modemq16_test_mod_demod(LIQUID_MODEM_APSK64);    }
void autotest_modemq16_apsk128()   { modemq16_test_mod_demod(LIQUID_MODEM_APSK128);   }
void autotest_modemq16_apsk256()   { modemq16_test_mod_demod(LIQUID_MODEM_APSK256);   }

// AUTOTESTS: Specific modems
void autotest_modemq16_bpsk()      { modemq16_test_mod_demod(LIQUID_MODEM_BPSK);      }
void autotest_modemq16_qpsk()      { modemq16_test_mod_demod(LIQUID_MODEM_QPSK);      }
void autotest_modemq16_ook()       { modemq16_test_mod_demod(LIQUID_MODEM_OOK);       }
void autotest_modemq16_sqam32()    { modemq16_test_mod_demod(LIQUID_MODEM_SQAM32);    }
void autotest_modemq16_sqam128()   { modemq16_test_mod_demod(LIQUID_MODEM_SQAM128);   }
void autotest_modemq16_V29()       { modemq16_test_mod_demod(LIQUID_MODEM_V29);       }
void autotest_modemq16_arb16opt()  { modemq16_test_mod_demod(LIQUID_MODEM_ARB16OPT);  }
void autotest_modemq16_arb32opt()  { modemq16_test_mod_demod(LIQUID_MODEM_ARB32OPT);  }
void autotest_modemq16_arb64opt()  { modemq16_test_mod_demod(LIQUID_MODEM_ARB64OPT);  }
void autotest_modemq16_arb128opt() { modemq16_test_mod_demod(LIQUID_MODEM_ARB128OPT); }
void autotest_modemq16_arb256opt() { modemq16_test_mod_demod(LIQUID_MODEM_ARB256OPT); }
void autotest_modemq16_arb64vt()   { modemq16_test_mod_demod(LIQUID_MODEM_ARB64VT);   }

