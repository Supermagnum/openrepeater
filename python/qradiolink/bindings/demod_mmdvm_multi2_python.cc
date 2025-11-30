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
#include <stdexcept>
#include <cstdint>

namespace py = pybind11;

#include <gnuradio/qradiolink/demod_mmdvm_multi2.h>

// Forward declaration
class BurstTimer;

void bind_demod_mmdvm_multi2(py::module& m)
{
    using demod_mmdvm_multi2 = gr::qradiolink::demod_mmdvm_multi2;

    py::class_<demod_mmdvm_multi2,
               gr::hier_block2,
               gr::basic_block,
               std::shared_ptr<demod_mmdvm_multi2>>(m, "demod_mmdvm_multi2")

        .def(py::init([](py::object burst_timer_obj,
                         int num_channels,
                         int channel_separation,
                         bool use_tdma,
                         int sps,
                         int samp_rate,
                         int carrier_freq,
                         int filter_width) {
            // Convert Python object to BurstTimer* pointer
            // Accept None/nullptr, or an integer representing a pointer address
            BurstTimer* burst_timer = nullptr;
            
            if (!burst_timer_obj.is_none()) {
                // Try to extract as integer (pointer address)
                try {
                    intptr_t ptr_val = py::cast<intptr_t>(burst_timer_obj);
                    burst_timer = reinterpret_cast<BurstTimer*>(ptr_val);
                } catch (py::cast_error&) {
                    throw std::runtime_error(
                        "burst_timer must be None or an integer pointer address. "
                        "For TDMA operation, provide a BurstTimer instance from C++ code.");
                }
            }
            
            return demod_mmdvm_multi2::make(
                burst_timer,
                num_channels,
                channel_separation,
                use_tdma,
                sps,
                samp_rate,
                carrier_freq,
                filter_width);
        }),
        py::arg("burst_timer") = py::none(),
        py::arg("num_channels") = 3,
        py::arg("channel_separation") = 25000,
        py::arg("use_tdma") = true,
        py::arg("sps") = 125,
        py::arg("samp_rate") = 250000,
        py::arg("carrier_freq") = 1700,
        py::arg("filter_width") = 5000,
        "Make an MMDVM multi-channel demodulator block (PFB version). "
        "burst_timer: None or integer pointer address to BurstTimer instance. "
        "Set use_tdma=False if not using TDMA timing.")

        .def("set_filter_width",
             &demod_mmdvm_multi2::set_filter_width,
             py::arg("filter_width"),
             "Set filter width")

        .def("calibrate_rssi",
             &demod_mmdvm_multi2::calibrate_rssi,
             py::arg("level"),
             "Calibrate RSSI");
}

