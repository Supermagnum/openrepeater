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

#include <gnuradio/qradiolink/demod_m17.h>

void bind_demod_m17(py::module& m)
{
    using demod_m17 = gr::qradiolink::demod_m17;

    py::class_<demod_m17,
               gr::hier_block2,
               gr::basic_block,
               std::shared_ptr<demod_m17>>(m, "demod_m17")

        .def(py::init(&demod_m17::make),
             py::arg("sps") = 125,
             py::arg("samp_rate") = 1000000,
             py::arg("carrier_freq") = 1700,
             py::arg("filter_width") = 9000,
             "Make an M17 demodulator block");
}

