/*
 * Copyright 2021 Free Software Foundation, Inc.
 *
 * This file is part of GNU Radio
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

/***********************************************************************************/
/* This file is automatically generated using bindtool and can be manually edited  */
/* The following lines can be configured to regenerate this file during cmake      */
/* If manual edits are made, the following tags should be modified accordingly.    */
/* BINDTOOL_GEN_AUTOMATIC(0)                                                       */
/* BINDTOOL_USE_PYGCCXML(0)                                                        */
/* BINDTOOL_HEADER_FILE(auto_polarization.h)                                        */
/* BINDTOOL_HEADER_FILE_HASH(863ad2f59bb858ab5afbc51f597b8139)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <xcorrelate/auto_polarization.h>
// pydoc.h is automatically generated in the build directory
#include <auto_polarization_pydoc.h>

void bind_auto_polarization(py::module& m)
{

    using auto_polarization    = ::gr::xcorrelate::auto_polarization;


    py::class_<auto_polarization, gr::sync_block, gr::block, gr::basic_block,
        std::shared_ptr<auto_polarization>>(m, "auto_polarization", D(auto_polarization))

        .def(py::init(&auto_polarization::make),
           py::arg("fft_size"),
           py::arg("fft_avg"),
           py::arg("iir_weight"),
           D(auto_polarization,make)
        )
        



        ;




}








