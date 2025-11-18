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

#include <gnuradio/qradiolink/demod_wbfm.h>

void bind_demod_wbfm(py::module& m)
{
    using demod_wbfm = gr::qradiolink::demod_wbfm;

    py::class_<demod_wbfm,
               gr::hier_block2,
               gr::basic_block,
               std::shared_ptr<demod_wbfm>>(m, "demod_wbfm")

        .def(py::init(&demod_wbfm::make),
             py::arg("sps") = 125,
             py::arg("samp_rate") = 250000,
             py::arg("carrier_freq") = 1700,
             py::arg("filter_width") = 8000,
             "Make a WBFM demodulator block")

        .def("set_squelch", &demod_wbfm::set_squelch, py::arg("value"), "Set squelch level")
        .def("set_filter_width", &demod_wbfm::set_filter_width, py::arg("filter_width"), "Set filter width");
}

