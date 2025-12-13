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

#include <gnuradio/qradiolink/rssi_tag_block.h>

void bind_rssi_tag_block(py::module& m)
{
    using rssi_tag_block = gr::qradiolink::rssi_tag_block;

    py::class_<rssi_tag_block,
               gr::sync_block,
               gr::block,
               gr::basic_block,
               std::shared_ptr<rssi_tag_block>>(m, "rssi_tag_block")

        .def(py::init(&rssi_tag_block::make),
             "Make an RSSI tag block")

        .def("calibrate_rssi",
             &rssi_tag_block::calibrate_rssi,
             py::arg("level"),
             "Calibrate RSSI level");
}

