/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#include <gnuradio/attributes.h>
#include <gnuradio/qradiolink/demod_freedv.h>
#include <gnuradio/vocoder/freedv_api.h>
#include <gnuradio/block.h>
#include <gnuradio/blocks/null_source.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/top_block.h>
#include <boost/test/unit_test.hpp>
#include <iostream>

namespace gr {
namespace qradiolink {

BOOST_AUTO_TEST_CASE(test_demod_freedv_instantiation)
{
    auto demod = demod_freedv::make(
        125, 8000, 1700, 2000, 200,
        gr::vocoder::freedv_api::MODE_1600, 0);
    BOOST_REQUIRE(demod != nullptr);
}

BOOST_AUTO_TEST_CASE(test_demod_freedv_flowgraph)
{
    auto tb = gr::make_top_block("test");
    auto demod = demod_freedv::make(
        125, 8000, 1700, 2000, 200,
        gr::vocoder::freedv_api::MODE_1600, 0);
    auto source = gr::blocks::null_source::make(sizeof(gr_complex));
    auto sink1 = gr::blocks::null_sink::make(sizeof(gr_complex));
    auto sink2 = gr::blocks::null_sink::make(sizeof(float));

    tb->connect(source, 0, demod, 0);
    tb->connect(demod, 0, sink1, 0); // Filtered output
    tb->connect(demod, 1, sink2, 0); // Audio output
    
    // If we get here, all connections succeeded
    BOOST_REQUIRE(true);
}

} // namespace qradiolink
} // namespace gr

