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

#include <gnuradio/qradiolink/mod_am.h>
#include <gnuradio/blocks/vector_source.h>
#include <gnuradio/blocks/head.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/top_block.h>
#include <gnuradio/gr_complex.h>
#include <iostream>
#include <vector>
#include <cmath>

int main(int argc, char** argv)
{
    std::cout << "Testing mod_am..." << std::endl;

    // Create test audio data (sine wave)
    std::vector<float> test_data;
    for (int i = 0; i < 1000; i++) {
        test_data.push_back(0.5 * std::sin(2.0 * M_PI * 440.0 * i / 8000.0));
    }

    // Create blocks
    auto src = gr::blocks::vector_source<float>::make(test_data);
    auto mod = gr::qradiolink::mod_am::make(125, 250000, 1700, 8000);
    auto head = gr::blocks::head::make(sizeof(gr_complex), 5000);
    auto sink = gr::blocks::null_sink::make(sizeof(gr_complex));

    // Create flowgraph
    auto tb = gr::make_top_block("test");

    tb->connect(src, 0, mod, 0);
    tb->connect(mod, 0, head, 0);
    tb->connect(head, 0, sink, 0);

    // Run
    tb->start();
    tb->wait();

    std::cout << "mod_am test passed!" << std::endl;
    return 0;
}
