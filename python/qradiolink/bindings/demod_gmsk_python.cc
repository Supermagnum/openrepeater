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

#include <gnuradio/qradiolink/demod_gmsk.h>

void bind_demod_gmsk(py::module& m)
{
    using demod_gmsk = gr::qradiolink::demod_gmsk;

    py::class_<demod_gmsk,
               gr::hier_block2,
               gr::basic_block,
               std::shared_ptr<demod_gmsk>>(m, "demod_gmsk")

        .def(py::init(&demod_gmsk::make),
             py::arg("sps") = 125,
             py::arg("samp_rate") = 250000,
             py::arg("carrier_freq") = 1700,
             py::arg("filter_width") = 8000,
             "Make a GMSK demodulator block");
}

