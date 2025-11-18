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

#include <gnuradio/qradiolink/demod_ssb.h>
#include "demod_ssb_impl.h"
#include <gnuradio/io_signature.h>
#include <gnuradio/filter/firdes.h>
#include <gnuradio/fft/window.h>

namespace gr {
namespace qradiolink {

demod_ssb::sptr demod_ssb::make(int sps, int samp_rate, int carrier_freq, int filter_width, int sb)
{
    return gnuradio::get_initial_sptr(new demod_ssb_impl(sps, samp_rate, carrier_freq, filter_width, sb));
}

demod_ssb_impl::demod_ssb_impl(int sps, int samp_rate, int carrier_freq, int filter_width, int sb)
    : demod_ssb("demod_ssb",
                gr::io_signature::make(1, 1, sizeof(gr_complex)),
                gr::io_signature::makev(2, 2, std::vector<int>{sizeof(gr_complex), sizeof(float)}))
{
    d_target_samp_rate = 8000;
    d_samp_rate = samp_rate;
    d_sps = sps;
    d_carrier_freq = carrier_freq;
    d_filter_width = filter_width;
    d_sb = sb;

    std::vector<float> taps = gr::filter::firdes::low_pass(
        1, d_samp_rate, d_target_samp_rate / 2, d_target_samp_rate / 2, gr::fft::window::WIN_BLACKMAN_HARRIS);
    d_resampler = gr::filter::rational_resampler_ccf::make(1, d_sps, taps);

    d_if_gain = gr::blocks::multiply_const_cc::make(0.9);

    d_filter_usb = gr::filter::fft_filter_ccc::make(
        1,
        gr::filter::firdes::complex_band_pass_2(
            1, d_target_samp_rate, 200, d_filter_width, 200, 90, gr::fft::window::WIN_BLACKMAN_HARRIS));
    d_filter_lsb = gr::filter::fft_filter_ccc::make(
        1,
        gr::filter::firdes::complex_band_pass_2(
            1, d_target_samp_rate, -d_filter_width, -200, 200, 90, gr::fft::window::WIN_BLACKMAN_HARRIS));
    d_squelch = gr::analog::pwr_squelch_cc::make(-140, 0.01, 0, true);
    d_agc = gr::analog::agc2_cc::make(1e-1, 1e-1, 0.25, 1);
    d_audio_filter = gr::filter::fft_filter_fff::make(
        1,
        gr::filter::firdes::band_pass_2(
            1, d_target_samp_rate, 200, d_filter_width, 200, 90, gr::fft::window::WIN_BLACKMAN_HARRIS));
    d_complex_to_real = gr::blocks::complex_to_real::make();
    d_level_control = gr::blocks::multiply_const_ff::make(1.333);
    d_clipper = gr::qradiolink::clipper_cc::make(0.95);
    d_stretcher = gr::qradiolink::stretcher_cc::make();

    connect(self(), 0, d_resampler, 0);
    connect(d_resampler, 0, d_if_gain, 0);
    if (!d_sb) {
        connect(d_if_gain, 0, d_filter_usb, 0);
        connect(d_filter_usb, 0, self(), 0);
        connect(d_filter_usb, 0, d_squelch, 0);
    } else {
        connect(d_if_gain, 0, d_filter_lsb, 0);
        connect(d_filter_lsb, 0, self(), 0);
        connect(d_filter_lsb, 0, d_squelch, 0);
    }
    connect(d_squelch, 0, d_agc, 0);
    connect(d_agc, 0, d_clipper, 0);
    connect(d_clipper, 0, d_stretcher, 0);
    connect(d_stretcher, 0, d_complex_to_real, 0);
    connect(d_complex_to_real, 0, d_level_control, 0);
    connect(d_level_control, 0, d_audio_filter, 0);
    connect(d_audio_filter, 0, self(), 1);
}

demod_ssb_impl::~demod_ssb_impl() {}

void demod_ssb_impl::set_squelch(int value) { d_squelch->set_threshold(value); }

void demod_ssb_impl::set_filter_width(int filter_width)
{
    d_filter_width = filter_width;
    std::vector<gr_complex> filter_usb_taps = gr::filter::firdes::complex_band_pass_2(
        1, d_target_samp_rate, 200, d_filter_width, 200, 90, gr::fft::window::WIN_BLACKMAN_HARRIS);
    std::vector<gr_complex> filter_lsb_taps = gr::filter::firdes::complex_band_pass_2(
        1, d_target_samp_rate, -d_filter_width, -200, 200, 90, gr::fft::window::WIN_BLACKMAN_HARRIS);

    d_filter_usb->set_taps(filter_usb_taps);
    d_filter_lsb->set_taps(filter_lsb_taps);
    d_audio_filter->set_taps(gr::filter::firdes::band_pass_2(
        2, d_target_samp_rate, 200, d_filter_width, 200, 90, gr::fft::window::WIN_BLACKMAN_HARRIS));
}

void demod_ssb_impl::set_agc_attack(float value) { d_agc->set_attack_rate(value); }

void demod_ssb_impl::set_agc_decay(float value) { d_agc->set_decay_rate(value); }

void demod_ssb_impl::set_gain(float value) { d_if_gain->set_k(value); }

void demod_ssb::set_squelch(int value)
{
    // This should never be called, as demod_ssb is only an interface
    // The actual implementation is in demod_ssb_impl
}

void demod_ssb::set_filter_width(int filter_width)
{
    // This should never be called, as demod_ssb is only an interface
    // The actual implementation is in demod_ssb_impl
}

void demod_ssb::set_agc_attack(float value)
{
    // This should never be called, as demod_ssb is only an interface
    // The actual implementation is in demod_ssb_impl
}

void demod_ssb::set_agc_decay(float value)
{
    // This should never be called, as demod_ssb is only an interface
    // The actual implementation is in demod_ssb_impl
}

void demod_ssb::set_gain(float value)
{
    // This should never be called, as demod_ssb is only an interface
    // The actual implementation is in demod_ssb_impl
}

} // namespace qradiolink
} // namespace gr

