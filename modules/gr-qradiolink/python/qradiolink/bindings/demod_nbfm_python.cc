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

#include <gnuradio/qradiolink/demod_nbfm.h>

void bind_demod_nbfm(py::module& m)
{
    using demod_nbfm = gr::qradiolink::demod_nbfm;

    py::class_<demod_nbfm,
               gr::hier_block2,
               gr::basic_block,
               std::shared_ptr<demod_nbfm>>(m, "demod_nbfm")

        .def(py::init(&demod_nbfm::make),
             py::arg("sps") = 125,
             py::arg("samp_rate") = 250000,
             py::arg("carrier_freq") = 1700,
             py::arg("filter_width") = 8000,
             "Make an NBFM demodulator block")

        .def("set_squelch", &demod_nbfm::set_squelch, py::arg("value"), "Set squelch level")
        .def("set_ctcss", &demod_nbfm::set_ctcss, py::arg("value"), "Set CTCSS tone frequency")
        .def("set_filter_width", &demod_nbfm::set_filter_width, py::arg("filter_width"), "Set filter width");
}

