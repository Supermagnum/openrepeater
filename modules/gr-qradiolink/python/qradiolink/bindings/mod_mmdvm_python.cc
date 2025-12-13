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

#include <gnuradio/qradiolink/mod_mmdvm.h>

void bind_mod_mmdvm(py::module& m)
{
    using mod_mmdvm = gr::qradiolink::mod_mmdvm;

    py::class_<mod_mmdvm,
               gr::hier_block2,
               gr::basic_block,
               std::shared_ptr<mod_mmdvm>>(m, "mod_mmdvm")

        .def(py::init(&mod_mmdvm::make),
             py::arg("sps") = 10,
             py::arg("samp_rate") = 250000,
             py::arg("carrier_freq") = 1700,
             py::arg("filter_width") = 5000,
             "Make an MMDVM modulator block")

        .def("set_bb_gain",
             &mod_mmdvm::set_bb_gain,
             py::arg("value"),
             "Set baseband gain");
}

