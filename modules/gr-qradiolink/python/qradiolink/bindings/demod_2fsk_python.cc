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

#include <gnuradio/qradiolink/demod_2fsk.h>

void bind_demod_2fsk(py::module& m)
{
    using demod_2fsk = gr::qradiolink::demod_2fsk;

    py::class_<demod_2fsk,
               gr::hier_block2,
               gr::basic_block,
               std::shared_ptr<demod_2fsk>>(m, "demod_2fsk")

        .def(py::init(&demod_2fsk::make),
             py::arg("sps") = 125,
             py::arg("samp_rate") = 250000,
             py::arg("carrier_freq") = 1700,
             py::arg("filter_width") = 8000,
             py::arg("fm") = false,
             "Make a 2FSK demodulator block");
}

