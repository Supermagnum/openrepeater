/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * libFuzzer harness for dsss_encoder_bb
 */

#include <cstdint>
#include <cstddef>
#include <algorithm>
#include "../../../src/gr/dsss_encoder_bb_impl.h"
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/blocks/vector_source.h>
#include <gnuradio/blocks/head.h>
#include <gnuradio/top_block.h>
#include <vector>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    if (size == 0 || size > 1024) {
        return 0;
    }

    try {
        auto tb = gr::make_top_block("fuzz");
        
        // Create spreading code (PN sequence)
        std::vector<int> spreading_code = {1, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 0};
        auto encoder = gr::dsss::dsss_encoder_bb::make(spreading_code);
        auto sink = gr::blocks::null_sink::make(sizeof(char));
        
        // Pad input to minimum size to ensure enough data is processed
        // Prevents corpus from shrinking to 1-2 bytes
        const size_t min_size = 64;
        std::vector<unsigned char> input_data;
        input_data.reserve(std::max(size, min_size));
        input_data.assign(data, data + size);
        // Pad with zeros if input is smaller than minimum
        if (input_data.size() < min_size) {
            input_data.resize(min_size, 0);
        }
        
        auto source = gr::blocks::vector_source<unsigned char>::make(input_data, false);
        auto head = gr::blocks::head::make(sizeof(unsigned char), input_data.size());
        
        tb->connect(source, 0, head, 0);
        tb->connect(head, 0, encoder, 0);
        tb->connect(encoder, 0, sink, 0);
        
        tb->start();
        tb->wait();
    } catch (...) {
    }
    
    return 0;
}

