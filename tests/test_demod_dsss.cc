/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <gnuradio/attributes.h>
#include <gnuradio/qradiolink/demod_dsss.h>
#include <gnuradio/block.h>
#include <gnuradio/blocks/null_source.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/top_block.h>
#include <boost/test/unit_test.hpp>
#include <iostream>

namespace gr {
namespace qradiolink {

BOOST_AUTO_TEST_CASE(test_demod_dsss_instantiation)
{
    // filter_width must be <= target_samp_rate/2, where target_samp_rate = 5200 (fixed)
    // So filter_width must be <= 2600. Use 2000 Hz
    auto demod = demod_dsss::make(25, 250000, 1700, 2000);
    BOOST_REQUIRE(demod != nullptr);
}

BOOST_AUTO_TEST_CASE(test_demod_dsss_flowgraph)
{
    auto tb = gr::make_top_block("test");
    auto demod = demod_dsss::make(25, 250000, 1700, 2000);
    auto source = gr::blocks::null_source::make(sizeof(gr_complex));
    auto sink1 = gr::blocks::null_sink::make(sizeof(gr_complex));
    auto sink2 = gr::blocks::null_sink::make(sizeof(gr_complex));
    auto sink3 = gr::blocks::null_sink::make(sizeof(char));
    auto sink4 = gr::blocks::null_sink::make(sizeof(char));

    tb->connect(source, 0, demod, 0);
    tb->connect(demod, 0, sink1, 0); // Filtered output
    tb->connect(demod, 1, sink2, 0); // Constellation output
    tb->connect(demod, 2, sink3, 0); // Decoded primary
    tb->connect(demod, 3, sink4, 0); // Decoded delayed
    
    // If we get here, all connections succeeded
    BOOST_REQUIRE(true);
}

} // namespace qradiolink
} // namespace gr
