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

#include <gnuradio/qradiolink/demod_am.h>
#include "demod_am_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/fft/window.h>

namespace gr {
namespace qradiolink {

demod_am::sptr demod_am::make(int sps, int samp_rate, int carrier_freq, int filter_width)
{
    return gnuradio::get_initial_sptr(new demod_am_impl(sps, samp_rate, carrier_freq, filter_width));
}

demod_am_impl::demod_am_impl(int sps, int samp_rate, int carrier_freq, int filter_width)
    : demod_am("demod_am",
               gr::io_signature::make(1, 1, sizeof(gr_complex)),
               gr::io_signature::makev(2, 2, std::vector<int>{sizeof(gr_complex), sizeof(float)}))
{
    (void)sps; // Unused in original
    d_target_samp_rate = 20000;
    d_samp_rate = samp_rate;
    d_carrier_freq = carrier_freq;
    d_filter_width = filter_width;

    std::vector<float> taps = gr::filter::firdes::low_pass(
        1, d_samp_rate, d_target_samp_rate / 2, d_target_samp_rate / 2, gr::fft::window::WIN_BLACKMAN_HARRIS);
    std::vector<float> audio_taps = gr::filter::firdes::low_pass(
        2, 2 * d_target_samp_rate, 3600, 600, gr::fft::window::WIN_BLACKMAN_HARRIS);
    d_resampler = gr::filter::rational_resampler_ccf::make(1, 50, taps);
    d_audio_resampler = gr::filter::rational_resampler_fff::make(2, 5, audio_taps);
    d_filter = gr::filter::fft_filter_ccc::make(
        1,
        gr::filter::firdes::complex_band_pass_2(
            1, d_target_samp_rate, -d_filter_width, d_filter_width, 200, 90,
            gr::fft::window::WIN_BLACKMAN_HARRIS));
    d_squelch = gr::analog::pwr_squelch_cc::make(-140, 0.01, 0, true);
    d_agc = gr::analog::agc2_ff::make(1e-1, 1e-1, 1.0, 1.0);
    d_complex_to_mag = gr::blocks::complex_to_mag::make();
    std::vector<double> fft;
    fft.push_back(1);
    fft.push_back(-1);
    std::vector<double> ffd;
    ffd.push_back(0);
    ffd.push_back(0.9999);
    d_iir_filter = gr::filter::iir_filter_ffd::make(fft, ffd);
    d_audio_gain = gr::blocks::multiply_const_ff::make(0.99);
    d_audio_filter = gr::filter::fft_filter_fff::make(
        1,
        gr::filter::firdes::low_pass(1, 8000, 3600, 300, gr::fft::window::WIN_BLACKMAN_HARRIS));

    connect(self(), 0, d_resampler, 0);
    connect(d_resampler, 0, d_filter, 0);
    connect(d_filter, 0, self(), 0); // Output 0: filtered complex
    connect(d_filter, 0, d_squelch, 0);
    connect(d_squelch, 0, d_complex_to_mag, 0);
    connect(d_complex_to_mag, 0, d_agc, 0);
    connect(d_agc, 0, d_iir_filter, 0);
    connect(d_iir_filter, 0, d_audio_gain, 0);
    connect(d_audio_gain, 0, d_audio_resampler, 0);
    connect(d_audio_resampler, 0, d_audio_filter, 0);
    connect(d_audio_filter, 0, self(), 1); // Output 1: audio
}

demod_am_impl::~demod_am_impl() {}

void demod_am_impl::set_squelch(int value) { d_squelch->set_threshold(value); }

void demod_am_impl::set_filter_width(int filter_width)
{
    d_filter_width = filter_width;
    std::vector<gr_complex> filter_taps = gr::filter::firdes::complex_band_pass(
        1, d_target_samp_rate, -d_filter_width, d_filter_width, 1200, gr::fft::window::WIN_BLACKMAN_HARRIS);
    d_filter->set_taps(filter_taps);
}

void demod_am_impl::set_agc_attack(float value) { d_agc->set_attack_rate(value); }

void demod_am_impl::set_agc_decay(float value) { d_agc->set_decay_rate(value); }

void demod_am::set_squelch(int value)
{
    // This should never be called, as demod_am is only an interface
    // The actual implementation is in demod_am_impl
}

void demod_am::set_filter_width(int filter_width)
{
    // This should never be called, as demod_am is only an interface
    // The actual implementation is in demod_am_impl
}

void demod_am::set_agc_attack(float value)
{
    // This should never be called, as demod_am is only an interface
    // The actual implementation is in demod_am_impl
}

void demod_am::set_agc_decay(float value)
{
    // This should never be called, as demod_am is only an interface
    // The actual implementation is in demod_am_impl
}

} // namespace qradiolink
} // namespace gr

