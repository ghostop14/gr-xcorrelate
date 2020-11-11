/* -*- c++ -*- */

#define XCORRELATE_API

%include "gnuradio.i"           // the common stuff

//load generated python docstrings
%include "xcorrelate_swig_doc.i"

%{
#include "xcorrelate/xcorrelate.h"
#include "xcorrelate/xcorrelate_fft_vcf2.h"
#include "xcorrelate/xcorrelate_engine.h"
%}

%include "xcorrelate/xcorrelate.h"
GR_SWIG_BLOCK_MAGIC2(xcorrelate, xcorrelate);
%include "xcorrelate/xcorrelate_fft_vcf2.h"
GR_SWIG_BLOCK_MAGIC2(xcorrelate, xcorrelate_fft_vcf2);
%include "xcorrelate/xcorrelate_engine.h"
GR_SWIG_BLOCK_MAGIC2(xcorrelate, xcorrelate_engine);
