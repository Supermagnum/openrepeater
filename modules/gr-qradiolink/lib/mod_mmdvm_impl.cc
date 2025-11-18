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

#include <gnuradio/qradiolink/mod_mmdvm.h>
#include "mod_mmdvm_impl.h"
#include <gnuradio/io_signature.h>
#include <cmath>

namespace gr {
namespace qradiolink {

mod_mmdvm::sptr mod_mmdvm::make(int sps, int samp_rate, int carrier_freq, int filter_width)
{
    return gnuradio::get_initial_sptr(new mod_mmdvm_impl(sps, samp_rate, carrier_freq, filter_width));
}

mod_mmdvm_impl::mod_mmdvm_impl(int sps, int samp_rate, int carrier_freq, int filter_width)
    : mod_mmdvm("mod_mmdvm",
                gr::io_signature::make(1, 1, sizeof(short)),
                gr::io_signature::make(1, 1, sizeof(gr_complex)))
{
    d_samp_rate = samp_rate;
    d_sps = sps;
    float target_samp_rate = 24000.0f;
    d_carrier_freq = carrier_freq;
    d_filter_width = filter_width;

    d_short_to_float = gr::blocks::short_to_float::make(1, 32767.0);
    d_fm_modulator = gr::analog::frequency_modulator_fc::make(2 * M_PI * 12500.0f / target_samp_rate);
    d_audio_amplify = gr::blocks::multiply_const_ff::make(1.0, 1);

    std::vector<float> interp_taps = gr::filter::firdes::low_pass_2(
        125, 125 * target_samp_rate, d_filter_width, 2000, 60, gr::fft::window::WIN_BLACKMAN_HARRIS);
    d_resampler = gr::filter::rational_resampler_ccf::make(125, 12, interp_taps);
    d_amplify = gr::blocks::multiply_const_cc::make(0.8, 1);
    d_bb_gain = gr::blocks::multiply_const_cc::make(1, 1);
    d_filter = gr::filter::fft_filter_ccf::make(
        1,
        gr::filter::firdes::low_pass_2(
            1, target_samp_rate, d_filter_width, 2000, 60, gr::fft::window::WIN_BLACKMAN_HARRIS));

    d_zero_idle_bursts = gr::qradiolink::zero_idle_bursts::make();

    connect(self(), 0, d_short_to_float, 0);
    connect(d_short_to_float, 0, d_audio_amplify, 0);
    connect(d_audio_amplify, 0, d_fm_modulator, 0);
    connect(d_fm_modulator, 0, d_zero_idle_bursts, 0);
    connect(d_zero_idle_bursts, 0, d_filter, 0);
    connect(d_filter, 0, d_amplify, 0);
    connect(d_amplify, 0, d_bb_gain, 0);
    connect(d_bb_gain, 0, d_resampler, 0);
    connect(d_resampler, 0, self(), 0);
}

mod_mmdvm_impl::~mod_mmdvm_impl() {}

void mod_mmdvm_impl::set_bb_gain(float value)
{
    d_bb_gain->set_k(value);
}


} // namespace qradiolink
} // namespace gr

