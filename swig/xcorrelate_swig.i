/* -*- c++ -*- */

#define XCORRELATE_API

%include "gnuradio.i"           // the common stuff

//load generated python docstrings
%include "xcorrelate_swig_doc.i"

%{
#include "xcorrelate/xcorrelate.h"
%}

%include "xcorrelate/xcorrelate.h"
GR_SWIG_BLOCK_MAGIC2(xcorrelate, xcorrelate);
