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

#include <gnuradio/qradiolink/demod_4fsk.h>

void bind_demod_4fsk(py::module& m)
{
    using demod_4fsk = gr::qradiolink::demod_4fsk;

    py::class_<demod_4fsk,
               gr::hier_block2,
               gr::basic_block,
               std::shared_ptr<demod_4fsk>>(m, "demod_4fsk")

        .def(py::init(&demod_4fsk::make),
             py::arg("sps") = 125,
             py::arg("samp_rate") = 250000,
             py::arg("carrier_freq") = 1700,
             py::arg("filter_width") = 8000,
             py::arg("fm") = true,
             "Make a 4FSK demodulator block");
}

