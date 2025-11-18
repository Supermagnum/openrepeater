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
#include <gnuradio/qradiolink/mod_ssb.h>
#include "mod_ssb_impl.h"
#include <gnuradio/io_signature.h>

// CESSB blocks - migrated to qradiolink namespace
#include <gnuradio/qradiolink/clipper_cc.h>
#include <gnuradio/qradiolink/stretcher_cc.h>

namespace gr {
namespace qradiolink {

mod_ssb::sptr mod_ssb::make(int sps, int samp_rate, int carrier_freq, int filter_width, int sb)
{
    return gnuradio::get_initial_sptr(
        new mod_ssb_impl(sps, samp_rate, carrier_freq, filter_width, sb));
}

mod_ssb_impl::mod_ssb_impl(int sps, int samp_rate, int carrier_freq, int filter_width, int sb)
    : mod_ssb("mod_ssb",
              gr::io_signature::make(1, 1, sizeof(float)),
              gr::io_signature::make(1, 1, sizeof(gr_complex)))
{
    d_samp_rate = samp_rate;
    d_sps = sps;
    float target_samp_rate = 8000.0;
    d_carrier_freq = carrier_freq;
    d_filter_width = filter_width;
    d_sb = sb;

    d_agc = gr::analog::agc2_ff::make(1, 1e-3, 0.5, 1);
    d_agc->set_max_gain(100);
    d_rail = gr::analog::rail_ff::make(-0.6, 0.6);
    d_audio_filter = gr::filter::fft_filter_fff::make(
        1,
        gr::filter::firdes::band_pass_2(
            1, target_samp_rate, 300, d_filter_width, 200, 90, gr::fft::window::WIN_BLACKMAN_HARRIS));
    d_float_to_complex = gr::blocks::float_to_complex::make();
    std::vector<float> interp_taps = gr::filter::firdes::low_pass_2(
        d_sps, d_samp_rate, d_filter_width, d_filter_width, 90, gr::fft::window::WIN_BLACKMAN_HARRIS);

    d_resampler = gr::filter::rational_resampler_ccf::make(d_sps, 1, interp_taps);
    d_feed_forward_agc = gr::analog::feedforward_agc_cc::make(640, 0.5);
    d_amplify = gr::blocks::multiply_const_cc::make(0.9, 1);
    d_bb_gain = gr::blocks::multiply_const_cc::make(1, 1);
    d_filter_usb = gr::filter::fft_filter_ccc::make(
        1,
        gr::filter::firdes::complex_band_pass_2(
            1, target_samp_rate, 200, d_filter_width, 200, 90, gr::fft::window::WIN_BLACKMAN_HARRIS));
    d_filter_lsb = gr::filter::fft_filter_ccc::make(
        1,
        gr::filter::firdes::complex_band_pass_2(
            1, target_samp_rate, -d_filter_width, -200, 200, 90, gr::fft::window::WIN_BLACKMAN_HARRIS));

    // CESSB blocks - using migrated qradiolink namespace
    d_clipper = gr::qradiolink::clipper_cc::make(0.95);
    d_stretcher = gr::qradiolink::stretcher_cc::make();

    connect(self(), 0, d_audio_filter, 0);
    connect(d_audio_filter, 0, d_float_to_complex, 0);
    connect(d_float_to_complex, 0, d_clipper, 0);
    connect(d_clipper, 0, d_stretcher, 0);
    if (!d_sb) {
        connect(d_stretcher, 0, d_filter_usb, 0);
        connect(d_filter_usb, 0, d_amplify, 0);
    } else {
        connect(d_stretcher, 0, d_filter_lsb, 0);
        connect(d_filter_lsb, 0, d_amplify, 0);
    }
    connect(d_amplify, 0, d_bb_gain, 0);
    connect(d_bb_gain, 0, d_resampler, 0);
    connect(d_resampler, 0, self(), 0);
}

mod_ssb_impl::~mod_ssb_impl() {}

void mod_ssb_impl::set_filter_width(int filter_width)
{
    d_filter_width = filter_width;
    float target_samp_rate = 8000.0;
    std::vector<float> interp_taps = gr::filter::firdes::low_pass_2(
        d_sps, d_samp_rate, d_filter_width, d_filter_width, 90, gr::fft::window::WIN_BLACKMAN_HARRIS);

    std::vector<gr_complex> filter_usb_taps = gr::filter::firdes::complex_band_pass_2(
        1, target_samp_rate, 300, d_filter_width, 250, 90, gr::fft::window::WIN_BLACKMAN_HARRIS);
    std::vector<gr_complex> filter_lsb_taps = gr::filter::firdes::complex_band_pass_2(
        1, target_samp_rate, -d_filter_width, -300, 250, 90, gr::fft::window::WIN_BLACKMAN_HARRIS);

    d_resampler->set_taps(interp_taps);
    d_filter_usb->set_taps(filter_usb_taps);
    d_filter_lsb->set_taps(filter_lsb_taps);
}

void mod_ssb_impl::set_bb_gain(float value) { d_bb_gain->set_k(value); }

void mod_ssb::set_filter_width(int filter_width)
{
    // This should never be called, as mod_ssb is only an interface
    // The actual implementation is in mod_ssb_impl
}

void mod_ssb::set_bb_gain(float value)
{
    // This should never be called, as mod_ssb is only an interface
    // The actual implementation is in mod_ssb_impl
}

} // namespace qradiolink
} // namespace gr

