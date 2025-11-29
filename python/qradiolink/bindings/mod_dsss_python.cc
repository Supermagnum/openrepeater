/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <pybind11/complex.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

#include <gnuradio/qradiolink/mod_dsss.h>

void bind_mod_dsss(py::module& m)
{
    using mod_dsss = gr::qradiolink::mod_dsss;

    py::class_<mod_dsss,
               gr::hier_block2,
               gr::basic_block,
               std::shared_ptr<mod_dsss>>(m, "mod_dsss")

        .def(py::init(&mod_dsss::make),
             py::arg("sps") = 25,
             py::arg("samp_rate") = 250000,
             py::arg("carrier_freq") = 1700,
             py::arg("filter_width") = 8000,
             "Make a DSSS modulator block")

        .def("set_bb_gain",
             &mod_dsss::set_bb_gain,
             py::arg("value"),
             "Set baseband gain");
}

