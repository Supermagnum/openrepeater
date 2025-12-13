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
#include <gnuradio/qradiolink/mod_freedv.h>
#include "mod_freedv_impl.h"
#include <typeinfo>
#include <gnuradio/io_signature.h>

namespace gr {
namespace qradiolink {

mod_freedv::~mod_freedv() {}

mod_freedv::sptr mod_freedv::make(int sps, int samp_rate, int carrier_freq, int filter_width, int low_cutoff, int mode, int sb)
{
    return gnuradio::get_initial_sptr(
        new mod_freedv_impl(sps, samp_rate, carrier_freq, filter_width, low_cutoff, mode, sb));
}

mod_freedv_impl::mod_freedv_impl(int sps, int samp_rate, int carrier_freq, int filter_width, int low_cutoff, int mode, int sb)
    : mod_freedv("mod_freedv",
                 gr::io_signature::make(1, 1, sizeof(float)),
                 gr::io_signature::make(1, 1, sizeof(gr_complex)))
{
    d_samp_rate = samp_rate;
    float target_samp_rate = 8000.0;
    d_carrier_freq = carrier_freq;
    d_filter_width = filter_width;
    d_audio_gain = gr::blocks::multiply_const_ff::make(0.15);
    d_agc = gr::analog::agc2_ff::make(1e-1, 1e-3, 0.95, 1);
    d_float_to_short = gr::blocks::float_to_short::make(1, 32765);
    d_short_to_float = gr::blocks::short_to_float::make(1, 32765);
    d_freedv = gr::vocoder::freedv_tx_ss::make(mode);
    d_audio_filter = gr::filter::fft_filter_fff::make(
        1,
        gr::filter::firdes::band_pass(
            1, target_samp_rate, 200, 3500, 350, gr::fft::window::WIN_BLACKMAN_HARRIS));

    d_float_to_complex = gr::blocks::float_to_complex::make();
    std::vector<float> interp_taps = gr::filter::firdes::low_pass(sps, d_samp_rate, d_filter_width, 1200);

    d_resampler = gr::filter::rational_resampler_ccf::make(sps, 1, interp_taps);
    d_feed_forward_agc = gr::analog::feedforward_agc_cc::make(512, 1.0f);
    d_amplify = gr::blocks::multiply_const_cc::make(0.98f, 1);
    d_bb_gain = gr::blocks::multiply_const_cc::make(1, 1);
    if (sb == 0) {
        d_filter = gr::filter::fft_filter_ccc::make(
            1,
            gr::filter::firdes::complex_band_pass_2(
                1, target_samp_rate, low_cutoff, d_filter_width, 250, 90,
                gr::fft::window::WIN_BLACKMAN_HARRIS));
    } else {
        d_filter = gr::filter::fft_filter_ccc::make(
            1,
            gr::filter::firdes::complex_band_pass_2(
                1, target_samp_rate, -d_filter_width, -low_cutoff, 250, 90,
                gr::fft::window::WIN_BLACKMAN_HARRIS));
    }

    connect(self(), 0, d_audio_filter, 0);
    connect(d_audio_filter, 0, d_float_to_short, 0);
    connect(d_float_to_short, 0, d_freedv, 0);
    connect(d_freedv, 0, d_short_to_float, 0);
    connect(d_short_to_float, 0, d_float_to_complex, 0);
    connect(d_float_to_complex, 0, d_filter, 0);
    connect(d_filter, 0, d_feed_forward_agc, 0);
    connect(d_feed_forward_agc, 0, d_resampler, 0);
    connect(d_resampler, 0, d_amplify, 0);
    connect(d_amplify, 0, d_bb_gain, 0);
    connect(d_bb_gain, 0, self(), 0);
}

mod_freedv_impl::~mod_freedv_impl() {}

void mod_freedv_impl::set_bb_gain(float value) { d_bb_gain->set_k(value); }

void mod_freedv::set_bb_gain(float value)
{
    // This should never be called, as mod_freedv is only an interface
    // The actual implementation is in mod_freedv_impl
}

// Force vtable and typeinfo generation for RTTI
namespace {
    const std::type_info& g_mod_freedv_typeinfo = typeid(gr::qradiolink::mod_freedv);
    __attribute__((used)) static const void* force_mod_freedv_typeinfo = &g_mod_freedv_typeinfo;
    
    void* force_vtable_generation() {
        return const_cast<void*>(static_cast<const void*>(&g_mod_freedv_typeinfo));
    }
    __attribute__((used)) static void* vtable_force = force_vtable_generation();
}

} // namespace qradiolink
} // namespace gr

