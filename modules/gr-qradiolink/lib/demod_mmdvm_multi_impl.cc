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

#include <gnuradio/qradiolink/demod_mmdvm_multi.h>
#include "demod_mmdvm_multi_impl.h"
#include <gnuradio/io_signature.h>
#include <cmath>
#include <algorithm>
#include <typeinfo>
#include "../src/bursttimer.h"
#include "../src/config_mmdvm.h"

namespace gr {
namespace qradiolink {

demod_mmdvm_multi::~demod_mmdvm_multi() {}

demod_mmdvm_multi::sptr demod_mmdvm_multi::make(BurstTimer* burst_timer,
                                               int num_channels,
                                               int channel_separation,
                                               bool use_tdma,
                                               int sps,
                                               int samp_rate,
                                               int carrier_freq,
                                               int filter_width)
{
    return gnuradio::get_initial_sptr(new demod_mmdvm_multi_impl(
        burst_timer, num_channels, channel_separation, use_tdma, sps, samp_rate, carrier_freq, filter_width));
}

demod_mmdvm_multi_impl::demod_mmdvm_multi_impl(BurstTimer* burst_timer,
                                               int num_channels,
                                               int channel_separation,
                                               bool use_tdma,
                                               int sps,
                                               int samp_rate,
                                               int carrier_freq,
                                               int filter_width)
    : demod_mmdvm_multi("demod_mmdvm_multi",
                       gr::io_signature::make(1, 1, sizeof(gr_complex)),
                       gr::io_signature::make(0, 0, sizeof(short))),
      d_samp_rate(samp_rate),
      d_carrier_freq(carrier_freq),
      d_filter_width(filter_width),
      d_num_channels(num_channels),
      d_use_tdma(use_tdma)
{
    (void)sps;
    if (num_channels > MAX_MMDVM_CHANNELS)
        num_channels = MAX_MMDVM_CHANNELS;
    d_num_channels = num_channels;
    float target_samp_rate = 24000;
    float intermediate_samp_rate = 240000;
    float fm_demod_width = 12500.0f;
    int c_n_b = num_channels;
    if (c_n_b > 4)
        c_n_b = 4;
    int resamp_filter_width = c_n_b * channel_separation;
    int resamp_filter_slope = 25000;
    float carrier_offset = float(-channel_separation);

    std::vector<float> taps = gr::filter::firdes::low_pass(
        1, d_samp_rate, resamp_filter_width, resamp_filter_slope, gr::fft::window::WIN_BLACKMAN_HARRIS);
    std::vector<float> intermediate_interp_taps = gr::filter::firdes::low_pass(
        1, intermediate_samp_rate, d_filter_width, 3500, gr::fft::window::WIN_BLACKMAN_HARRIS);

    for (int i = 0; i < d_num_channels; i++) {
        d_resampler[i] = gr::filter::rational_resampler_ccf::make(1, 10, intermediate_interp_taps);
    }
    for (int i = 0; i < d_num_channels; i++) {
        d_filter[i] = gr::filter::fft_filter_ccf::make(
            1, gr::filter::firdes::low_pass(1, target_samp_rate, d_filter_width, 3500, gr::fft::window::WIN_BLACKMAN_HARRIS));
    }
    for (int i = 0; i < d_num_channels; i++) {
        d_fm_demod[i] = gr::analog::quadrature_demod_cf::make(
            float(target_samp_rate) / (2 * M_PI * float(fm_demod_width)));
    }
    for (int i = 0; i < d_num_channels; i++) {
        d_level_control[i] = gr::blocks::multiply_const_ff::make(1.0);
    }
    for (int i = 0; i < d_num_channels; i++) {
        d_float_to_short[i] = gr::blocks::float_to_short::make(1, 32767.0);
    }
    for (int i = 0; i < d_num_channels; i++) {
        int ct = i;
        if (i > 3)
            ct = 3 - i;
        d_rotator[i] = gr::blocks::rotator_cc::make(2 * M_PI * carrier_offset * ct / intermediate_samp_rate);
    }
    for (int i = 0; i < d_num_channels; i++) {
        d_rssi_tag_block[i] = gr::qradiolink::rssi_tag_block::make();
    }

    d_first_resampler = gr::filter::rational_resampler_ccf::make(1, 5, taps);
    d_mmdvm_sink = gr::qradiolink::mmdvm_sink::make(burst_timer, num_channels, true, d_use_tdma);

    for (int i = 0; i < d_num_channels; i++) {
        if (i == 0) {
            connect(self(), 0, d_resampler[i], 0);
        } else {
            connect(self(), 0, d_rotator[i], 0);
            connect(d_rotator[i], 0, d_resampler[i], 0);
        }
        connect(d_resampler[i], 0, d_filter[i], 0);
        connect(d_filter[i], 0, d_rssi_tag_block[i], 0);
        connect(d_rssi_tag_block[i], 0, d_fm_demod[i], 0);
        connect(d_fm_demod[i], 0, d_level_control[i], 0);
        connect(d_level_control[i], 0, d_float_to_short[i], 0);
        connect(d_float_to_short[i], 0, d_mmdvm_sink, i);
    }
}

demod_mmdvm_multi_impl::~demod_mmdvm_multi_impl() {}

void demod_mmdvm_multi::set_filter_width(int filter_width)
{
    // Base class implementation - should never be called
    // Actual implementation is in demod_mmdvm_multi_impl
    (void)filter_width; // Suppress unused parameter warning
}

void demod_mmdvm_multi::calibrate_rssi(float level)
{
    // Base class implementation - should never be called
    (void)level; // Suppress unused parameter warning
}

void demod_mmdvm_multi_impl::set_filter_width(int filter_width)
{
    d_filter_width = filter_width;
    // Recreate filters with new width
    for (int i = 0; i < d_num_channels; i++) {
        std::vector<float> filter_taps = gr::filter::firdes::low_pass(
            1, 24000, d_filter_width, 3500, gr::fft::window::WIN_BLACKMAN_HARRIS);
        d_filter[i] = gr::filter::fft_filter_ccf::make(1, filter_taps);
    }
}

void demod_mmdvm_multi_impl::calibrate_rssi(float level)
{
    for (int i = 0; i < d_num_channels; i++) {
        d_rssi_tag_block[i]->calibrate_rssi(level);
    }
}

// Force vtable and typeinfo generation for RTTI
namespace {
    const std::type_info& g_demod_mmdvm_multi_typeinfo = typeid(gr::qradiolink::demod_mmdvm_multi);
    
    struct ForceTypeinfo {
        ForceTypeinfo() {
            const void* ptr = &g_demod_mmdvm_multi_typeinfo;
            (void)ptr;
        }
    };
    __attribute__((used)) static ForceTypeinfo force_typeinfo;
    
    __attribute__((used)) static const void* force_demod_mmdvm_multi_typeinfo = &g_demod_mmdvm_multi_typeinfo;
}

} // namespace qradiolink
} // namespace gr

