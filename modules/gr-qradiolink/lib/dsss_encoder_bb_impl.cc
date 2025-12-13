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

#include <gnuradio/dsss/dsss_encoder_bb.h>
#include "dsss_encoder_bb_impl.h"
#include <gnuradio/io_signature.h>
#include <cstring>
#include <stdexcept>

namespace gr {
namespace dsss {

dsss_encoder_bb::sptr dsss_encoder_bb::make(const std::vector<int>& spreading_code)
{
    return gnuradio::get_initial_sptr(new dsss_encoder_bb_impl(spreading_code));
}

dsss_encoder_bb_impl::dsss_encoder_bb_impl(const std::vector<int>& spreading_code)
    : dsss_encoder_bb("dsss_encoder_bb",
                     gr::io_signature::make(1, 1, sizeof(unsigned char)),
                     gr::io_signature::make(1, 1, sizeof(unsigned char)),
                     spreading_code),
      d_spreading_code(spreading_code),
      d_code_length(spreading_code.size())
{
    if (d_code_length == 0) {
        throw std::invalid_argument("Spreading code cannot be empty");
    }
}

dsss_encoder_bb_impl::~dsss_encoder_bb_impl() {}

int dsss_encoder_bb_impl::work(int noutput_items,
                               gr_vector_const_void_star& input_items,
                               gr_vector_void_star& output_items)
{
    const unsigned char* in = (const unsigned char*)input_items[0];
    unsigned char* out = (unsigned char*)output_items[0];

    // DSSS encoding: Input is unpacked bits (one bit per byte, 0 or 1)
    // For each input bit, output the spreading code chips
    // If input bit is 1, output spreading code as-is
    // If input bit is 0, output inverted spreading code (XOR with 1)
    
    int input_idx = 0;
    int output_idx = 0;
    
    while (output_idx < noutput_items && input_idx < noutput_items) {
        // Get input bit (unpacked format: byte is 0 or 1)
        unsigned char input_bit = (in[input_idx] != 0) ? 1 : 0;
        
        // Spread this bit: output spreading code chips
        // For each chip in spreading code, output chip XOR input_bit
        for (size_t chip_idx = 0; chip_idx < d_code_length && output_idx < noutput_items; chip_idx++) {
            int chip_value = d_spreading_code[chip_idx];
            // XOR chip with input bit: if input is 0, invert code; if 1, use code as-is
            unsigned char output_chip = chip_value ^ (1 - input_bit);
            out[output_idx++] = output_chip;
        }
        
        input_idx++;
    }

    return output_idx;
}

} // namespace dsss
} // namespace gr

