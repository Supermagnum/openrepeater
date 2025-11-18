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
#include <gnuradio/qradiolink/mod_dmr.h>
#include "mod_dmr_impl.h"
#include <gnuradio/io_signature.h>
#include <cmath>
#include <cstdlib>
#include <cstring>

#include <gnuradio/qradiolink/zero_idle_bursts.h>

namespace gr {
namespace qradiolink {

mod_dmr::sptr mod_dmr::make(int sps, int samp_rate, int carrier_freq, int filter_width)
{
    return gnuradio::get_initial_sptr(new mod_dmr_impl(sps, samp_rate, carrier_freq, filter_width));
}

mod_dmr_impl::mod_dmr_impl(int sps, int samp_rate, int carrier_freq, int filter_width)
    : mod_dmr("mod_dmr",
              gr::io_signature::make(1, 1, sizeof(char)),
              gr::io_signature::make(1, 1, sizeof(gr_complex)))
{
    d_samp_rate = samp_rate;
    d_sps = sps;
    d_samples_per_symbol = 5;
    float if_samp_rate = 24000;
    float symbol_rate = ((float)if_samp_rate / (float)d_samples_per_symbol);
    d_carrier_freq = carrier_freq;
    d_filter_width = filter_width;
    std::vector<float> constellation;
    constellation.push_back(-1.5);
    constellation.push_back(-0.5);
    constellation.push_back(0.5);
    constellation.push_back(1.5);

    std::vector<int> map;
    map.push_back(2);
    map.push_back(3);
    map.push_back(1);
    map.push_back(0);

    d_packed_to_unpacked = gr::blocks::packed_to_unpacked_bb::make(1, gr::GR_MSB_FIRST);
    d_packer = gr::blocks::pack_k_bits_bb::make(2);
    d_map = gr::digital::map_bb::make(map);

    d_chunks_to_symbols = gr::digital::chunks_to_symbols_bf::make(constellation);
    std::vector<float> first_resampler_taps = gr::filter::firdes::root_raised_cosine(
        d_samples_per_symbol, if_samp_rate, symbol_rate, 0.2, 25 * d_samples_per_symbol);
    unsigned int first_resampler_delay = (first_resampler_taps.size() - 1) / 2;
    d_first_resampler = gr::filter::rational_resampler_fff::make(d_samples_per_symbol, 1, first_resampler_taps);
    d_scale_pulses = gr::blocks::multiply_const_ff::make(0.66666666, 1);

    d_fm_modulator = gr::analog::frequency_modulator_fc::make((M_PI * symbol_rate * 0.85) / if_samp_rate);
    
    std::vector<float> interp_taps = gr::filter::firdes::low_pass_2(
        d_sps, d_samp_rate * 3, d_filter_width, 2000, 60, gr::fft::window::WIN_BLACKMAN_HARRIS);
    d_resampler = gr::filter::rational_resampler_ccf::make(d_sps, 3, interp_taps);
    d_amplify = gr::blocks::multiply_const_cc::make(0.9, 1);
    d_bb_gain = gr::blocks::multiply_const_cc::make(1, 1);
    d_filter = gr::filter::fft_filter_ccf::make(
        1,
        gr::filter::firdes::low_pass(
            1, if_samp_rate, d_filter_width, d_filter_width, gr::fft::window::WIN_BLACKMAN_HARRIS));

    d_zero_idle = gr::qradiolink::zero_idle_bursts::make(first_resampler_delay);

    connect(self(), 0, d_packed_to_unpacked, 0);
    connect(d_packed_to_unpacked, 0, d_packer, 0);
    connect(d_packer, 0, d_map, 0);
    connect(d_map, 0, d_chunks_to_symbols, 0);
    connect(d_chunks_to_symbols, 0, d_first_resampler, 0);
    connect(d_first_resampler, 0, d_scale_pulses, 0);
    connect(d_scale_pulses, 0, d_fm_modulator, 0);
    connect(d_fm_modulator, 0, d_zero_idle, 0);
    connect(d_zero_idle, 0, d_amplify, 0);
    connect(d_amplify, 0, d_bb_gain, 0);
    connect(d_bb_gain, 0, d_resampler, 0);
    connect(d_resampler, 0, self(), 0);
}

mod_dmr_impl::~mod_dmr_impl() {}

void mod_dmr_impl::set_bb_gain(float value) { d_bb_gain->set_k(value); }

void mod_dmr::set_bb_gain(float value)
{
    // This should never be called, as mod_dmr is only an interface
    // The actual implementation is in mod_dmr_impl
}

} // namespace qradiolink
} // namespace gr

