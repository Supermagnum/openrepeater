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

#include <gnuradio/qradiolink/mod_bpsk.h>

void bind_mod_bpsk(py::module& m)
{
    using mod_bpsk = gr::qradiolink::mod_bpsk;

    py::class_<mod_bpsk,
               gr::hier_block2,
               gr::basic_block,
               std::shared_ptr<mod_bpsk>>(m, "mod_bpsk")

        .def(py::init(&mod_bpsk::make),
             py::arg("sps") = 125,
             py::arg("samp_rate") = 250000,
             py::arg("carrier_freq") = 1700,
             py::arg("filter_width") = 8000,
             "Make a BPSK modulator block")

        .def("set_bb_gain",
             &mod_bpsk::set_bb_gain,
             py::arg("value"),
             "Set baseband gain");
}

