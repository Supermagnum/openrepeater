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

#include <gnuradio/qradiolink/mod_nxdn.h>

void bind_mod_nxdn(py::module& m)
{
    using mod_nxdn = gr::qradiolink::mod_nxdn;

    py::class_<mod_nxdn,
               gr::hier_block2,
               gr::basic_block,
               std::shared_ptr<mod_nxdn>>(m, "mod_nxdn")

        .def(py::init(&mod_nxdn::make),
             py::arg("symbol_rate") = 2400,
             py::arg("sps") = 125,
             py::arg("samp_rate") = 1000000,
             py::arg("carrier_freq") = 1700,
             py::arg("filter_width") = 6000,
             "Make an NXDN modulator block")

        .def("set_bb_gain",
             &mod_nxdn::set_bb_gain,
             py::arg("value"),
             "Set baseband gain");
}

