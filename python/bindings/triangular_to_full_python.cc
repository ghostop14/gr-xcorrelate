/*
 * Copyright 2022 Free Software Foundation, Inc.
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
/* BINDTOOL_HEADER_FILE(triangular_to_full.h)                                        */
/* BINDTOOL_HEADER_FILE_HASH(a2e32a7421e4c63b804056583b58aede)                     */
/***********************************************************************************/

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <xcorrelate/triangular_to_full.h>
// pydoc.h is automatically generated in the build directory
#include <triangular_to_full_pydoc.h>

void bind_triangular_to_full(py::module& m)
{

    using triangular_to_full    = ::gr::xcorrelate::triangular_to_full;


    py::class_<triangular_to_full, gr::block, gr::basic_block,
        std::shared_ptr<triangular_to_full>>(m, "triangular_to_full", D(triangular_to_full))

        .def(py::init(&triangular_to_full::make),
           py::arg("polarization"),
           py::arg("num_inputs"),
           py::arg("num_channels"),
           D(triangular_to_full,make)
        )
        




        
        .def("xgpuExtractMatrix",&triangular_to_full::xgpuExtractMatrix,       
            py::arg("matrix"),
            py::arg("packed"),
            D(triangular_to_full,xgpuExtractMatrix)
        )

        ;




}








