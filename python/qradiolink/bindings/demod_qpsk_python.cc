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

#include <gnuradio/qradiolink/demod_qpsk.h>

void bind_demod_qpsk(py::module& m)
{
    using demod_qpsk = gr::qradiolink::demod_qpsk;

    py::class_<demod_qpsk,
               gr::hier_block2,
               gr::basic_block,
               std::shared_ptr<demod_qpsk>>(m, "demod_qpsk")

        .def(py::init(&demod_qpsk::make),
             py::arg("sps") = 125,
             py::arg("samp_rate") = 250000,
             py::arg("carrier_freq") = 1700,
             py::arg("filter_width") = 8000,
             "Make a QPSK demodulator block");
}

