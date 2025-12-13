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

#include <gnuradio/qradiolink/ysf_decoder.h>

void bind_ysf_decoder(py::module& m)
{
    using ysf_decoder = gr::qradiolink::ysf_decoder;

    py::class_<ysf_decoder,
               gr::sync_block,
               gr::block,
               gr::basic_block,
               std::shared_ptr<ysf_decoder>>(m, "ysf_decoder")

        .def(py::init(&ysf_decoder::make),
             py::arg("sync_threshold") = 0.9f,
             "Make a YSF decoder block");
}

