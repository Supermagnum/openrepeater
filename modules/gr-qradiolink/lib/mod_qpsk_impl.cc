/*
 * Copyright 2024 QRadioLink Contributors
 *
 * This file is part of gr-qradiolink
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 */


#include <climits>
#include <cstdint>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <gnuradio/qradiolink/mod_qpsk.h>
#include "mod_qpsk_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/digital/constellation.h>

namespace gr {
namespace qradiolink {

mod_qpsk::sptr mod_qpsk::make(int sps, int samp_rate, int carrier_freq, int filter_width)
{
    return gnuradio::get_initial_sptr(
        new mod_qpsk_impl(sps, samp_rate, carrier_freq, filter_width));
}

mod_qpsk_impl::mod_qpsk_impl(int sps, int samp_rate, int carrier_freq, int filter_width)
    : mod_qpsk("mod_qpsk",
               gr::io_signature::make(1, 1, sizeof(char)),
               gr::io_signature::make(1, 1, sizeof(gr_complex)))
{
    gr::digital::constellation_qpsk::sptr constellation = gr::digital::constellation_qpsk::make();
    std::vector<gr_complex> symbol_table;
    symbol_table.push_back(gr_complex(-0.707, -0.707));
    symbol_table.push_back(gr_complex(-0.707, 0.707));
    symbol_table.push_back(gr_complex(0.707, 0.707));
    symbol_table.push_back(gr_complex(0.707, -0.707));

    std::vector<int> map;
    map.push_back(0);
    map.push_back(1);
    map.push_back(3);
    map.push_back(2);

    std::vector<int> polys;
    polys.push_back(109);
    polys.push_back(79);

    d_samples_per_symbol = sps;
    d_samp_rate = samp_rate;
    d_carrier_freq = carrier_freq;
    d_filter_width = filter_width;
    int nfilts = 32;

    d_packed_to_unpacked = gr::blocks::packed_to_unpacked_bb::make(1, gr::GR_MSB_FIRST);
    gr::fec::code::cc_encoder::sptr encoder =
        gr::fec::code::cc_encoder::make(80, 7, 2, polys);
    d_encode_ccsds = gr::fec::encoder::make(encoder, 1, 1);

    d_packer = gr::blocks::pack_k_bits_bb::make(2);
    d_scrambler = gr::digital::scrambler_bb::make(0x8A, 0x7F, 7);
    d_diff_encoder = gr::digital::diff_encoder_bb::make(4);
    d_map = gr::digital::map_bb::make(map);

    d_chunks_to_symbols = gr::digital::chunks_to_symbols_bc::make(symbol_table);
    if (d_samples_per_symbol > 120)
        nfilts = 11;
    else if (d_samples_per_symbol > 10)
        nfilts = 13;
    else
        nfilts = 15;

    d_resampler = gr::filter::rational_resampler_ccf::make(
        d_samples_per_symbol,
        1,
        gr::filter::firdes::root_raised_cosine(
            d_samples_per_symbol, d_samples_per_symbol, 1.0, 0.35, nfilts * d_samples_per_symbol));
    d_amplify = gr::blocks::multiply_const_cc::make(0.6, 1);
    d_bb_gain = gr::blocks::multiply_const_cc::make(1, 1);

    connect(self(), 0, d_packed_to_unpacked, 0);
    connect(d_packed_to_unpacked, 0, d_scrambler, 0);
    connect(d_scrambler, 0, d_encode_ccsds, 0);
    connect(d_encode_ccsds, 0, d_packer, 0);
    connect(d_packer, 0, d_map, 0);
    connect(d_map, 0, d_diff_encoder, 0);
    connect(d_diff_encoder, 0, d_chunks_to_symbols, 0);
    connect(d_chunks_to_symbols, 0, d_resampler, 0);
    connect(d_resampler, 0, d_amplify, 0);
    connect(d_amplify, 0, d_bb_gain, 0);
    connect(d_bb_gain, 0, self(), 0);
}

mod_qpsk_impl::~mod_qpsk_impl() {}

void mod_qpsk_impl::set_bb_gain(float value) { d_bb_gain->set_k(value); }

void mod_qpsk::set_bb_gain(float value)
{
    // This should never be called, as mod_qpsk is only an interface
    // The actual implementation is in mod_qpsk_impl
}

} // namespace qradiolink
} // namespace gr

