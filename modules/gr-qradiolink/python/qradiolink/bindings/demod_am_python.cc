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

#include <gnuradio/qradiolink/demod_am.h>

void bind_demod_am(py::module& m)
{
    using demod_am = gr::qradiolink::demod_am;

    py::class_<demod_am,
               gr::hier_block2,
               gr::basic_block,
               std::shared_ptr<demod_am>>(m, "demod_am")

        .def(py::init(&demod_am::make),
             py::arg("sps") = 125,
             py::arg("samp_rate") = 250000,
             py::arg("carrier_freq") = 1700,
             py::arg("filter_width") = 8000,
             "Make an AM demodulator block")

        .def("set_squelch", &demod_am::set_squelch, py::arg("value"), "Set squelch level")
        .def("set_filter_width", &demod_am::set_filter_width, py::arg("filter_width"), "Set filter width")
        .def("set_agc_attack", &demod_am::set_agc_attack, py::arg("value"), "Set AGC attack rate")
        .def("set_agc_decay", &demod_am::set_agc_decay, py::arg("value"), "Set AGC decay rate");
}

