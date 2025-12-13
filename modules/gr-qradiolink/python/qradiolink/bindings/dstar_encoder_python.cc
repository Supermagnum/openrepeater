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

#include <gnuradio/qradiolink/dstar_encoder.h>

void bind_dstar_encoder(py::module& m)
{
    using dstar_encoder = gr::qradiolink::dstar_encoder;

    py::class_<dstar_encoder,
               gr::sync_block,
               gr::block,
               gr::basic_block,
               std::shared_ptr<dstar_encoder>>(m, "dstar_encoder")

        .def(py::init(&dstar_encoder::make),
             py::arg("my_callsign") = "N0CALL ",
             py::arg("your_callsign") = "CQCQCQ  ",
             py::arg("rpt1_callsign") = "        ",
             py::arg("rpt2_callsign") = "        ",
             py::arg("message_text") = "",
             "Make a D-STAR encoder block");
}

