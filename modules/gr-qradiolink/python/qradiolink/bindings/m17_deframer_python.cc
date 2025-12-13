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

#include <gnuradio/qradiolink/m17_deframer.h>

void bind_m17_deframer(py::module& m)
{
    using m17_deframer = gr::qradiolink::m17_deframer;

    py::class_<m17_deframer,
               gr::sync_block,
               gr::block,
               gr::basic_block,
               std::shared_ptr<m17_deframer>>(m, "m17_deframer")

        .def(py::init(&m17_deframer::make),
             py::arg("max_frame_length") = 330,
             "Make an M17 deframer block");
}

