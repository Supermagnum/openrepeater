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

#include <gnuradio/qradiolink/demod_dmr.h>

void bind_demod_dmr(py::module& m)
{
    using demod_dmr = gr::qradiolink::demod_dmr;

    py::class_<demod_dmr,
               gr::hier_block2,
               gr::basic_block,
               std::shared_ptr<demod_dmr>>(m, "demod_dmr")

        .def(py::init(&demod_dmr::make),
             py::arg("sps") = 125,
             py::arg("samp_rate") = 1000000,
             "Make a DMR demodulator block");
}

