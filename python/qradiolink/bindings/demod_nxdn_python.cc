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

#include <gnuradio/qradiolink/demod_nxdn.h>

void bind_demod_nxdn(py::module& m)
{
    using demod_nxdn = gr::qradiolink::demod_nxdn;

    py::class_<demod_nxdn,
               gr::hier_block2,
               gr::basic_block,
               std::shared_ptr<demod_nxdn>>(m, "demod_nxdn")

        .def(py::init(&demod_nxdn::make),
             py::arg("symbol_rate") = 2400,
             py::arg("sps") = 125,
             py::arg("samp_rate") = 1000000,
             "Make an NXDN demodulator block");
}

