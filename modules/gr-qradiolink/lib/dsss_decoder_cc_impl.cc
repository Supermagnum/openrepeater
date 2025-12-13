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

#include <gnuradio/dsss/dsss_decoder_cc.h>
#include "dsss_decoder_cc_impl.h"
#include <gnuradio/io_signature.h>
#include <cmath>
#include <complex>

namespace gr {
namespace dsss {

dsss_decoder_cc::sptr dsss_decoder_cc::make(const std::vector<int>& spreading_code, int samples_per_symbol)
{
    return gnuradio::get_initial_sptr(new dsss_decoder_cc_impl(spreading_code, samples_per_symbol));
}

dsss_decoder_cc_impl::dsss_decoder_cc_impl(const std::vector<int>& spreading_code, int samples_per_symbol)
    : dsss_decoder_cc("dsss_decoder_cc",
                      gr::io_signature::make(1, 1, sizeof(gr_complex)),
                      gr::io_signature::make(1, 1, sizeof(gr_complex)),
                      spreading_code, samples_per_symbol),
      d_spreading_code(spreading_code),
      d_code_length(spreading_code.size()),
      d_samples_per_symbol(samples_per_symbol)
{
    if (d_code_length == 0) {
        throw std::invalid_argument("Spreading code cannot be empty");
    }
    
    // Convert spreading code to complex (for correlation)
    // Code values: 0 -> -1, 1 -> +1
    d_code_complex.resize(d_code_length);
    for (size_t i = 0; i < d_code_length; i++) {
        d_code_complex[i] = (d_spreading_code[i] == 1) ? gr_complex(1.0, 0.0) : gr_complex(-1.0, 0.0);
    }
    
    // Initialize correlation buffer
    d_correlation_buffer.resize(d_code_length * d_samples_per_symbol);
}

dsss_decoder_cc_impl::~dsss_decoder_cc_impl() {}

int dsss_decoder_cc_impl::work(int noutput_items,
                               gr_vector_const_void_star& input_items,
                               gr_vector_void_star& output_items)
{
    const gr_complex* in = (const gr_complex*)input_items[0];
    gr_complex* out = (gr_complex*)output_items[0];

    // DSSS decoding: Correlate input signal with spreading code
    // For each symbol period, correlate with the spreading code
    int samples_per_symbol = d_samples_per_symbol;
    int code_length = d_code_length;
    int samples_per_code = code_length * samples_per_symbol;
    
    int output_idx = 0;
    int input_idx = 0;
    
    while (output_idx < noutput_items) {
        // Check if we have enough input samples for one symbol
        if (input_idx + samples_per_code > noutput_items) {
            break;  // Not enough input samples
        }
        
        // Correlate current window with spreading code
        gr_complex correlation = gr_complex(0.0, 0.0);
        
        for (int chip = 0; chip < code_length; chip++) {
            int sample_offset = chip * samples_per_symbol;
            
            // Average samples over this chip period
            gr_complex chip_sum = gr_complex(0.0, 0.0);
            for (int s = 0; s < samples_per_symbol; s++) {
                chip_sum += in[input_idx + sample_offset + s];
            }
            gr_complex chip_avg = chip_sum / static_cast<float>(samples_per_symbol);
            
            // Multiply by code chip value and accumulate
            // Code: 0 -> -1, 1 -> +1
            correlation += chip_avg * d_code_complex[chip];
        }
        
        // Normalize correlation
        correlation /= static_cast<float>(code_length);
        
        // Output correlated value (one complex sample per symbol)
        out[output_idx++] = correlation;
        
        // Advance input by one symbol period
        input_idx += samples_per_code;
    }

    return output_idx;
}

} // namespace dsss
} // namespace gr

