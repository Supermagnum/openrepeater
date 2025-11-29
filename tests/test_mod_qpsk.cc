/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <gnuradio/attributes.h>
#include <gnuradio/qradiolink/mod_qpsk.h>
#include <gnuradio/block.h>
#include <gnuradio/blocks/null_source.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/top_block.h>
#include <boost/test/unit_test.hpp>
#include <iostream>

namespace gr {
namespace qradiolink {

BOOST_AUTO_TEST_CASE(test_mod_qpsk_instantiation)
{
    auto mod = mod_qpsk::make(125, 250000, 1700, 8000);
    BOOST_REQUIRE(mod != nullptr);
}

BOOST_AUTO_TEST_CASE(test_mod_qpsk_flowgraph)
{
    auto tb = gr::make_top_block("test");
    auto mod = mod_qpsk::make(125, 250000, 1700, 8000);
    auto source = gr::blocks::null_source::make(sizeof(char));
    auto sink = gr::blocks::null_sink::make(sizeof(gr_complex));

    tb->connect(source, 0, mod, 0);
    // If we get here, all connections succeeded
    BOOST_REQUIRE(true);
    tb->connect(mod, 0, sink, 0);
    // If we get here, all connections succeeded
    BOOST_REQUIRE(true);

}

BOOST_AUTO_TEST_CASE(test_mod_qpsk_set_bb_gain)
{
    auto mod = mod_qpsk::make(125, 250000, 1700, 8000);
    mod->set_bb_gain(0.5f);
    // If no exception is thrown, the call succeeded
    BOOST_REQUIRE(true);
}

} // namespace qradiolink
} // namespace gr

