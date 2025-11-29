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
#include <gnuradio/qradiolink/mod_nbfm.h>
#include "mod_nbfm_impl.h"
#include <gnuradio/io_signature.h>
#include "../../src/gr/emphasis.h"

namespace gr {
namespace qradiolink {

mod_nbfm::sptr mod_nbfm::make(int sps, int samp_rate, int carrier_freq, int filter_width)
{
    return gnuradio::get_initial_sptr(
        new mod_nbfm_impl(sps, samp_rate, carrier_freq, filter_width));
}

mod_nbfm_impl::mod_nbfm_impl(int sps, int samp_rate, int carrier_freq, int filter_width)
    : mod_nbfm("mod_nbfm",
               gr::io_signature::make(1, 1, sizeof(float)),
               gr::io_signature::make(1, 1, sizeof(gr_complex)))
{
    d_samp_rate = samp_rate;
    d_sps = sps;
    float target_samp_rate = 8000;
    float if_samp_rate = 50000;
    d_carrier_freq = carrier_freq;
    d_filter_width = filter_width;
    gr::calculate_preemph_taps(8000, 50e-6, d_ataps, d_btaps);

    d_fm_modulator = gr::analog::frequency_modulator_fc::make(4 * M_PI * d_filter_width / if_samp_rate);
    d_audio_amplify = gr::blocks::multiply_const_ff::make(0.99, 1);
    d_audio_filter = gr::filter::fft_filter_fff::make(
        1,
        gr::filter::firdes::low_pass_2(
            1, target_samp_rate, 3500, 200, 35, gr::fft::window::WIN_BLACKMAN_HARRIS));

    d_pre_emph_filter = gr::filter::iir_filter_ffd::make(d_btaps, d_ataps, false);

    std::vector<float> if_taps = gr::filter::firdes::low_pass_2(
        25, if_samp_rate * 4, d_filter_width, 3500, 60, gr::fft::window::WIN_BLACKMAN_HARRIS);
    d_if_resampler = gr::filter::rational_resampler_fff::make(25, 4, if_taps);

    d_tone_source = gr::analog::sig_source_f::make(
        target_samp_rate, gr::analog::GR_COS_WAVE, 88.5, 0.15);
    d_add = gr::blocks::add_ff::make();

    std::vector<float> interp_taps = gr::filter::firdes::low_pass_2(
        d_sps, d_samp_rate, d_filter_width, 3500, 60, gr::fft::window::WIN_BLACKMAN_HARRIS);
    d_resampler = gr::filter::rational_resampler_ccf::make(d_sps, 1, interp_taps);
    d_amplify = gr::blocks::multiply_const_cc::make(0.8, 1);
    d_bb_gain = gr::blocks::multiply_const_cc::make(1, 1);
    d_filter = gr::filter::fft_filter_ccf::make(
        1,
        gr::filter::firdes::low_pass_2(
            1, if_samp_rate, d_filter_width, 3500, 60, gr::fft::window::WIN_BLACKMAN_HARRIS));

    connect(self(), 0, d_audio_filter, 0);
    connect(d_audio_filter, 0, d_audio_amplify, 0);
    connect(d_audio_amplify, 0, d_pre_emph_filter, 0);
    connect(d_pre_emph_filter, 0, d_if_resampler, 0);
    connect(d_if_resampler, 0, d_fm_modulator, 0);
    connect(d_fm_modulator, 0, d_filter, 0);
    connect(d_filter, 0, d_amplify, 0);
    connect(d_amplify, 0, d_bb_gain, 0);
    connect(d_bb_gain, 0, d_resampler, 0);
    connect(d_resampler, 0, self(), 0);
}

mod_nbfm_impl::~mod_nbfm_impl() {}

void mod_nbfm_impl::set_filter_width(int filter_width)
{
    d_filter_width = filter_width;
    float if_samp_rate = 50000;
    std::vector<float> if_taps = gr::filter::firdes::low_pass_2(
        25, if_samp_rate * 4, d_filter_width, d_filter_width, 60, gr::fft::window::WIN_BLACKMAN_HARRIS);
    std::vector<float> filter_taps = gr::filter::firdes::low_pass_2(
        1, if_samp_rate, d_filter_width, 1200, 60, gr::fft::window::WIN_BLACKMAN_HARRIS);
    std::vector<float> interp_taps = gr::filter::firdes::low_pass_2(
        d_sps, d_samp_rate, d_filter_width, d_filter_width, 60, gr::fft::window::WIN_BLACKMAN_HARRIS);

    d_if_resampler->set_taps(if_taps);
    d_filter->set_taps(filter_taps);
    d_resampler->set_taps(interp_taps);
    d_fm_modulator->set_sensitivity(4 * M_PI * d_filter_width / if_samp_rate);
}

void mod_nbfm_impl::set_ctcss(float value)
{
    // TODO: Implement CTCSS tone setting
    // This would require reconnecting the tone source
}

void mod_nbfm_impl::set_bb_gain(float value) { d_bb_gain->set_k(value); }

void mod_nbfm::set_filter_width(int filter_width)
{
    // This should never be called, as mod_nbfm is only an interface
    // The actual implementation is in mod_nbfm_impl
}

void mod_nbfm::set_ctcss(float value)
{
    // This should never be called, as mod_nbfm is only an interface
    // The actual implementation is in mod_nbfm_impl
}

void mod_nbfm::set_bb_gain(float value)
{
    // This should never be called, as mod_nbfm is only an interface
    // The actual implementation is in mod_nbfm_impl
}

} // namespace qradiolink
} // namespace gr

