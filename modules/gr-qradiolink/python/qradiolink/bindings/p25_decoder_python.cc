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

#include <gnuradio/qradiolink/p25_decoder.h>

void bind_p25_decoder(py::module& m)
{
    using p25_decoder = gr::qradiolink::p25_decoder;

    py::class_<p25_decoder,
               gr::sync_block,
               gr::block,
               gr::basic_block,
               std::shared_ptr<p25_decoder>>(m, "p25_decoder")

        .def(py::init(&p25_decoder::make),
             py::arg("sync_threshold") = 0.9f,
             "Make a P25 Phase 1 decoder block");
}

