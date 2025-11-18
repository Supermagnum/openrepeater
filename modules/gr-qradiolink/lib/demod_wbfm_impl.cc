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

#include <gnuradio/qradiolink/demod_wbfm.h>
#include "demod_wbfm_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/fft/window.h>
#include <cmath>
#include "../../src/gr/emphasis.h"

namespace gr {
namespace qradiolink {

demod_wbfm::sptr demod_wbfm::make(int sps, int samp_rate, int carrier_freq, int filter_width)
{
    return gnuradio::get_initial_sptr(new demod_wbfm_impl(sps, samp_rate, carrier_freq, filter_width));
}

demod_wbfm_impl::demod_wbfm_impl(int sps, int samp_rate, int carrier_freq, int filter_width)
    : demod_wbfm("demod_wbfm",
                 gr::io_signature::make(1, 1, sizeof(gr_complex)),
                 gr::io_signature::makev(2, 2, std::vector<int>{sizeof(gr_complex), sizeof(float)}))
{
    (void)sps;
    d_target_samp_rate = 200000;

    d_samp_rate = samp_rate;
    d_carrier_freq = carrier_freq;
    d_filter_width = filter_width;
    gr::calculate_deemph_taps(8000, 50e-6, d_ataps, d_btaps);

    d_de_emph_filter = gr::filter::iir_filter_ffd::make(d_btaps, d_ataps, false);

    std::vector<float> taps = gr::filter::firdes::low_pass(
        1, d_samp_rate, d_target_samp_rate / 2, d_target_samp_rate / 2, gr::fft::window::WIN_BLACKMAN_HARRIS);
    std::vector<float> audio_taps = gr::filter::firdes::low_pass(
        1, d_target_samp_rate, 4000, 2000, gr::fft::window::WIN_BLACKMAN_HARRIS);
    d_resampler = gr::filter::rational_resampler_ccf::make(1, 5, taps);
    d_audio_resampler = gr::filter::rational_resampler_fff::make(1, 25, audio_taps);

    d_filter = gr::filter::fft_filter_ccf::make(
        1,
        gr::filter::firdes::low_pass_2(
            1, d_target_samp_rate, d_filter_width, 600, 90, gr::fft::window::WIN_BLACKMAN_HARRIS));

    d_fm_demod = gr::analog::quadrature_demod_cf::make(d_target_samp_rate / (2 * M_PI * d_filter_width));
    d_squelch = gr::analog::pwr_squelch_cc::make(-140, 0.01, 0, true);
    d_amplify = gr::blocks::multiply_const_ff::make(0.9);

    connect(self(), 0, d_resampler, 0);
    connect(d_resampler, 0, d_filter, 0);
    connect(d_filter, 0, self(), 0);
    connect(d_filter, 0, d_squelch, 0);
    connect(d_squelch, 0, d_fm_demod, 0);
    connect(d_fm_demod, 0, d_amplify, 0);
    connect(d_amplify, 0, d_de_emph_filter, 0);
    connect(d_de_emph_filter, 0, d_audio_resampler, 0);
    connect(d_audio_resampler, 0, self(), 1);
}

demod_wbfm_impl::~demod_wbfm_impl() {}

void demod_wbfm_impl::set_squelch(int value) { d_squelch->set_threshold(value); }

void demod_wbfm_impl::set_filter_width(int filter_width)
{
    d_filter_width = filter_width;
    std::vector<float> filter_taps = gr::filter::firdes::low_pass(
        1, d_target_samp_rate, d_filter_width, 1200, gr::fft::window::WIN_BLACKMAN_HARRIS);

    d_filter->set_taps(filter_taps);
    d_fm_demod->set_gain(d_target_samp_rate / (2 * M_PI * d_filter_width));
}

void demod_wbfm::set_squelch(int value)
{
    // This should never be called, as demod_wbfm is only an interface
    // The actual implementation is in demod_wbfm_impl
}

void demod_wbfm::set_filter_width(int filter_width)
{
    // This should never be called, as demod_wbfm is only an interface
    // The actual implementation is in demod_wbfm_impl
}

} // namespace qradiolink
} // namespace gr

