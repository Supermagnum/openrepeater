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

#include <gnuradio/qradiolink/demod_freedv.h>
#include <gnuradio/vocoder/freedv_api.h>

void bind_demod_freedv(py::module& m)
{
    using demod_freedv = gr::qradiolink::demod_freedv;

    py::class_<demod_freedv,
               gr::hier_block2,
               gr::basic_block,
               std::shared_ptr<demod_freedv>>(m, "demod_freedv")

        .def(py::init(&demod_freedv::make),
             py::arg("sps") = 125,
             py::arg("samp_rate") = 8000,
             py::arg("carrier_freq") = 1700,
             py::arg("filter_width") = 2000,
             py::arg("low_cutoff") = 200,
             py::arg("mode") = gr::vocoder::freedv_api::MODE_1600,
             py::arg("sb") = 0,
             "Make a FreeDV demodulator block")

        .def("set_agc_attack",
             &demod_freedv::set_agc_attack,
             py::arg("value"),
             "Set AGC attack rate")

        .def("set_agc_decay",
             &demod_freedv::set_agc_decay,
             py::arg("value"),
             "Set AGC decay rate")

        .def("set_squelch",
             &demod_freedv::set_squelch,
             py::arg("value"),
             "Set squelch threshold");
}

