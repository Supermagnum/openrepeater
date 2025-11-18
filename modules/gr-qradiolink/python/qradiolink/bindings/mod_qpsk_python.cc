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

#include <gnuradio/qradiolink/mod_qpsk.h>

void bind_mod_qpsk(py::module& m)
{
    using mod_qpsk = gr::qradiolink::mod_qpsk;

    py::class_<mod_qpsk,
               gr::hier_block2,
               gr::basic_block,
               std::shared_ptr<mod_qpsk>>(m, "mod_qpsk")

        .def(py::init(&mod_qpsk::make),
             py::arg("sps") = 125,
             py::arg("samp_rate") = 250000,
             py::arg("carrier_freq") = 1700,
             py::arg("filter_width") = 8000,
             "Make a QPSK modulator block")

        .def("set_bb_gain",
             &mod_qpsk::set_bb_gain,
             py::arg("value"),
             "Set baseband gain");
}

