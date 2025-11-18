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

#include <gnuradio/qradiolink/demod_mmdvm_multi2.h>
#include "demod_mmdvm_multi2_impl.h"
#include <gnuradio/io_signature.h>
#include <cmath>
#include <algorithm>
#include "../../src/bursttimer.h"
#include "../../src/config_mmdvm.h"

namespace gr {
namespace qradiolink {

demod_mmdvm_multi2::sptr demod_mmdvm_multi2::make(BurstTimer* burst_timer,
                                                 int num_channels,
                                                 int channel_separation,
                                                 bool use_tdma,
                                                 int sps,
                                                 int samp_rate,
                                                 int carrier_freq,
                                                 int filter_width)
{
    return gnuradio::get_initial_sptr(new demod_mmdvm_multi2_impl(
        burst_timer, num_channels, channel_separation, use_tdma, sps, samp_rate, carrier_freq, filter_width));
}

demod_mmdvm_multi2_impl::demod_mmdvm_multi2_impl(BurstTimer* burst_timer,
                                                 int num_channels,
                                                 int channel_separation,
                                                 bool use_tdma,
                                                 int sps,
                                                 int samp_rate,
                                                 int carrier_freq,
                                                 int filter_width)
    : demod_mmdvm_multi2("demod_mmdvm_multi2",
                        gr::io_signature::make(1, 1, sizeof(gr_complex)),
                        gr::io_signature::make(0, 0, sizeof(short))),
      d_samp_rate(samp_rate),
      d_carrier_freq(carrier_freq),
      d_filter_width(filter_width),
      d_num_channels(num_channels),
      d_use_tdma(use_tdma)
{
    (void)sps;
    (void)channel_separation;
    int min_c = std::min(num_channels, 4);
    if (num_channels > MAX_MMDVM_CHANNELS)
        num_channels = MAX_MMDVM_CHANNELS;
    d_num_channels = num_channels;
    float target_samp_rate = 24000;
    float intermediate_samp_rate = 600000.0f;
    float fm_demod_width = 12500.0f;

    std::vector<float> taps = gr::filter::firdes::low_pass_2(
        1, d_samp_rate, d_filter_width, 2000, 60, gr::fft::window::WIN_BLACKMAN_HARRIS);
    std::vector<float> intermediate_interp_taps = gr::filter::firdes::low_pass_2(
        1, intermediate_samp_rate, d_filter_width, 2000, 60, gr::fft::window::WIN_BLACKMAN_HARRIS);
    std::vector<float> filter_taps = gr::filter::firdes::low_pass_2(
        1, target_samp_rate, d_filter_width, 2000, 60, gr::fft::window::WIN_BLACKMAN_HARRIS);

    for (int i = 0; i < d_num_channels; i++) {
        d_resampler[i] = gr::filter::rational_resampler_ccf::make(24, 25, intermediate_interp_taps);
    }
    for (int i = 0; i < d_num_channels; i++) {
        d_filter[i] = gr::filter::fft_filter_ccf::make(1, filter_taps);
    }
    for (int i = 0; i < d_num_channels; i++) {
        d_fm_demod[i] = gr::analog::quadrature_demod_cf::make(
            float(target_samp_rate) / (2 * M_PI * float(fm_demod_width)));
    }
    for (int i = 0; i < d_num_channels; i++) {
        d_level_control[i] = gr::blocks::multiply_const_ff::make(1.0);
    }
    for (int i = 0; i < 10 - d_num_channels; i++) {
        d_null_sink[i] = gr::blocks::null_sink::make(sizeof(gr_complex));
    }
    for (int i = 0; i < d_num_channels; i++) {
        d_float_to_short[i] = gr::blocks::float_to_short::make(1, 32767.0);
    }
    for (int i = 0; i < d_num_channels; i++) {
        d_rssi_tag_block[i] = gr::qradiolink::rssi_tag_block::make();
    }
    d_channelizer = gr::filter::pfb_channelizer_ccf::make(10, taps, 1.0f);
    d_channelizer->set_tag_propagation_policy(gr::block::TPP_ALL_TO_ALL);
    d_stream_to_streams = gr::blocks::stream_to_streams::make(sizeof(gr_complex), 10);
    d_mmdvm_sink = gr::qradiolink::mmdvm_sink::make(burst_timer, num_channels, true, d_use_tdma);

    connect(self(), 0, d_stream_to_streams, 0);
    for (int i = 0; i < 10; i++) {
        connect(d_stream_to_streams, i, d_channelizer, i);
    }
    uint32_t m = 1;
    for (int i = 0; i < d_num_channels; i++) {
        if (i <= 3) {
            connect(d_channelizer, i, d_resampler[i], 0);
        } else if (i > 3) {
            connect(d_channelizer, 10 - m, d_resampler[i], 0);
            m++;
        }
        connect(d_resampler[i], 0, d_filter[i], 0);
        connect(d_filter[i], 0, d_rssi_tag_block[i], 0);
        connect(d_rssi_tag_block[i], 0, d_fm_demod[i], 0);
        connect(d_fm_demod[i], 0, d_level_control[i], 0);
        connect(d_level_control[i], 0, d_float_to_short[i], 0);
        connect(d_float_to_short[i], 0, d_mmdvm_sink, i);
    }
    for (int i = 0; i < 10 - d_num_channels; i++) {
        connect(d_channelizer, min_c + i, d_null_sink[i], 0);
    }
}

demod_mmdvm_multi2_impl::~demod_mmdvm_multi2_impl() {}

void demod_mmdvm_multi2_impl::set_filter_width(int filter_width)
{
    d_filter_width = filter_width;
    // Recreate filters with new width
    std::vector<float> filter_taps = gr::filter::firdes::low_pass_2(
        1, 24000, d_filter_width, 2000, 60, gr::fft::window::WIN_BLACKMAN_HARRIS);
    for (int i = 0; i < d_num_channels; i++) {
        d_filter[i] = gr::filter::fft_filter_ccf::make(1, filter_taps);
    }
}

void demod_mmdvm_multi2_impl::calibrate_rssi(float level)
{
    for (int i = 0; i < d_num_channels; i++) {
        d_rssi_tag_block[i]->calibrate_rssi(level);
    }
}


} // namespace qradiolink
} // namespace gr

