/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <pybind11/pybind11.h>

namespace py = pybind11;

#include <gnuradio/qradiolink/p25_encoder.h>

void bind_p25_encoder(py::module& m)
{
    using p25_encoder = gr::qradiolink::p25_encoder;

    py::class_<p25_encoder,
               gr::sync_block,
               gr::block,
               gr::basic_block,
               std::shared_ptr<p25_encoder>>(m, "p25_encoder")

        .def(py::init(&p25_encoder::make),
             py::arg("nac") = 0x293,
             py::arg("source_id") = 0,
             py::arg("destination_id") = 0,
             py::arg("talkgroup_id") = 0,
             "Make a P25 Phase 1 encoder block");
}

