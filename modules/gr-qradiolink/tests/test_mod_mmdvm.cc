/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <gnuradio/qradiolink/mod_mmdvm.h>
#include <gnuradio/blocks/vector_source.h>
#include <gnuradio/blocks/head.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/top_block.h>
#include <gnuradio/gr_complex.h>
#include <iostream>
#include <vector>

int main(int argc, char** argv)
{
    std::cout << "Testing mod_mmdvm..." << std::endl;

    // Create test data (shorts - mod_mmdvm expects 16-bit samples)
    std::vector<short> test_data = {0x0100, 0x2300, 0x4500, 0x6700, static_cast<short>(0x8900), static_cast<short>(0xAB00), static_cast<short>(0xCD00), static_cast<short>(0xEF00)};

    // Create blocks
    auto src = gr::blocks::vector_source<short>::make(test_data);
    auto mod = gr::qradiolink::mod_mmdvm::make(10, 250000, 1700, 5000);
    auto head = gr::blocks::head::make(sizeof(gr_complex), 1000);
    auto sink = gr::blocks::null_sink::make(sizeof(gr_complex));

    // Create flowgraph
    auto tb = gr::make_top_block("test");

    tb->connect(src, 0, mod, 0);
    tb->connect(mod, 0, head, 0);
    tb->connect(head, 0, sink, 0);

    // Run
    tb->start();
    tb->wait();

    std::cout << "mod_mmdvm test passed!" << std::endl;
    return 0;
}

