/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <gnuradio/attributes.h>
#include <gnuradio/qradiolink/demod_mmdvm_multi2.h>
#include <gnuradio/block.h>
#include <gnuradio/blocks/null_source.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/top_block.h>
#include <boost/test/unit_test.hpp>
#include <iostream>
#include "test_bursttimer.h"

namespace gr {
namespace qradiolink {

BOOST_AUTO_TEST_CASE(test_demod_mmdvm_multi2_instantiation_without_tdma)
{
    // Test without TDMA (use_tdma=false, burst_timer can be nullptr)
    auto demod = demod_mmdvm_multi2::make(
        nullptr,  // burst_timer
        3,        // num_channels
        25000,    // channel_separation
        false,    // use_tdma
        125,      // sps
        250000,   // samp_rate
        1700,     // carrier_freq
        5000      // filter_width
    );
    BOOST_REQUIRE(demod != nullptr);
}

BOOST_AUTO_TEST_CASE(test_demod_mmdvm_multi2_instantiation_with_tdma)
{
    // Test with TDMA using mock BurstTimer
    MockBurstTimer mock_timer;
    auto demod = demod_mmdvm_multi2::make(
        &mock_timer,  // burst_timer
        3,            // num_channels
        25000,        // channel_separation
        true,         // use_tdma
        125,          // sps
        250000,       // samp_rate
        1700,         // carrier_freq
        5000          // filter_width
    );
    BOOST_REQUIRE(demod != nullptr);
}

BOOST_AUTO_TEST_CASE(test_demod_mmdvm_multi2_flowgraph)
{
    // Test flowgraph connection without TDMA
    // Note: demod_mmdvm_multi2 has no output ports (output signature is 0, 0)
    // It uses mmdvm_sink internally which outputs via ZeroMQ
    auto tb = gr::make_top_block("test");
    auto demod = demod_mmdvm_multi2::make(
        nullptr,  // burst_timer
        3,        // num_channels
        25000,    // channel_separation
        false,    // use_tdma
        125,      // sps
        250000,   // samp_rate
        1700,     // carrier_freq
        5000      // filter_width
    );
    auto source = gr::blocks::null_source::make(sizeof(gr_complex));
    
    // Connect input only (no output ports to connect)
    tb->connect(source, 0, demod, 0);
    
    // If we get here, connection succeeded
    BOOST_REQUIRE(true);
}

BOOST_AUTO_TEST_CASE(test_demod_mmdvm_multi2_methods)
{
    MockBurstTimer mock_timer;
    auto demod = demod_mmdvm_multi2::make(
        &mock_timer,
        3,
        25000,
        false,
        125,
        250000,
        1700,
        5000
    );
    BOOST_REQUIRE(demod != nullptr);
    
    // Test set_filter_width
    demod->set_filter_width(10000);
    
    // Test calibrate_rssi
    demod->calibrate_rssi(0.5);
    
    // If we get here, methods succeeded
    BOOST_REQUIRE(true);
}

} // namespace qradiolink
} // namespace gr

