/*
 * Copyright (c) 2013 Joseph Gaeddert
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

#include "autotest/autotest.h"
#include "liquid.h"

// 
// AUTOTEST : test arbitrary resampler
//
void autotest_resamp_crcf()
{
    // options
    unsigned int m = 13;        // filter semi-length (filter delay)
    float r=1.27115323f;        // resampling rate (output/input)
    float bw=0.45f;             // resampling filter bandwidth
    float As=60.0f;             // resampling filter stop-band attenuation [dB]
    unsigned int npfb=128;      // number of filters in bank (timing resolution)
    unsigned int n=400;         // number of input samples
    float fx=0.381345969f;      // complex input sinusoid frequency (0.3*r)

    unsigned int i;

    // number of input samples (zero-padded)
    unsigned int nx = n + 2*m + 1;

    // output buffer with extra padding for good measure
    unsigned int y_len = (unsigned int) ceilf(1.1 * nx * r) + 4;

    // arrays
    float complex x[nx];
    float complex y[y_len];

    // create resampler
    resamp_crcf q = resamp_crcf_create(r,m,bw,As,npfb);

    // generate input signal
    float wsum = 0.0f;
    for (i=0; i<nx; i++) {
        // compute window
        float w = i < n ? kaiser(i, nx, 10.0f, 0.0f) : 0.0f;

        // apply window to complex sinusoid
        x[i] = cexpf(_Complex_I*2*M_PI*fx*i) * w;

        // accumulate window
        wsum += w;
    }

    // resample
    unsigned int ny=0;
    unsigned int nw;
    for (i=0; i<nx; i++) {
        // execute resampler, storing in output buffer
        resamp_crcf_execute(q, x[i], &y[ny], &nw);

        // increment output size
        ny += nw;
    }

    // clean up allocated objects
    resamp_crcf_destroy(q);

    // check that the actual resampling rate is close to the target
    float r_actual = (float)ny / (float)nx;
    if (liquid_autotest_verbose) {
        printf("  desired resampling rate   :   %12.8f\n", r);
        printf("  measured resampling rate  :   %12.8f (%u/%u)\n", r_actual, ny, nx);
    }
    CONTEND_DELTA( r_actual, r, 0.01f );

    // TODO: run FFT and ensure that carrier has moved and that
    //       image frequencies have been adequately suppressed
    unsigned int nfft = 1 << liquid_nextpow2(ny);
    float complex yfft[nfft];   // fft input
    float complex Yfft[nfft];   // fft output
    for (i=0; i<nfft; i++)
        yfft[i] = i < ny ? y[i] : 0.0f;
    fft_run(nfft, yfft, Yfft, FFT_FORWARD, 0);
    fft_shift(Yfft, nfft);  // run FFT shift

    // find peak frequency
    float Ypeak = 0.0f;
    float fpeak = 0.0f;
    for (i=0; i<nfft; i++) {
        // normalized output frequency
        float f = (float)i/(float)nfft - 0.5f;

        // scale FFT output appropriately
        Yfft[i] *= 1.0f / (r * wsum);

        float Ymag = cabsf(Yfft[i]);
        if (Ymag > Ypeak || i==0) {
            Ypeak = Ymag;
            fpeak = f;
        }
    }

    // print results and check frequency location
    float fy = fx / r;      // expected output frequency
    if (liquid_autotest_verbose) {
        printf("  peak spectrum             :   %12.8f (expected 1.0)\n", Ypeak);
        printf("  peak frequency            :   %12.8f (expected %-12.8f)\n", fpeak, fy);
    }
    CONTEND_DELTA( Ypeak, 1.0f, 0.05f );  // peak should be 1
    CONTEND_DELTA( fpeak, fy,   0.01f );  // value should be nearly 0.3

    // check magnitude
    for (i=0; i<nfft; i++) {
        // normalized output frequency
        float f = (float)i/(float)nfft - 0.5f;

        // ignore frequencies within a certain range of
        // signal frequency
        if ( fabsf(f-fy) < 0.05f ) {
            // skip
        } else {
            // check output spectral content is sufficiently suppressed
            //CONTEND_LESS_THAN( 20*log10f( cabsf(Yfft[i]) ), -As );
        }
    }

#if 0
    // export results for debugging
    char filename[] = "resamp_crcf_autotest.m";
    FILE*fid = fopen(filename,"w");
    fprintf(fid,"%% %s: auto-generated file\n",filename);
    fprintf(fid,"clear all;\n");
    fprintf(fid,"close all;\n");
    fprintf(fid,"r    = %12.8f;\n", r);
    fprintf(fid,"nx   = %u;\n", nx);
    fprintf(fid,"ny   = %u;\n", ny);
    fprintf(fid,"nfft = %u;\n", nfft);

    fprintf(fid,"Y = zeros(1,nfft);\n");
    for (i=0; i<nfft; i++)
        fprintf(fid,"Y(%3u) = %12.4e + j*%12.4e;\n", i+1, crealf(Yfft[i]), cimagf(Yfft[i]));

    fprintf(fid,"\n\n");
    fprintf(fid,"%% plot frequency-domain result\n");
    fprintf(fid,"f=[0:(nfft-1)]/nfft-0.5;\n");
    fprintf(fid,"figure;\n");
    fprintf(fid,"plot(f,20*log10(abs(Y)),'Color',[0.25 0.5 0.0],'LineWidth',2);\n");
    fprintf(fid,"grid on;\n");
    fprintf(fid,"xlabel('normalized frequency');\n");
    fprintf(fid,"ylabel('PSD [dB]');\n");
    fprintf(fid,"axis([-0.5 0.5 -100 20]);\n");

    fclose(fid);
    printf("results written to %s\n",filename);
#endif
}
