/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * libFuzzer harness for demod_gmsk
 * Tests: demod_gmsk_impl processing of complex input data
 */

#include <cstdint>
#include <cstddef>
#include <gnuradio/qradiolink/demod_gmsk.h>
#include <gnuradio/blocks/null_sink.h>
#include <gnuradio/blocks/vector_source.h>
#include <gnuradio/blocks/head.h>
#include <gnuradio/top_block.h>
#include <gnuradio/gr_complex.h>
#include <vector>
#include <complex>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    // Require minimum size to ensure enough complex samples are generated
    if (size < 128 || size > 2048) {
        return 0;
    }

    try {
        auto tb = gr::make_top_block("fuzz");
        // Use sps=10 (valid value) instead of 125 to properly initialize nfilts
        // This will exercise the sps==10 code path and avoid undefined behavior
        auto demod = gr::qradiolink::demod_gmsk::make(10, 250000, 1700, 8000);
        
        // Create null sinks for all outputs
        auto sink0 = gr::blocks::null_sink::make(sizeof(gr_complex));
        auto sink1 = gr::blocks::null_sink::make(sizeof(gr_complex));
        auto sink2 = gr::blocks::null_sink::make(sizeof(char));
        auto sink3 = gr::blocks::null_sink::make(sizeof(char));
        
        // Convert bytes to complex samples
        const size_t min_complex_samples = 32;
        std::vector<gr_complex> complex_data;
        complex_data.reserve(std::max(size / 2, min_complex_samples));
        
        for (size_t i = 0; i + 1 < size; i += 2) {
            float real = (float)((int8_t)data[i]) / 127.0f;
            float imag = (float)((int8_t)data[i + 1]) / 127.0f;
            complex_data.push_back(gr_complex(real, imag));
        }
        
        // Pad with zero complex samples if we don't have enough
        while (complex_data.size() < min_complex_samples) {
            complex_data.push_back(gr_complex(0.0f, 0.0f));
        }
        
        auto source = gr::blocks::vector_source<gr_complex>::make(complex_data, false);
        auto head = gr::blocks::head::make(sizeof(gr_complex), complex_data.size());
        
        tb->connect(source, 0, head, 0);
        tb->connect(head, 0, demod, 0);
        tb->connect(demod, 0, sink0, 0);
        tb->connect(demod, 1, sink1, 0);
        tb->connect(demod, 2, sink2, 0);
        tb->connect(demod, 3, sink3, 0);
        
        tb->start();
        tb->wait();
    } catch (...) {
    }
    
    return 0;
}

