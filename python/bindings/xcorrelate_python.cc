/*
 * Copyright 2020 Free Software Foundation, Inc.
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
/* BINDTOOL_HEADER_FILE(xcorrelate.h)                                        */
/* BINDTOOL_HEADER_FILE_HASH(f0ede94f918e2c7d1d8bf4cc58d83050)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <xcorrelate/xcorrelate.h>
// pydoc.h is automatically generated in the build directory
#include <xcorrelate_pydoc.h>

void bind_xcorrelate(py::module& m)
{

    using xcorrelate    = ::gr::xcorrelate::xcorrelate;


    py::class_<xcorrelate, gr::sync_decimator,
        std::shared_ptr<xcorrelate>>(m, "xcorrelate", D(xcorrelate))

        .def(py::init(&xcorrelate::make),
           py::arg("num_inputs"),
           py::arg("signal_length"),
           py::arg("data_type"),
           py::arg("data_size"),
           py::arg("max_search_index"),
           py::arg("decim_frames"),
           py::arg("num_outputs"),
           py::arg("async") = false,
           py::arg("normalize") = true,
           D(xcorrelate,make)
        )
        



        ;




}








