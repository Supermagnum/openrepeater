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

#include <gnuradio/qradiolink/pocsag_decoder.h>

void bind_pocsag_decoder(py::module& m)
{
    using pocsag_decoder = gr::qradiolink::pocsag_decoder;

    py::class_<pocsag_decoder,
               gr::sync_block,
               gr::block,
               gr::basic_block,
               std::shared_ptr<pocsag_decoder>>(m, "pocsag_decoder")

        .def(py::init(&pocsag_decoder::make),
             py::arg("baud_rate") = 1200,
             py::arg("sync_threshold") = 0.8f,
             "Make a POCSAG decoder block");
}

