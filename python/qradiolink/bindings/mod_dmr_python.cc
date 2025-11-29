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

#include <gnuradio/qradiolink/mod_dmr.h>

void bind_mod_dmr(py::module& m)
{
    using mod_dmr = gr::qradiolink::mod_dmr;

    py::class_<mod_dmr,
               gr::hier_block2,
               gr::basic_block,
               std::shared_ptr<mod_dmr>>(m, "mod_dmr")

        .def(py::init(&mod_dmr::make),
             py::arg("sps") = 125,
             py::arg("samp_rate") = 1000000,
             py::arg("carrier_freq") = 1700,
             py::arg("filter_width") = 9000,
             "Make a DMR modulator block")

        .def("set_bb_gain",
             &mod_dmr::set_bb_gain,
             py::arg("value"),
             "Set baseband gain");
}


