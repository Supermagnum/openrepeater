/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <gnuradio/attributes.h>
#include <gnuradio/qradiolink/mod_ssb.h>
#include <gnuradio/block.h>
#include <gnuradio/blocks/null_source.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/top_block.h>
#include <boost/test/unit_test.hpp>
#include <iostream>

namespace gr {
namespace qradiolink {

BOOST_AUTO_TEST_CASE(test_mod_ssb_instantiation)
{
    // filter_width must be <= target_samp_rate/2, where target_samp_rate = 8000 (fixed)
    // So filter_width must be <= 4000. Use 3000 Hz which is reasonable for SSB
    auto mod = mod_ssb::make(125, 250000, 1700, 3000, 0);
    BOOST_REQUIRE(mod != nullptr);
}

BOOST_AUTO_TEST_CASE(test_mod_ssb_flowgraph)
{
    auto tb = gr::make_top_block("test");
    auto mod = mod_ssb::make(125, 250000, 1700, 3000, 0);
    auto source = gr::blocks::null_source::make(sizeof(float));
    auto sink = gr::blocks::null_sink::make(sizeof(gr_complex));

    tb->connect(source, 0, mod, 0);
    tb->connect(mod, 0, sink, 0);
    
    // If we get here, all connections succeeded
    BOOST_REQUIRE(true);
}

BOOST_AUTO_TEST_CASE(test_mod_ssb_set_bb_gain)
{
    auto mod = mod_ssb::make(125, 250000, 1700, 3000, 0);
    mod->set_bb_gain(0.5f);
    // If no exception is thrown, the call succeeded
    BOOST_REQUIRE(true);
}

BOOST_AUTO_TEST_CASE(test_mod_ssb_set_filter_width)
{
    auto mod = mod_ssb::make(125, 250000, 1700, 3000, 0);
    mod->set_filter_width(2500);
    // If no exception is thrown, the call succeeded
    BOOST_REQUIRE(true);
}

BOOST_AUTO_TEST_CASE(test_mod_ssb_lsb)
{
    // filter_width must be <= target_samp_rate/2 = 4000, use 3000 Hz
    auto mod = mod_ssb::make(125, 250000, 1700, 3000, 1);
    BOOST_REQUIRE(mod != nullptr);
}

} // namespace qradiolink
} // namespace gr

