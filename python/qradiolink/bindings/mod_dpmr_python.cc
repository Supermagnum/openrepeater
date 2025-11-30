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

#include <gnuradio/qradiolink/mod_dpmr.h>

void bind_mod_dpmr(py::module& m)
{
    using mod_dpmr = gr::qradiolink::mod_dpmr;

    py::class_<mod_dpmr,
               gr::hier_block2,
               gr::basic_block,
               std::shared_ptr<mod_dpmr>>(m, "mod_dpmr")

        .def(py::init(&mod_dpmr::make),
             py::arg("sps") = 125,
             py::arg("samp_rate") = 1000000,
             py::arg("carrier_freq") = 1700,
             py::arg("filter_width") = 6000,
             "Make a dPMR modulator block")

        .def("set_bb_gain",
             &mod_dpmr::set_bb_gain,
             py::arg("value"),
             "Set baseband gain");
}

