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

#include <gnuradio/qradiolink/mod_mmdvm_multi2.h>
#include "mod_mmdvm_multi2_impl.h"
#include <gnuradio/io_signature.h>
#include <cmath>
#include <algorithm>
#include "../src/bursttimer.h"

namespace gr {
namespace qradiolink {

mod_mmdvm_multi2::sptr mod_mmdvm_multi2::make(BurstTimer* burst_timer,
                                             int num_channels,
                                             int channel_separation,
                                             bool use_tdma,
                                             int sps,
                                             int samp_rate,
                                             int carrier_freq,
                                             int filter_width)
{
    return gnuradio::get_initial_sptr(new mod_mmdvm_multi2_impl(
        burst_timer, num_channels, channel_separation, use_tdma, sps, samp_rate, carrier_freq, filter_width));
}

mod_mmdvm_multi2_impl::mod_mmdvm_multi2_impl(BurstTimer* burst_timer,
                                            int num_channels,
                                            int channel_separation,
                                            bool use_tdma,
                                            int sps,
                                            int samp_rate,
                                            int carrier_freq,
                                            int filter_width)
    : mod_mmdvm_multi2("mod_mmdvm_multi2",
                      gr::io_signature::make(0, 0, sizeof(short)),
                      gr::io_signature::make(1, 1, sizeof(gr_complex))),
      d_samp_rate(samp_rate),
      d_sps(sps),
      d_carrier_freq(carrier_freq),
      d_filter_width(filter_width),
      d_num_channels(num_channels),
      d_use_tdma(use_tdma)
{
    (void)channel_separation;
    if (num_channels > MAX_MMDVM_CHANNELS)
        num_channels = MAX_MMDVM_CHANNELS;
    d_num_channels = num_channels;
    int min_c = std::min(num_channels, 4);
    float target_samp_rate = 24000.0f;
    float intermediate_samp_rate = 600000.0f;

    std::vector<float> intermediate_interp_taps = gr::filter::firdes::low_pass_2(
        25, intermediate_samp_rate, d_filter_width, 2000, 60, gr::fft::window::WIN_BLACKMAN_HARRIS);
    std::vector<float> filter_taps = gr::filter::firdes::low_pass_2(
        1, target_samp_rate, d_filter_width, 2000, 60, gr::fft::window::WIN_BLACKMAN_HARRIS);

    for (int i = 0; i < 10 - d_num_channels; i++) {
        d_null_source[i] = gr::blocks::null_source::make(sizeof(gr_complex));
    }
    for (int i = 0; i < d_num_channels; i++) {
        d_short_to_float[i] = gr::blocks::short_to_float::make(1, 32767.0);
    }
    for (int i = 0; i < d_num_channels; i++) {
        d_fm_modulator[i] = gr::analog::frequency_modulator_fc::make(2 * M_PI * 12500.0f / target_samp_rate);
    }
    for (int i = 0; i < d_num_channels; i++) {
        d_audio_amplify[i] = gr::blocks::multiply_const_ff::make(1.0, 1);
    }
    for (int i = 0; i < d_num_channels; i++) {
        d_resampler[i] = gr::filter::rational_resampler_ccf::make(25, 24, intermediate_interp_taps);
    }
    for (int i = 0; i < d_num_channels; i++) {
        d_amplify[i] = gr::blocks::multiply_const_cc::make(0.8, 1);
    }
    for (int i = 0; i < d_num_channels; i++) {
        d_filter[i] = gr::filter::fft_filter_ccf::make(1, filter_taps);
    }
    for (int i = 0; i < MAX_MMDVM_CHANNELS; i++) {
        d_zero_idle[i] = gr::qradiolink::zero_idle_bursts::make();
    }
    std::vector<float> synth_taps = gr::filter::firdes::low_pass_2(
        d_sps, d_samp_rate, d_filter_width, 2000, 60, gr::fft::window::WIN_BLACKMAN_HARRIS);
    d_synthesizer = gr::filter::pfb_synthesizer_ccf::make(10, synth_taps, false);
    d_divide_level = gr::blocks::multiply_const_cc::make(1.0f / float(num_channels));
    d_mmdvm_source = gr::qradiolink::mmdvm_source::make(burst_timer, num_channels, true, d_use_tdma);
    d_bb_gain = gr::blocks::multiply_const_cc::make(1, 1);

    uint32_t m = 1;
    for (int i = 0; i < d_num_channels; i++) {
        connect(d_mmdvm_source, i, d_short_to_float[i], 0);
        connect(d_short_to_float[i], 0, d_audio_amplify[i], 0);
        connect(d_audio_amplify[i], 0, d_fm_modulator[i], 0);
        connect(d_fm_modulator[i], 0, d_filter[i], 0);
        connect(d_filter[i], 0, d_amplify[i], 0);
        connect(d_amplify[i], 0, d_resampler[i], 0);
        connect(d_resampler[i], 0, d_zero_idle[i], 0);
        if (i <= 3) {
            connect(d_zero_idle[i], 0, d_synthesizer, i);
        } else if (i > 3) {
            connect(d_zero_idle[i], 0, d_synthesizer, 10 - m);
            m++;
        }
    }
    for (int i = 0; i < 10 - d_num_channels; i++) {
        connect(d_null_source[i], 0, d_synthesizer, min_c + i);
    }

    connect(d_synthesizer, 0, d_divide_level, 0);
    connect(d_divide_level, 0, d_bb_gain, 0);
    connect(d_bb_gain, 0, self(), 0);
}

mod_mmdvm_multi2_impl::~mod_mmdvm_multi2_impl() {}

void mod_mmdvm_multi2_impl::set_bb_gain(float value)
{
    d_bb_gain->set_k(value);
}

void mod_mmdvm_multi2::set_bb_gain(float value)
{
    // This should never be called, as mod_mmdvm_multi2 is only an interface
    // The actual implementation is in mod_mmdvm_multi2_impl
}

} // namespace qradiolink
} // namespace gr

