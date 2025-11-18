/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <pybind11/pybind11.h>
#include <pybind11/complex.h>

namespace py = pybind11;

#include <gnuradio/qradiolink/mod_ssb.h>

void bind_mod_ssb(py::module& m)
{
    using mod_ssb = gr::qradiolink::mod_ssb;

    py::class_<mod_ssb,
               gr::hier_block2,
               gr::basic_block,
               std::shared_ptr<mod_ssb>>(m, "mod_ssb")

        .def(py::init(&mod_ssb::make),
             py::arg("sps") = 125,
             py::arg("samp_rate") = 250000,
             py::arg("carrier_freq") = 1700,
             py::arg("filter_width") = 8000,
             py::arg("sb") = 0,
             "Make an SSB modulator block (sb=0 for USB, sb=1 for LSB)")

        .def("set_filter_width",
             &mod_ssb::set_filter_width,
             py::arg("filter_width"),
             "Set filter width")

        .def("set_bb_gain",
             &mod_ssb::set_bb_gain,
             py::arg("value"),
             "Set baseband gain");
}

