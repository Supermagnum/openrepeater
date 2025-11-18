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
#include <gnuradio/qradiolink/mod_gmsk.h>
#include "mod_gmsk_impl.h"
#include <gnuradio/io_signature.h>
#include <cmath>

namespace gr {
namespace qradiolink {

mod_gmsk::sptr mod_gmsk::make(int sps, int samp_rate, int carrier_freq, int filter_width)
{
    return gnuradio::get_initial_sptr(
        new mod_gmsk_impl(sps, samp_rate, carrier_freq, filter_width));
}

mod_gmsk_impl::mod_gmsk_impl(int sps, int samp_rate, int carrier_freq, int filter_width)
    : mod_gmsk("mod_gmsk",
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
    int nfilts = 35;
    float amplif = 0.9f;

    int second_interp = 5;
    if (d_samples_per_symbol == 10) {
        d_samples_per_symbol = 50;
        second_interp = 1;
        nfilts = 55;
    }
    if (d_samples_per_symbol == 50) {
        nfilts = 55;
    }
    if (d_samples_per_symbol == 100) {
        nfilts = 35;
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
    d_freq_modulator = gr::analog::frequency_modulator_fc::make((M_PI / 2) / (d_samples_per_symbol));
    d_resampler = gr::filter::rational_resampler_fff::make(
        d_samples_per_symbol,
        1,
        gr::filter::firdes::gaussian(d_samples_per_symbol, d_samples_per_symbol, 0.3, nfilts));
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
    connect(d_chunks_to_symbols, 0, d_resampler, 0);
    connect(d_resampler, 0, d_freq_modulator, 0);
    connect(d_freq_modulator, 0, d_amplify, 0);
    connect(d_amplify, 0, d_bb_gain, 0);
    connect(d_bb_gain, 0, d_resampler2, 0);
    connect(d_resampler2, 0, self(), 0);
}

mod_gmsk_impl::~mod_gmsk_impl() {}

void mod_gmsk_impl::set_bb_gain(float value) { d_bb_gain->set_k(value); }

void mod_gmsk::set_bb_gain(float value)
{
    // This should never be called, as mod_gmsk is only an interface
    // The actual implementation is in mod_gmsk_impl
}

} // namespace qradiolink
} // namespace gr

