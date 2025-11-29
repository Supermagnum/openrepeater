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

#include <gnuradio/qradiolink/demod_ssb.h>

void bind_demod_ssb(py::module& m)
{
    using demod_ssb = gr::qradiolink::demod_ssb;

    py::class_<demod_ssb,
               gr::hier_block2,
               gr::basic_block,
               std::shared_ptr<demod_ssb>>(m, "demod_ssb")

        .def(py::init(&demod_ssb::make),
             py::arg("sps") = 125,
             py::arg("samp_rate") = 250000,
             py::arg("carrier_freq") = 1700,
             py::arg("filter_width") = 8000,
             py::arg("sb") = 0,
             "Make an SSB demodulator block")

        .def("set_squelch", &demod_ssb::set_squelch, py::arg("value"), "Set squelch level")
        .def("set_filter_width", &demod_ssb::set_filter_width, py::arg("filter_width"), "Set filter width")
        .def("set_agc_attack", &demod_ssb::set_agc_attack, py::arg("value"), "Set AGC attack rate")
        .def("set_agc_decay", &demod_ssb::set_agc_decay, py::arg("value"), "Set AGC decay rate")
        .def("set_gain", &demod_ssb::set_gain, py::arg("value"), "Set gain");
}

