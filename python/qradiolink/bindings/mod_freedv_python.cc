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

#include <gnuradio/qradiolink/mod_freedv.h>
#include <gnuradio/vocoder/freedv_api.h>

void bind_mod_freedv(py::module& m)
{
    using mod_freedv = gr::qradiolink::mod_freedv;

    py::class_<mod_freedv,
               gr::hier_block2,
               gr::basic_block,
               std::shared_ptr<mod_freedv>>(m, "mod_freedv")

        .def(py::init(&mod_freedv::make),
             py::arg("sps") = 125,
             py::arg("samp_rate") = 8000,
             py::arg("carrier_freq") = 1700,
             py::arg("filter_width") = 2000,
             py::arg("low_cutoff") = 200,
             py::arg("mode") = gr::vocoder::freedv_api::MODE_1600,
             py::arg("sb") = 0,
             "Make a FreeDV modulator block")

        .def("set_bb_gain",
             &mod_freedv::set_bb_gain,
             py::arg("value"),
             "Set baseband gain");
}

