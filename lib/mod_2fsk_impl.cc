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
#include <gnuradio/qradiolink/mod_2fsk.h>
#include "mod_2fsk_impl.h"
#include <gnuradio/io_signature.h>

namespace gr {
namespace qradiolink {

mod_2fsk::sptr mod_2fsk::make(int sps,
                               int samp_rate,
                               int carrier_freq,
                               int filter_width,
                               bool fm)
{
    return gnuradio::get_initial_sptr(
        new mod_2fsk_impl(sps, samp_rate, carrier_freq, filter_width, fm));
}

mod_2fsk_impl::mod_2fsk_impl(int sps,
                               int samp_rate,
                               int carrier_freq,
                               int filter_width,
                               bool fm)
    : mod_2fsk("mod_2fsk",
               gr::io_signature::make(1, 1, sizeof(char)),
               gr::io_signature::make(1, 1, sizeof(gr_complex)))
{
    std::vector<float> constellation;
    constellation.push_back(-1);
    constellation.push_back(1);
    std::vector<int> polys;
    polys.push_back(109);
    polys.push_back(79);

    std::vector<int> map;
    map.push_back(0);
    map.push_back(1);

    d_samples_per_symbol = sps;
    d_samp_rate = samp_rate;
    d_carrier_freq = carrier_freq;
    d_filter_width = filter_width;
    int nfilts = 25 * d_samples_per_symbol;
    int spacing = 2;
    float amplif = 0.8f;
    if (fm) {
        spacing = 1;
        amplif = 0.9f;
    }
    int second_interp = 10;
    if (d_samples_per_symbol == 5) {
        nfilts = nfilts * 5;
    }
    if ((nfilts % 2) == 0)
        nfilts += 1;

    d_packed_to_unpacked = gr::blocks::packed_to_unpacked_bb::make(1, gr::GR_MSB_FIRST);
    d_scrambler = gr::digital::scrambler_bb::make(0x8A, 0x7F, 7);
    d_map = gr::digital::map_bb::make(map);

    gr::fec::code::cc_encoder::sptr encoder =
        gr::fec::code::cc_encoder::make(80, 7, 2, polys);
    d_encode_ccsds = gr::fec::encoder::make(encoder, 1, 1);

    d_chunks_to_symbols = gr::digital::chunks_to_symbols_bf::make(constellation);
    d_freq_modulator = gr::analog::frequency_modulator_fc::make(
        (spacing * M_PI / 2) / (d_samples_per_symbol));
    d_repeat = gr::blocks::repeat::make(4, d_samples_per_symbol);
    d_resampler = gr::filter::rational_resampler_fff::make(
        d_samples_per_symbol,
        1,
        gr::filter::firdes::root_raised_cosine(
            d_samples_per_symbol, d_samples_per_symbol, 1.0, 0.2, nfilts));
    d_amplify = gr::blocks::multiply_const_cc::make(amplif, 1);
    d_bb_gain = gr::blocks::multiply_const_cc::make(1, 1);
    d_resampler2 = gr::filter::rational_resampler_ccf::make(
        second_interp,
        1,
        gr::filter::firdes::low_pass(
            second_interp, d_samp_rate, d_filter_width, d_filter_width));

    connect(self(), 0, d_packed_to_unpacked, 0);
    connect(d_packed_to_unpacked, 0, d_scrambler, 0);
    connect(d_scrambler, 0, d_encode_ccsds, 0);
    connect(d_encode_ccsds, 0, d_map, 0);
    connect(d_map, 0, d_chunks_to_symbols, 0);
    if (fm) {
        connect(d_chunks_to_symbols, 0, d_resampler, 0);
        connect(d_resampler, 0, d_freq_modulator, 0);
    } else {
        connect(d_chunks_to_symbols, 0, d_repeat, 0);
        connect(d_repeat, 0, d_freq_modulator, 0);
    }
    connect(d_freq_modulator, 0, d_amplify, 0);
    connect(d_amplify, 0, d_bb_gain, 0);
    connect(d_bb_gain, 0, d_resampler2, 0);
    connect(d_resampler2, 0, self(), 0);
}

mod_2fsk_impl::~mod_2fsk_impl() {}

void mod_2fsk_impl::set_bb_gain(float value) { d_bb_gain->set_k(value); }

void mod_2fsk::set_bb_gain(float value)
{
    // This should never be called, as mod_2fsk is only an interface
    // The actual implementation is in mod_2fsk_impl
}

} // namespace qradiolink
} // namespace gr

