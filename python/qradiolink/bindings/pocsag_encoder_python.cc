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

#include <gnuradio/qradiolink/pocsag_encoder.h>

void bind_pocsag_encoder(py::module& m)
{
    using pocsag_encoder = gr::qradiolink::pocsag_encoder;

    py::class_<pocsag_encoder,
               gr::sync_block,
               gr::block,
               gr::basic_block,
               std::shared_ptr<pocsag_encoder>>(m, "pocsag_encoder")

        .def(py::init(&pocsag_encoder::make),
             py::arg("baud_rate") = 1200,
             py::arg("address") = 0,
             py::arg("function_bits") = 0,
             "Make a POCSAG encoder block");
}

