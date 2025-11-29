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

#include <gnuradio/qradiolink/demod_nbfm.h>
#include "demod_nbfm_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/fft/window.h>
#include <cmath>
#include "../../src/gr/emphasis.h"

namespace gr {
namespace qradiolink {

demod_nbfm::sptr demod_nbfm::make(int sps, int samp_rate, int carrier_freq, int filter_width)
{
    return gnuradio::get_initial_sptr(new demod_nbfm_impl(sps, samp_rate, carrier_freq, filter_width));
}

demod_nbfm_impl::demod_nbfm_impl(int sps, int samp_rate, int carrier_freq, int filter_width)
    : demod_nbfm("demod_nbfm",
                 gr::io_signature::make(1, 1, sizeof(gr_complex)),
                 gr::io_signature::makev(2, 2, std::vector<int>{sizeof(gr_complex), sizeof(float)}))
{
    (void)sps;
    d_target_samp_rate = 20000;

    d_samp_rate = samp_rate;
    d_carrier_freq = carrier_freq;
    d_filter_width = filter_width;
    gr::calculate_deemph_taps(d_target_samp_rate, 50e-6, d_ataps, d_btaps);

    d_de_emph_filter = gr::filter::iir_filter_ffd::make(d_btaps, d_ataps, false);

    std::vector<float> taps = gr::filter::firdes::low_pass(
        1, d_samp_rate, d_target_samp_rate / 2, d_target_samp_rate / 2, gr::fft::window::WIN_BLACKMAN_HARRIS);
    std::vector<float> audio_taps = gr::filter::firdes::low_pass_2(
        2, 2 * d_target_samp_rate, 3600, 250, 60, gr::fft::window::WIN_BLACKMAN_HARRIS);
    d_resampler = gr::filter::rational_resampler_ccf::make(1, 50, taps);
    d_audio_resampler = gr::filter::rational_resampler_fff::make(2, 5, audio_taps);

    d_filter = gr::filter::fft_filter_ccf::make(
        1,
        gr::filter::firdes::low_pass_2(
            1, d_target_samp_rate, d_filter_width, 3500, 60, gr::fft::window::WIN_BLACKMAN_HARRIS));

    d_fm_demod = gr::analog::quadrature_demod_cf::make(d_target_samp_rate / (4 * M_PI * d_filter_width));
    d_squelch = gr::analog::pwr_squelch_cc::make(-140, 0.01, 320, true);
    d_ctcss = gr::analog::ctcss_squelch_ff::make(8000, 88.5, 0.01, 8000, 160, true);
    d_level_control = gr::blocks::multiply_const_ff::make(2.0);
    d_audio_filter = gr::filter::fft_filter_fff::make(
        1,
        gr::filter::firdes::low_pass_2(
            1, 8000, 3500, 200, 35, gr::fft::window::WIN_BLACKMAN_HARRIS));

    connect(self(), 0, d_resampler, 0);
    connect(d_resampler, 0, d_filter, 0);
    connect(d_filter, 0, self(), 0);
    connect(d_filter, 0, d_squelch, 0);
    connect(d_squelch, 0, d_fm_demod, 0);
    connect(d_fm_demod, 0, d_audio_resampler, 0);
    connect(d_audio_resampler, 0, d_audio_filter, 0);
    connect(d_audio_filter, 0, d_de_emph_filter, 0);
    connect(d_de_emph_filter, 0, d_level_control, 0);
    connect(d_level_control, 0, self(), 1);
}

demod_nbfm_impl::~demod_nbfm_impl() {}

void demod_nbfm_impl::set_squelch(int value) { d_squelch->set_threshold(value); }

void demod_nbfm_impl::set_ctcss(float value)
{
    if (value == 0) {
        // Disable CTCSS
        d_ctcss->set_level(0);
    } else {
        // Enable CTCSS with specified frequency
        d_ctcss->set_level(0.01);
        // Note: set_frequency may not be available, may need to recreate block
        // For now, just set the level
    }
}

void demod_nbfm_impl::set_filter_width(int filter_width)
{
    d_filter_width = filter_width;
    std::vector<float> filter_taps = gr::filter::firdes::low_pass(
        1, d_target_samp_rate, d_filter_width, 1200, gr::fft::window::WIN_BLACKMAN_HARRIS);

    d_filter->set_taps(filter_taps);
    d_fm_demod->set_gain(d_target_samp_rate / (4 * M_PI * d_filter_width));
}

void demod_nbfm::set_squelch(int value)
{
    // This should never be called, as demod_nbfm is only an interface
    // The actual implementation is in demod_nbfm_impl
}

void demod_nbfm::set_ctcss(float value)
{
    // This should never be called, as demod_nbfm is only an interface
    // The actual implementation is in demod_nbfm_impl
}

void demod_nbfm::set_filter_width(int filter_width)
{
    // This should never be called, as demod_nbfm is only an interface
    // The actual implementation is in demod_nbfm_impl
}

} // namespace qradiolink
} // namespace gr

