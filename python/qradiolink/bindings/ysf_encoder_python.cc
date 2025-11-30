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

#include <gnuradio/qradiolink/ysf_encoder.h>

void bind_ysf_encoder(py::module& m)
{
    using ysf_encoder = gr::qradiolink::ysf_encoder;

    py::class_<ysf_encoder,
               gr::sync_block,
               gr::block,
               gr::basic_block,
               std::shared_ptr<ysf_encoder>>(m, "ysf_encoder")

        .def(py::init(&ysf_encoder::make),
             py::arg("source_callsign") = "N0CALL    ",
             py::arg("destination_callsign") = "CQCQCQ    ",
             py::arg("radio_id") = 0,
             py::arg("group_id") = 0,
             "Make a YSF encoder block");
}

