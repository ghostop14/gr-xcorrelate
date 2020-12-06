/* -*- c++ -*- */

#define XCORRELATE_API

%include "gnuradio.i"           // the common stuff

//load generated python docstrings
%include "xcorrelate_swig_doc.i"

%{
#include "xcorrelate/xcorrelate.h"
#include "xcorrelate/xcorrelate_fft_vcf2.h"
#include "xcorrelate/xcorrelate_engine.h"
#include "xcorrelate/triangular_to_full.h"
#include "xcorrelate/auto_polarization.h"
%}

%include "xcorrelate/xcorrelate.h"
GR_SWIG_BLOCK_MAGIC2(xcorrelate, xcorrelate);
%include "xcorrelate/xcorrelate_fft_vcf2.h"
GR_SWIG_BLOCK_MAGIC2(xcorrelate, xcorrelate_fft_vcf2);
%include "xcorrelate/xcorrelate_engine.h"
GR_SWIG_BLOCK_MAGIC2(xcorrelate, xcorrelate_engine);
%include "xcorrelate/triangular_to_full.h"
GR_SWIG_BLOCK_MAGIC2(xcorrelate, triangular_to_full);
%include "xcorrelate/auto_polarization.h"
GR_SWIG_BLOCK_MAGIC2(xcorrelate, auto_polarization);
