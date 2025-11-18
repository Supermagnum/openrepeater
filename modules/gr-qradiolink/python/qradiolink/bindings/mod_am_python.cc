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

#include <gnuradio/qradiolink/mod_am.h>

void bind_mod_am(py::module& m)
{
    using mod_am = gr::qradiolink::mod_am;

    py::class_<mod_am,
               gr::hier_block2,
               gr::basic_block,
               std::shared_ptr<mod_am>>(m, "mod_am")

        .def(py::init(&mod_am::make),
             py::arg("sps") = 125,
             py::arg("samp_rate") = 250000,
             py::arg("carrier_freq") = 1700,
             py::arg("filter_width") = 8000,
             "Make an AM modulator block")

        .def("set_filter_width",
             &mod_am::set_filter_width,
             py::arg("filter_width"),
             "Set filter width")

        .def("set_bb_gain",
             &mod_am::set_bb_gain,
             py::arg("value"),
             "Set baseband gain");
}

