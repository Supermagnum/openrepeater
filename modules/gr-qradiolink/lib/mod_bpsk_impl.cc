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
#include <gnuradio/qradiolink/mod_bpsk.h>
#include "mod_bpsk_impl.h"
#include <gnuradio/io_signature.h>

namespace gr {
namespace qradiolink {

mod_bpsk::sptr mod_bpsk::make(int sps, int samp_rate, int carrier_freq, int filter_width)
{
    return gnuradio::get_initial_sptr(
        new mod_bpsk_impl(sps, samp_rate, carrier_freq, filter_width));
}

mod_bpsk_impl::mod_bpsk_impl(int sps, int samp_rate, int carrier_freq, int filter_width)
    : mod_bpsk("mod_bpsk",
               gr::io_signature::make(1, 1, sizeof(char)),
               gr::io_signature::make(1, 1, sizeof(gr_complex)))
{
    std::vector<gr_complex> constellation;
    constellation.push_back(-1);
    constellation.push_back(1);
    std::vector<int> polys;
    polys.push_back(109);
    polys.push_back(79);

    d_samples_per_symbol = sps;
    d_samp_rate = samp_rate;
    d_carrier_freq = carrier_freq;
    d_filter_width = filter_width;

    d_packed_to_unpacked = gr::blocks::packed_to_unpacked_bb::make(1, gr::GR_MSB_FIRST);
    d_scrambler = gr::digital::scrambler_bb::make(0x8A, 0x7F, 7);

    gr::fec::code::cc_encoder::sptr encoder =
        gr::fec::code::cc_encoder::make(80, 7, 2, polys);
    d_encode_ccsds = gr::fec::encoder::make(encoder, 1, 1);

    d_chunks_to_symbols = gr::digital::chunks_to_symbols_bc::make(constellation);
    d_resampler = gr::filter::rational_resampler_ccf::make(
        d_samples_per_symbol,
        1,
        gr::filter::firdes::root_raised_cosine(
            d_samples_per_symbol, d_samples_per_symbol, 1.0, 0.35, 11 * d_samples_per_symbol));
    d_amplify = gr::blocks::multiply_const_cc::make(0.6, 1);
    d_bb_gain = gr::blocks::multiply_const_cc::make(1, 1);

    connect(self(), 0, d_packed_to_unpacked, 0);
    connect(d_packed_to_unpacked, 0, d_scrambler, 0);
    connect(d_scrambler, 0, d_encode_ccsds, 0);
    connect(d_encode_ccsds, 0, d_chunks_to_symbols, 0);
    connect(d_chunks_to_symbols, 0, d_resampler, 0);
    connect(d_resampler, 0, d_amplify, 0);
    connect(d_amplify, 0, d_bb_gain, 0);
    connect(d_bb_gain, 0, self(), 0);
}

mod_bpsk_impl::~mod_bpsk_impl() {}

void mod_bpsk_impl::set_bb_gain(float value) { d_bb_gain->set_k(value); }

void mod_bpsk::set_bb_gain(float value)
{
    // This should never be called, as mod_bpsk is only an interface
    // The actual implementation is in mod_bpsk_impl
}

} // namespace qradiolink
} // namespace gr

