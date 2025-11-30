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

#include <gnuradio/qradiolink/demod_dpmr.h>

void bind_demod_dpmr(py::module& m)
{
    using demod_dpmr = gr::qradiolink::demod_dpmr;

    py::class_<demod_dpmr,
               gr::hier_block2,
               gr::basic_block,
               std::shared_ptr<demod_dpmr>>(m, "demod_dpmr")

        .def(py::init(&demod_dpmr::make),
             py::arg("sps") = 125,
             py::arg("samp_rate") = 1000000,
             "Make a dPMR demodulator block");
}

