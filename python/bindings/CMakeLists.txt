# Copyright 2020 Free Software Foundation, Inc.
#
# This file is part of GNU Radio
#
# SPDX-License-Identifier: GPL-3.0-or-later
#

########################################################################
# Check if there is C++ code at all
########################################################################
if(NOT xcorrelate_sources)
    MESSAGE(STATUS "No C++ sources... skipping python bindings")
    return()
endif(NOT xcorrelate_sources)

########################################################################
# Check for pygccxml
########################################################################
GR_PYTHON_CHECK_MODULE_RAW(
    "pygccxml"
    "import pygccxml"
    PYGCCXML_FOUND
    )

include(GrPybind)

########################################################################
# Python Bindings
########################################################################

list(APPEND xcorrelate_python_files
    auto_polarization_python.cc
    triangular_to_full_python.cc
    xcorrelate_engine_python.cc
    xcorrelate_fft_vcf2_python.cc
    xcorrelate_python.cc python_bindings.cc)

GR_PYBIND_MAKE_OOT(xcorrelate 
   ../..
   gr::xcorrelate
   "${xcorrelate_python_files}")

install(TARGETS xcorrelate_python DESTINATION ${GR_PYTHON_DIR}/xcorrelate COMPONENT pythonapi)
