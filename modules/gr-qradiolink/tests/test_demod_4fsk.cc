/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <gnuradio/attributes.h>
#include <gnuradio/qradiolink/demod_4fsk.h>
#include <gnuradio/block.h>
#include <gnuradio/blocks/null_source.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/top_block.h>
#include <boost/test/unit_test.hpp>
#include <iostream>

namespace gr {
namespace qradiolink {

BOOST_AUTO_TEST_CASE(test_demod_4fsk_instantiation)
{
    // Use sps=10 which has target_samp_rate=10000, so filter_width <= 5000
    // Using 3000 Hz should be fine for sps=10
    auto demod = demod_4fsk::make(10, 200000, 1700, 3000, true);
    BOOST_REQUIRE(demod != nullptr);
}

BOOST_AUTO_TEST_CASE(test_demod_4fsk_flowgraph)
{
    auto tb = gr::make_top_block("test");
    auto demod = demod_4fsk::make(10, 200000, 1700, 3000, true);
    auto source = gr::blocks::null_source::make(sizeof(gr_complex));
    auto sink1 = gr::blocks::null_sink::make(sizeof(gr_complex));
    auto sink2 = gr::blocks::null_sink::make(sizeof(gr_complex));
    auto sink3 = gr::blocks::null_sink::make(sizeof(char));

    tb->connect(source, 0, demod, 0);
    tb->connect(demod, 0, sink1, 0); // Filtered output
    tb->connect(demod, 1, sink2, 0); // Constellation output
    tb->connect(demod, 2, sink3, 0); // Decoded bytes
    
    // If we get here, all connections succeeded
    BOOST_REQUIRE(true);
}

BOOST_AUTO_TEST_CASE(test_demod_4fsk_fm_mode)
{
    auto demod = demod_4fsk::make(10, 200000, 1700, 3000, false);
    BOOST_REQUIRE(demod != nullptr);
}

} // namespace qradiolink
} // namespace gr

