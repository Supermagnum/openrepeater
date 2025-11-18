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

#include <gnuradio/qradiolink/demod_freedv.h>
#include "demod_freedv_impl.h"
#include <gnuradio/io_signature.h>
#include <cmath>

namespace gr {
namespace qradiolink {

demod_freedv::sptr demod_freedv::make(int sps,
                                      int samp_rate,
                                      int carrier_freq,
                                      int filter_width,
                                      int low_cutoff,
                                      int mode,
                                      int sb)
{
    return gnuradio::get_initial_sptr(new demod_freedv_impl(
        sps, samp_rate, carrier_freq, filter_width, low_cutoff, mode, sb));
}

demod_freedv_impl::demod_freedv_impl(int sps,
                                     int samp_rate,
                                     int carrier_freq,
                                     int filter_width,
                                     int low_cutoff,
                                     int mode,
                                     int sb)
    : demod_freedv("demod_freedv",
                   gr::io_signature::make(1, 1, sizeof(gr_complex)),
                   gr::io_signature::makev(2, 2, {sizeof(gr_complex), sizeof(float)})),
      d_samp_rate(samp_rate),
      d_carrier_freq(carrier_freq),
      d_filter_width(filter_width),
      d_target_samp_rate(8000)
{
    std::vector<float> taps = gr::filter::firdes::low_pass(
        sps, d_samp_rate, d_target_samp_rate / 2, d_target_samp_rate / 2, gr::fft::window::WIN_BLACKMAN_HARRIS);
    d_resampler = gr::filter::rational_resampler_ccf::make(1, sps, taps);
    if (sb == 0) {
        d_filter = gr::filter::fft_filter_ccc::make(
            1,
            gr::filter::firdes::complex_band_pass_2(
                1, d_target_samp_rate, low_cutoff, d_filter_width, 200, 90, gr::fft::window::WIN_BLACKMAN_HARRIS));
    } else {
        d_filter = gr::filter::fft_filter_ccc::make(
            1,
            gr::filter::firdes::complex_band_pass_2(
                1, d_target_samp_rate, -d_filter_width, -low_cutoff, 200, 90, gr::fft::window::WIN_BLACKMAN_HARRIS));
    }

    d_feed_forward_agc = gr::analog::feedforward_agc_cc::make(512, 1);
    d_agc = gr::analog::agc2_ff::make(1e-1, 1e-3, 0.5, 1);
    d_complex_to_real = gr::blocks::complex_to_real::make();
    d_audio_filter = gr::filter::fft_filter_fff::make(
        1,
        gr::filter::firdes::band_pass_2(
            1, d_target_samp_rate, 200, 3500, 200, 90, gr::fft::window::WIN_BLACKMAN_HARRIS));
    d_freedv_gain = gr::blocks::multiply_const_ff::make(0.1);
    d_float_to_short = gr::blocks::float_to_short::make(1, 32768);
    d_freedv = gr::vocoder::freedv_rx_ss::make(mode);
    d_short_to_float = gr::blocks::short_to_float::make(1, 32768);
    d_audio_gain = gr::blocks::multiply_const_ff::make(2);

    connect(self(), 0, d_resampler, 0);

    connect(d_resampler, 0, d_filter, 0);
    connect(d_filter, 0, self(), 0);
    connect(d_filter, 0, d_complex_to_real, 0);
    connect(d_complex_to_real, 0, d_agc, 0);
    connect(d_agc, 0, d_audio_filter, 0);
    connect(d_audio_filter, 0, d_freedv_gain, 0);
    connect(d_freedv_gain, 0, d_float_to_short, 0);
    connect(d_float_to_short, 0, d_freedv, 0);
    connect(d_freedv, 0, d_short_to_float, 0);
    connect(d_short_to_float, 0, d_audio_gain, 0);
    connect(d_audio_gain, 0, self(), 1);
}

demod_freedv_impl::~demod_freedv_impl() {}

void demod_freedv_impl::set_agc_attack(float value)
{
    d_agc->set_attack_rate(value);
}

void demod_freedv_impl::set_agc_decay(float value)
{
    d_agc->set_decay_rate(value);
}

void demod_freedv_impl::set_squelch(int value)
{
    d_freedv->set_squelch_thresh((float)value);
}


} // namespace qradiolink
} // namespace gr

