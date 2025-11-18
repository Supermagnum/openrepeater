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

#include <gnuradio/qradiolink/mod_nbfm.h>

void bind_mod_nbfm(py::module& m)
{
    using mod_nbfm = gr::qradiolink::mod_nbfm;

    py::class_<mod_nbfm,
               gr::hier_block2,
               gr::basic_block,
               std::shared_ptr<mod_nbfm>>(m, "mod_nbfm")

        .def(py::init(&mod_nbfm::make),
             py::arg("sps") = 125,
             py::arg("samp_rate") = 250000,
             py::arg("carrier_freq") = 1700,
             py::arg("filter_width") = 8000,
             "Make an NBFM modulator block")

        .def("set_filter_width",
             &mod_nbfm::set_filter_width,
             py::arg("filter_width"),
             "Set filter width")

        .def("set_ctcss",
             &mod_nbfm::set_ctcss,
             py::arg("value"),
             "Set CTCSS tone")

        .def("set_bb_gain",
             &mod_nbfm::set_bb_gain,
             py::arg("value"),
             "Set baseband gain");
}

