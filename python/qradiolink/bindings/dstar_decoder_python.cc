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

#include <gnuradio/qradiolink/dstar_decoder.h>

void bind_dstar_decoder(py::module& m)
{
    using dstar_decoder = gr::qradiolink::dstar_decoder;

    py::class_<dstar_decoder,
               gr::sync_block,
               gr::block,
               gr::basic_block,
               std::shared_ptr<dstar_decoder>>(m, "dstar_decoder")

        .def(py::init(&dstar_decoder::make),
             py::arg("sync_threshold") = 0.9f,
             "Make a D-STAR decoder block");
}

