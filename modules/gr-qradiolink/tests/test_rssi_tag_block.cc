/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <gnuradio/attributes.h>
#include <gnuradio/qradiolink/rssi_tag_block.h>
#include <gnuradio/block.h>
#include <gnuradio/blocks/null_source.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/top_block.h>
#include <boost/test/unit_test.hpp>
#include <iostream>

namespace gr {
namespace qradiolink {

BOOST_AUTO_TEST_CASE(test_rssi_tag_block_instantiation)
{
    auto rssi = rssi_tag_block::make();
    BOOST_REQUIRE(rssi != nullptr);
}

BOOST_AUTO_TEST_CASE(test_rssi_tag_block_flowgraph)
{
    auto tb = gr::make_top_block("test");
    auto rssi = rssi_tag_block::make();
    auto source = gr::blocks::null_source::make(sizeof(gr_complex));
    auto sink = gr::blocks::null_sink::make(sizeof(gr_complex));

    tb->connect(source, 0, rssi, 0);
    tb->connect(rssi, 0, sink, 0);
    
    // If we get here, all connections succeeded
    BOOST_REQUIRE(true);
}

BOOST_AUTO_TEST_CASE(test_rssi_tag_block_calibrate)
{
    auto rssi = rssi_tag_block::make();
    BOOST_REQUIRE(rssi != nullptr);
    
    // Test calibration method
    rssi->calibrate_rssi(0.5);
    
    // If we get here, calibration succeeded
    BOOST_REQUIRE(true);
}

} // namespace qradiolink
} // namespace gr

