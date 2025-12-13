/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * libFuzzer harness for m17_deframer
 * Tests: m17_deframer_impl processing of byte input data
 */

#include <cstdint>
#include <cstddef>
#include <gnuradio/qradiolink/m17_deframer.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/blocks/vector_source.h>
#include <gnuradio/blocks/head.h>
#include <gnuradio/top_block.h>
#include <vector>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Limit input size to prevent excessive processing time
    if (size == 0 || size > 1024) {
        return 0;
    }

    try {
        auto tb = gr::make_top_block("fuzz");
        auto deframer = gr::qradiolink::m17_deframer::make(330);
        auto sink = gr::blocks::null_sink::make(sizeof(uint8_t));
        
        // Use input as-is, no padding needed
        std::vector<uint8_t> input_data(data, data + size);
        
        auto source = gr::blocks::vector_source<uint8_t>::make(input_data, false);
        // Limit processing to input size
        auto head = gr::blocks::head::make(sizeof(uint8_t), input_data.size());
        
        tb->connect(source, 0, head, 0);
        tb->connect(head, 0, deframer, 0);
        tb->connect(deframer, 0, sink, 0);
        
        tb->start();
        // libFuzzer's timeout will catch hangs
        tb->wait();
    } catch (...) {
    }
    
    return 0;
}

